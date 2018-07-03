/*
 * GeekOS - virtual filesystem (VFS)
 *
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

#include <geekos/vfs.h>
#include <geekos/errno.h>
#include <geekos/string.h>
#include <geekos/mem.h>

/*
 * VFS locking and refcounting rules:
 *
 * - s_fs_mutex must be held while navigating the tree.
 *
 * - A directory inode is marked as busy while performing
 *   a lookup or other operation that depends upon or changes
 *   the tree structure.
 *
 * - s_fs_mutex is unlocked when performing a potentially long-running
 *   I/O operation on a directory inode; re-acquired when finished.
 *   This is safe because the inode is marked as busy, so any
 *   concurrent vfs calls will be blocked if they reach the inode
 *   and try to do something.
 *
 * - An inode's refcount is the number of threads holding an
 *   active reference to the inode, or any tree descendent of the inode.
 *   In other words, if a thread holds a reference to an inode,
 *   it has incremented the refcount of the inode and all of
 *   the inode's tree ancestors back to the root directory.
 *
 * - Acquisition order: s_fs_mutex is acquired before
 *   s_driver_list_mutex if both are to be held simultaneously.
 */

/* ---------- Private Implementation ---------- */

IMPLEMENT_LIST_GET_FIRST(inode_list, inode)
IMPLEMENT_LIST_NEXT(inode_list, inode)
IMPLEMENT_LIST_APPEND(inode_list, inode)

IMPLEMENT_LIST_APPEND(fs_instance_list, fs_instance)

/* filesystem driver list */
struct mutex s_driver_list_mutex;    /* protects changes/access to fs driver list */
struct fs_driver *s_driver_list;     /* list of filesystem drivers */

/* filesystem data structures */
struct mutex s_fs_mutex;             /* protects changes/access to the tree structure */
struct fs_instance *s_root_instance; /* root filesystem instance */
struct inode *s_root_dir;            /* root directory */
struct fs_instance_list s_inst_list; /* list of all mounted fs_instances */

/*
 * Adjust the refcount of given inode and all of its tree ancestors
 * by given delta.
 */
static void vfs_adjust_refcounts(struct inode *inode, int delta)
{
	KASSERT(MUTEX_IS_HELD(&s_fs_mutex));

	/*
	 * FIXME: how should this operation work when it
	 *        crosses filesystem (mount) boundaries?
	 *        need to think about.
	 */

	for (; inode != 0; inode = inode->parent) {
		KASSERT(inode->refcount >= 0);
		KASSERT(delta > 0 || inode->refcount > 0);
		inode->refcount += delta;
	}
}

/*
 * Find an fs_driver.
 */
int vfs_find_fs_driver(const char *name, struct fs_driver **p_driver)
{
	struct fs_driver *fs_driver;

	mutex_lock(&s_driver_list_mutex);

	for (fs_driver = s_driver_list; fs_driver != 0; fs_driver = fs_driver->next) {
		if (strcmp(name, fs_driver->ops->get_name(fs_driver)) == 0) {
			break;
		}
	}

	mutex_unlock(&s_driver_list_mutex);

	if (fs_driver != 0) {
		*p_driver = fs_driver;
		return 0;
	} else {
		return EINVAL;
	}
}

/*
 * Common implementation function for vfs_mount_root()
 * and vfs_mount(), since they are quite similar.
 */
int vfs_do_mount(const char *fs_driver_name,
	struct inode *mountpoint,
	const char *init, const char *opts,
	struct inode **p_root_dir)
{
	int rc;
	struct fs_driver *fs_driver;
	struct fs_instance *fs_inst = 0;
	struct inode *root_dir = 0;

	KASSERT(MUTEX_IS_HELD(&s_fs_mutex));
	KASSERT(mountpoint == 0 || (mountpoint->type == VFS_DIR && mountpoint->mount == 0));

	/* if mounting root,
	 * make sure a root filesystem hasn't already been mounted */
	if (mountpoint == 0 && s_root_instance != 0) {
		rc = EEXIST;
		goto done;
	}

