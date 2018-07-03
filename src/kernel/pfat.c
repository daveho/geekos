/*
 * GeekOS - PFAT filesystem
 * Copyright (C) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *   
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *  
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* PFAT - a simple FAT-like filesystem */

#include <geekos/vfs.h>
#include <geekos/dev.h>
#include <geekos/blockdev.h>
#include <geekos/mem.h>
#include <geekos/errno.h>
#include <geekos/range.h>
#include <geekos/synch.h>
#include <geekos/vm.h>
#include <geekos/pfat.h>

/*
 * NOTES:
 * - p field of fs_instance object points to pfat_instance object
 * - p field of inode object points to pfat_inode object
 */

#define PFAT_FORMAT_ERROR    EINVAL /* FIXME: better error code? */

/* ------------------- private data ------------------- */

/*
 * Data structure representing a mounted PFAT filesystem instance.
 */
struct pfat_instance {
	struct blockdev *dev;
	struct pfat_superblock *super;
	struct inode *root_dir;
	struct vm_pagecache *fat_cache; /* cache of FAT data */
	struct mutex lock;
};

/*
 * PFAT inode data.
 */
struct pfat_inode {
	u32_t fat_index;  /* first FAT entry */
};

/*
 * fs_driver_ops functions.
 */
static const char *pfat_get_name(struct fs_driver *driver);
static int pfat_create_instance(
	struct fs_driver *fs,
	const char *init, const char *opts, struct fs_instance **p_instance);

static struct fs_driver_ops s_pfat_driver_ops = {
	.get_name = &pfat_get_name,
	.create_instance = &pfat_create_instance,
};

static struct fs_driver s_pfat_driver = {
	.ops = &s_pfat_driver_ops,
};

/*
 * fs_instance_ops functions
 */
static int pfat_get_root(struct fs_instance *instance, struct inode **p_dir);
static int pfat_open(struct fs_instance *instance, const char *path, int mode, struct inode **p_inode);
static int pfat_close_instance(struct fs_instance *instance);

static struct fs_instance_ops s_pfat_instance_ops = {
	.get_root = &pfat_get_root,
	.open = &pfat_open,
	.close_instance = &pfat_close_instance,
};

/*
 * inode_ops functions
 */
static int pfat_read_page(struct inode *inode, void *buf, u32_t page_num);
static int pfat_write_page(struct inode *inode, void *buf, u32_t page_num);
static int pfat_close(struct inode *inode);
static int pfat_lookup(struct inode *inode, const char *name, struct inode **p_inode);

static struct inode_ops s_pfat_inode_ops = {
	.read_page = &pfat_read_page,
	.write_page = &pfat_write_page,
	.close = &pfat_close,
	.lookup = &pfat_lookup,
};

/* ------------------- private functions ------------------- */

static int pfat_read_super(struct blockdev *dev, struct pfat_superblock **p_super)
{
	int rc;
	struct pfat_superblock *super = 0;
	blocksize_t dev_block_size;
	unsigned super_bufsize;

	/* device block size */
	dev_block_size = blockdev_get_block_size(dev);

	/* read superblock into a buffer */
	super_bufsize = range_umax(sizeof(struct pfat_superblock), blocksize_size(dev_block_size));
	super = mem_alloc(super_bufsize);
	rc = blockdev_read_sync(dev, lba_from_num(0), super_bufsize / blocksize_size(dev_block_size), super);
	if (rc != 0) {
		goto fail;
	}

	/* check magic */
	if (super->magic != PFAT_MAGIC) {
		rc = PFAT_FORMAT_ERROR;
		goto fail;
	}

	/* success! */
	return 0;

fail:
	mem_free(super);
	return rc;
}

u32_t pfat_get_fat_num_blocks(struct blockdev *dev, struct pfat_superblock *super)
{
	return lba_get_num_blocks_in_table(
		blockdev_get_block_size(dev), super->fat_num_entries, sizeof(struct pfat_entry));
}

static int pfat_check_super(struct blockdev *dev, struct pfat_superblock *super)
{
	u32_t fat_num_blocks;

	fat_num_blocks = pfat_get_fat_num_blocks(dev, super);

	/* make sure FAT fits on device */
	if (!range_is_valid_u32(super->fat_lba, fat_num_blocks, blockdev_get_num_blocks(dev))) {
		goto fail;
	}

	/* first cluster LBA must be past FAT */
	if (super->first_cluster_lba < super->fat_lba + fat_num_blocks) {
		goto fail;
	}

	/* TODO: other checks? */

	/* Super block looks OK */
	return 0;

fail:
	return PFAT_FORMAT_ERROR;
}

