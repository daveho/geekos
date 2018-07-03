/*
 * GeekOS - block devices
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

#ifndef GEEKOS_BLOCKDEV_H
#define GEEKOS_BLOCKDEV_H

#include <geekos/types.h>
#include <geekos/thread.h>
#include <geekos/lba.h>

/* request type */
typedef enum { BLOCKDEV_REQ_READ, BLOCKDEV_REQ_WRITE } blockdev_req_type_t;

/* request states */
typedef enum { BLOCKDEV_REQ_PENDING, BLOCKDEV_REQ_FINISHED } blockdev_req_state_t;

struct blockdev;

/*
 * A request for block I/O.
 */
struct blockdev_req {
	lba_t lba;                     /* LBA of first block */
	unsigned num_blocks;           /* number of blocks requested */
	void *buf;                     /* memory buffer */
	blockdev_req_type_t type;      /* request type */
	blockdev_req_state_t state;    /* state of request */
	int rc;                        /* return code (when request completes) */
	struct thread_queue waitqueue; /* queue in which to wait for completion */
	struct blockdev *dev;          /* the block device */
	void *data;                    /* scratch pointer for use by driver */
};

/*
 * Block device operations.
 */
struct blockdev_ops {
	void (*post_request)(struct blockdev *dev, struct blockdev_req *req);
	ulong_t (*get_num_blocks)(struct blockdev *dev);
	blocksize_t (*get_block_size)(struct blockdev *dev);
	int (*close)(struct blockdev *dev);
};

/*
 * Block device base class.
 */
struct blockdev {
	struct blockdev_ops *ops;
	void *data; /* for use by driver */
};

/* block device functions */
struct blockdev_req *blockdev_create_request(lba_t lba, unsigned num_blocks, void *buf, blockdev_req_type_t type);
void blockdev_post_request(struct blockdev *dev, struct blockdev_req *req);
int blockdev_wait_for_completion(struct blockdev_req *req);
int blockdev_post_and_wait(struct blockdev *dev, struct blockdev_req *req);
void blockdev_notify_complete(struct blockdev_req *req, int rc);

int blockdev_read_sync(struct blockdev *dev, lba_t lba, unsigned num_blocks, void *buf);
int blockdev_write_sync(struct blockdev *dev, lba_t lba, unsigned num_blocks, void *buf);

blocksize_t blockdev_get_block_size(struct blockdev *dev);
ulong_t blockdev_get_num_blocks(struct blockdev *dev);
int blockdev_close(struct blockdev *dev);

#endif /* ifndef GEEKOS_BLOCKDEV_H */