	/* find the filesystem driver */
	rc = vfs_find_fs_driver(fs_driver_name, &fs_driver);
	if (rc != 0) {
		goto done;
	}

	/* create the fs_instance */
	rc = fs_driver->ops->create_instance(fs_driver, init, opts, &fs_inst);
	if (rc != 0) {
		goto done;
	}

	/* find the root directory */
	rc = fs_inst->ops->get_root(fs_inst, &root_dir);
	if (rc != 0) {
		goto done;
	}

	/* success! */
	*p_root_dir = root_dir;

	/* keep a list of all mounted filesystems */
	fs_instance_list_append(&s_inst_list, fs_inst);

done:
	if (rc != 0) {
		KASSERT(root_dir == 0);
		if (fs_inst != 0) {
			fs_inst->ops->close_instance(fs_inst);
		}
	}
	return rc;
}

/*
 * Determine whether or not given path has more
 * path elements.
 */
static bool vfs_has_more_path_elements(const char *path)
{
	return *path != '\0';
}

/*
 * Copy the next path element in given path buffer
 * into given name buffer.
 * If successful, *p_path is updated to point to the beginning
 * of the remaining path components, and 0 is returned.
 * Otherwise, an error code is returned.
 */
static int vfs_get_next_path_element(const char **p_path, char *namebuf)
{
	size_t namelen = 0;
	const char *p = *p_path;

	KASSERT(*p != '/');

	/* search for next separator (or end of string) */
	while (*p != '/' && *p != '\0') {
		p++;
	}

	/* make sure length of this component is legal */
	namelen = p - *p_path;
	if (namelen > VFS_NAMELEN_MAX)
		return EINVAL;

	/* copy name into buffer */
	memcpy(namebuf, *p_path, namelen);
	namebuf[namelen] = '\0';
	KASSERT(strnlen(namebuf, VFS_NAMELEN_MAX + 1) <= VFS_NAMELEN_MAX);

	/* skip over any path separators */
	while (*p == '/') {
		p++;
	}

	/* update the path pointer to beginning of next path component
	 * (or end of string) */
	*p_path = p;

	return 0;
}

/*
 * Search for named child in given directory.
 * The dir inode must be locked.
 * If sucessful, stores pointer to named child in p_inode and
 * returns 0.  Otherwise, returns error code.
 */
int vfs_lookup_child(struct inode *dir, const char *name, struct inode **p_inode)
{
	int rc = 0;
	struct inode *child;

	KASSERT(MUTEX_IS_HELD(&s_fs_mutex));
	KASSERT(dir->type == VFS_DIR);
	KASSERT(dir->busy);

	/* first, see if the child is already part of the dir's child list */
	for (child = inode_list_get_first(&dir->child_list);
	     child != 0;
	     child = inode_list_next(child)) {
		if (strncmp(name, dir->name, VFS_NAMELEN_MAX) == 0) {
			*p_inode = child;
			goto done;
		}
	}

	/* release fs mutex while search is in progress */
	mutex_unlock(&s_fs_mutex);

	/* look up child from filesystem */
	rc = dir->ops->lookup(dir, name, p_inode);

	/* if lookup succeeded, add to dir's child list */
	if (rc == 0) {
		inode_list_append(&dir->child_list, *p_inode);
	}

	/* re-acquire the fs mutex */
	mutex_lock(&s_fs_mutex);

done:
	return rc;
}

/*
 * Lock a directory in preparation for a lookup.
 * fs mutex must be held.
 */
static void vfs_lock_dir(struct inode *dir)
{
	KASSERT(MUTEX_IS_HELD(&s_fs_mutex));
	while (dir->busy) {
		cond_wait(&dir->inode_cond, &s_fs_mutex);
	}
	dir->busy = true;
}

/*
 * Unlock a directory after completing a lookup.
 * fs mutex must be held.
 */