static int pfat_get_inode(
	struct fs_instance *fs_inst, u32_t fat_index, struct inode *parent,
	vfs_inode_type_t type, char *name, struct inode **p_inode)
{
	int rc;
	struct pfat_inode *pfat_inode;
	struct inode *inode;

	/* create PFAT-specific inode data */
	pfat_inode = mem_alloc(sizeof(struct pfat_inode));
	pfat_inode->fat_index = fat_index;

	/* create the actual inode */
	rc = vfs_inode_create(&s_pfat_inode_ops, fs_inst, parent, type, name, pfat_inode, &inode);
	if (rc != 0) {
		goto done;
	}

	/* success! add the initial reference */
	inode->refcount++;
	KASSERT(inode->refcount == 1);
	*p_inode = inode;

done:
	if (rc != 0) {
		mem_free(pfat_inode);
	}
	return rc;
}

/* ------------------- fs_driver operations ------------------- */

static const char *pfat_get_name(struct fs_driver *driver)
{
	return "pfat";
}

static int pfat_create_instance(
	struct fs_driver *fs,
	const char *init, const char *opts, struct fs_instance **p_instance)
{
	int rc;
	struct fs_instance *fs_inst;
	struct blockdev *dev = 0;
	struct pfat_superblock *super = 0;
	struct pfat_instance *inst_data = 0;

	KASSERT(fs == &s_pfat_driver);

	/* the init parameter is the name of the block device containing the fs */
	rc = dev_find_blockdev(init, &dev);
	if (rc != 0) {
		goto done;
	}

	/* read superblock */
	rc = pfat_read_super(dev, &super);
	if (rc != 0) {
		goto done;
	}

	/* check superblock for consistency */
	rc = pfat_check_super(dev, super);
	if (rc != 0) {
		goto done;
	}

	/* read root directory? */

	/*
	 * things look good - create the instance
	 */
	inst_data = mem_alloc(sizeof(struct pfat_instance));
	inst_data->dev = dev;
	inst_data->super = super;
	mutex_init(&inst_data->lock);

	rc = vfs_fs_instance_create(&s_pfat_instance_ops, inst_data, &fs_inst);

done:
	if (rc != 0) {
		mem_free(inst_data);
		mem_free(super);
		blockdev_close(dev);
	}
	return rc;
}

/* ------------------- fs_instance operations ------------------- */

static int pfat_get_root(struct fs_instance *instance, struct inode **p_dir)
{
	int rc;
	struct pfat_instance *inst_data = instance->p;

	mutex_lock(&inst_data->lock);

	if (inst_data->root_dir == 0) {
		/*
		 * Root directory inode object hasn't been created yet;
		 * instantiate it.
		 */
		rc = pfat_get_inode(instance, inst_data->super->root_dir_fat_index, 0, VFS_DIR, 0, p_dir);
	} else {
		/*
		 * Add a reference to the already-instantiated
		 * root directory inode.
		 */
		*p_dir = inst_data->root_dir;
		(*p_dir)->refcount++;
		rc = 0;
	}
	KASSERT(rc != 0 || (*p_dir)->refcount > 0);

	mutex_unlock(&inst_data->lock);

	return rc;
}

static int pfat_open(struct fs_instance *instance, const char *path, int mode, struct inode **p_inode)
{
	KASSERT(false);
	return ENOTSUP;
}

static int pfat_close_instance(struct fs_instance *instance)
{
	KASSERT(false);
	return ENOTSUP;
}

/* ------------------- inode_ops functions ------------------- */

static int pfat_read_page(struct inode *inode, void *buf, u32_t page_num)
{
	KASSERT(false);
	return ENOTSUP;
}

static int pfat_write_page(struct inode *inode, void *buf, u32_t page_num)
{
	KASSERT(false);
	return ENOTSUP;
}

static int pfat_close(struct inode *inode)
{
	KASSERT(false);
	return ENOTSUP;
}

static int pfat_lookup(struct inode *inode, const char *name, struct inode **p_inode)
{
	KASSERT(false);
	return ENOTSUP;
}

/* ------------------- public interface ------------------- */

int pfat_init(void)
{
	return vfs_register_fs_driver(&s_pfat_driver);
}
