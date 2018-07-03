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

#include <geekos/blockdev.h>
#include <geekos/mem.h>
#include <geekos/int.h>
#include <geekos/errno.h>

/* ------------------- private implementation ------------------- */

static int blockdev_issue_sync(
	struct blockdev *dev, lba_t lba, unsigned num_blocks, void *buf, blockdev_req_type_t type)
{
	struct blockdev_req *req;
	blockdev_req_state_t state;

	req = blockdev_create_request(lba, num_blocks, buf, type);
	state = blockdev_post_and_wait(dev, req);
	KASSERT(state == BLOCKDEV_REQ_FINISHED);

	mem_free(req);

	return req->rc;
}

/* ------------------- public interface ------------------- */

struct blockdev_req *blockdev_create_request(lba_t lba, unsigned num_blocks, void *buf, blockdev_req_type_t type)
{
	struct blockdev_req *req;

	req = mem_alloc(sizeof(struct blockdev_req));
	req->lba = lba;
	req->num_blocks = num_blocks;
	req->buf = buf;
	req->type = type;
	req->state = BLOCKDEV_REQ_PENDING;
	req->rc = 0;
	thread_queue_clear(&req->waitqueue);
	req->dev = 0;
	req->data = 0;

	return req;
}

void blockdev_post_request(struct blockdev *dev, struct blockdev_req *req)
{
	req->dev = dev;
	dev->ops->post_request(dev, req);
}

int blockdev_wait_for_completion(struct blockdev_req *req)
{
	bool iflag = int_begin_atomic();
	while (req->state == BLOCKDEV_REQ_PENDING) {
		thread_wait(&req->waitqueue);
	}
	int_end_atomic(iflag);
	return req->rc;
}

int blockdev_post_and_wait(struct blockdev *dev, struct blockdev_req *req)
{
	blockdev_post_request(dev, req);
	return blockdev_wait_for_completion(req);
}

void blockdev_notify_complete(struct blockdev_req *req, int rc)
{
	bool iflag = int_begin_atomic();
	req->state = BLOCKDEV_REQ_FINISHED;
	req->rc = rc;
	thread_wakeup(&req->waitqueue);
	int_end_atomic(iflag);
}

int blockdev_read_sync(struct blockdev *dev, lba_t lba, unsigned num_blocks, void *buf)
{
	return blockdev_issue_sync(dev, lba, num_blocks, buf, BLOCKDEV_REQ_READ);
}

int blockdev_write_sync(struct blockdev *dev, lba_t lba, unsigned num_blocks, void *buf)
{
	return blockdev_issue_sync(dev, lba, num_blocks, buf, BLOCKDEV_REQ_WRITE);
}

blocksize_t blockdev_get_block_size(struct blockdev *dev)
{
	return dev->ops->get_block_size(dev);
}

int blockdev_close(struct blockdev *dev)
{
	if (dev == 0) {
		return 0;
	}

	return dev->ops->close(dev);
}