static void vfs_unlock_dir(struct inode *dir)
{
	KASSERT(MUTEX_IS_HELD(&s_fs_mutex));
	KASSERT(dir->busy);
	dir->busy = false;
	cond_broadcast(&dir->inode_cond);
}

/* ---------- Public Interface ---------- */

int vfs_mount_root(const char *fs_driver_name, const char *init, const char *opts)
{
	int rc;

	mutex_lock(&s_fs_mutex);
	rc = vfs_do_mount(fs_driver_name, 0, init, opts, &s_root_dir);
	mutex_unlock(&s_fs_mutex);

	return rc;
}

int vfs_mount(const char *path, const char *fs_driver_name, const char *init, const char *opts)
{
	int rc;
	struct inode *root_dir = 0;
	struct inode *mountpoint = 0;
	struct inode *mount_root_dir = 0;

	rc = vfs_get_root_dir(&root_dir);
	if (rc != 0) {
		goto done;
	}

	/* find the mountpoint directory */
	rc = vfs_lookup_inode(root_dir, path, &mountpoint);
	if (rc != 0) {
		goto done;
	}

	/* it must be a directory... */
	if (mountpoint->type != VFS_DIR) {
		rc = ENOTDIR;
		goto done;
	}

	/* and it must not already have a filesystem mounted on it... */
	if (mountpoint->mount != 0) {
		rc = EEXIST;
		goto done;
	}

	/* now we can attempt to mount the filesystem. */
	mutex_lock(&s_fs_mutex);
	rc = vfs_do_mount(fs_driver_name, 0, init, opts, &mount_root_dir);
	if (rc == 0) {
		/* success */
		mountpoint->mount = mount_root_dir;

		/* root directory of mounted fs now has the mountpoint's
		 * parent as its parent */
		mount_root_dir->parent = mountpoint->parent;

		/* the mounted root directory adds a ref to the directory
		 * it's mounted on, as well as the directories back to
		 * the overall root directory */
		vfs_adjust_refcounts(mountpoint, 1);
	}

	mutex_unlock(&s_fs_mutex);

done:
	vfs_release_ref(mountpoint);
	vfs_release_ref(root_dir);
	return rc;
}

/*
 * Get a pointer to the root directory.
 * Stores a pointer to the root directory in *p_dir and adds a reference if successful.
 * Returns EEXIST if the root filesystem hasn't been mounted.
 */
int vfs_get_root_dir(struct inode **p_dir)
{
	int rc = 0;

	mutex_lock(&s_fs_mutex);

	/* return EEXIST if root filesystem hasn't been mounted yet */
	if (!s_root_dir) {
		rc = EEXIST;
		goto done;
	}

	/* return ptr to root dir in p_dir and add a reference */
	*p_dir = s_root_dir;
	(*p_dir)->refcount++;

done:
	mutex_lock(&s_fs_mutex);

	return rc;
}

/*
 * Starting from the given start directory, look up the inode
 * named by given relative path.  If successful, a pointer
 * to the named inode (with an incremented refcount) is stored in p_inode,
 * and 0 is returned.  Otherwise, an error code is returned.
 */
