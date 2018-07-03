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

#ifndef GEEKOS_PFAT_H
#define GEEKOS_PFAT_H

#include <geekos/types.h>

#define PFAT_MAGIC 0x77e2ef5aU

#define PFAT_NAMELEN_MAX 127      /* max filename length */
#define PFAT_DIR_ENTRY_SIZE 140   /* size in bytes of a directory entry */

/*
 * Bits in "bits" field of pfat_dir_entry
 */
#define PFAT_BIT_DIR          1   /* entry is a directory */

/*
 * Data stored in the first block of the filesystem.
 */
struct pfat_superblock {
	u32_t magic;               /* must contain PFAT_MAGIC */
	u32_t fat_lba;             /* lba of FAT (located after superblock) */
	u32_t fat_num_entries;     /* number of entries in FAT */
	u32_t first_cluster_lba;   /* lba of first cluster (storing file/dir data) */
	u32_t cluster_size;        /* bytes per cluster */
	u32_t root_dir_fat_index;  /* index of root directory's first FAT entry */

	/* other metainfo could go here... */
	char reserved[512 - 24];
};

/*
 * Each FAT entry corresponds to one allocation cluster.
 */
struct pfat_entry {
	uint_t allocated : 1;  /* entry has been allocated */
	uint_t next      : 31; /* index of next FAT entry in allocation chain */
};

/*
 * Stored in "next" field of pfat_entry to terminate allocation chain
 */
#define PFAT_END_OF_CHAIN 0x7FFFFFFF

/*
 * PFAT directory entry.
 */
struct pfat_dir_entry {
	u32_t fat_index;                  /* index of first FAT entry */
	u16_t bits;                       /* attributes */
	u16_t perms;                      /* file permissions */
	u16_t uid;                        /* uid of owner */
	u16_t gid;                        /* gid of group */
	char name[PFAT_NAMELEN_MAX + 1];  /* filename */
};

int pfat_init(void);

#endif /* GEEKOS_PFAT_H */