int vfs_lookup_inode(struct inode *start_dir, const char *path, struct inode **p_inode)
{
	int rc = 0;
	struct inode *inode = start_dir, *child;
	char *name = 0;

	KASSERT(*path != '/'); /* must be a relative path! */
	KASSERT(start_dir->refcount > 0);

	/* check length of path */
	if (strnlen(path, VFS_PATHLEN_MAX + 1) > VFS_PATHLEN_MAX) {
		return EINVAL;
	}

	/* allocate a name buffer */
	name = mem_alloc(VFS_NAMELEN_MAX + 1);

	mutex_lock(&s_fs_mutex);

	/* increment the refcount of start inode and each tree ancestor */
	vfs_adjust_refcounts(start_dir, 1);

	while (vfs_has_more_path_elements(path)) {
		/* extract one path element */
		if ((rc = vfs_get_next_path_element(&path, name)) != 0) {
			goto done;
		}

		/*
		 * FIXME: support continuing search in
		 *        mounted filesystem.
		 */

		/* current inode needs to be a directory */
		if (inode->type != VFS_DIR) {
			rc = ENOTDIR;
			goto done;
		}

		/* lock dir */
		vfs_lock_dir(inode);

		/* look up child */
		rc = vfs_lookup_child(inode, name, &child);

		/* unlock dir */
		vfs_unlock_dir(inode);

		if (rc != 0) {
			/* child not found */
			goto done;
		}

		/* continue search in child */
		inode = child;
		inode->refcount++;
	}

	/* success: the path is empty and we have located the named inode */
	KASSERT(!vfs_has_more_path_elements(path));
	KASSERT(inode != 0);
	KASSERT(inode->refcount > 0);
	*p_inode = inode;

done:
	if (rc != 0) {
		/* failed search: decrement refcounts of current inode
		 * and all tree ancestors */
		vfs_adjust_refcounts(inode, -1);
	}

	mutex_unlock(&s_fs_mutex);

	mem_free(name);

	return rc;
}

/*
 * Release the reference to given inode.
 */
void vfs_release_ref(struct inode *inode)
{
	/* To allow robust cleanup code, ignore null pointer */
	if (inode == 0) {
		return;
	}

	mutex_lock(&s_fs_mutex);

	KASSERT(inode->refcount > 0);

	/* decremenent refcounts of inode and all tree ancestors */
	vfs_adjust_refcounts(inode, -1);

	/*
	 * Note: we allow the refcount of a inode to reach 0.
	 * It will be removed from the tree only if the
	 * underlying filesystem file is deleted.
	 */

	mutex_unlock(&s_fs_mutex);
}

int vfs_read(struct inode *inode, void *buf, size_t len)
{
	/* TODO */
	return -1;
}

int vfs_write(struct inode *inode, void *buf, size_t len)
{
	/* TODO */
	return -1;
}

int vfs_close(struct inode *inode)
{
	/* TODO */
	return -1;
}

/*
 * Register a filesystem driver.
 */
int vfs_register_fs_driver(struct fs_driver *fs)
{
	mutex_lock(&s_driver_list_mutex);

	fs->next = s_driver_list;
	s_driver_list = fs;

	mutex_unlock(&s_driver_list_mutex);

	return 0;
}

/*
 * Create an fs_instance object.
 *
 * Parameters:
 *   ops - the fs_instance operations
 *   p   - fs-specific data
 *   p_fs_inst - where the pointer to the new fs_instance object should be returned
 */
int vfs_fs_instance_create(struct fs_instance_ops *ops, void *p, struct fs_instance **p_fs_inst)
{
	struct fs_instance *fs_inst;

	fs_inst = mem_alloc(sizeof(struct fs_instance));
	fs_inst->ops = ops;
	fs_inst->p = p;

	*p_fs_inst = fs_inst;
	return 0;
}

/*
 * Create an inode object.
 *
 * Parameters:
 *   ops - the inode_ops
 *   fs_inst - the fs_instance the inode belongs to
 *   parent - the parent inode
 *   type - the type of inode (file or directory)
 *   name - string containing name of file or directory
 *   p_inode - where the pointer to the new inode object should be returned
 */
int vfs_inode_create(
	struct inode_ops *ops, struct fs_instance *fs_inst, struct inode *parent,
	vfs_inode_type_t type, char *name,
	void *p,
	struct inode **p_inode)
{
	struct inode *inode;

	inode = mem_alloc(sizeof(struct inode));
	inode->ops = ops;
	inode->fs_inst = fs_inst;
	inode->parent = parent;
	inode->type = type;
	inode->name = name;
	inode->p = p;
	/* can omit initialization of other fields because
	 * mem_alloc() has already zeroed the buffer */

	*p_inode = inode;
	return 0;
}
