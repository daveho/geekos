/*
 * GeekOS - block device pager
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

#include <geekos/mem.h>
#include <geekos/blockdev.h>
#include <geekos/vm.h>
#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/range.h>
#include <geekos/blockdev_pager.h>

/*
 * Private data stored in p field of the vm_pager object.
 */
struct blockdev_pager {
	struct blockdev *dev;
	lba_t start;
	u32_t num_blocks;
	unsigned num_blocks_per_page;
};

typedef int (blockdev_rw_op)(struct blockdev *dev, lba_t lba, unsigned num_blocks, void *buf);

static int blockdev_pager_rw_page(struct vm_pager *pager, void *buf, u32_t page_num,
	blockdev_rw_op *rw_func)
{
	int rc;
	struct blockdev_pager *blkdev_pager;
	lba_t io_start_lba;   /* io start LBA, inclusive */
	lba_t io_end_lba;     /* io end LBA, exclusive */
	lba_t range_end_lba;  /* last LBA in range covered by this blockdev_pager, exclusive */

	blkdev_pager = pager->p;

	io_start_lba = lba_add_offset(blkdev_pager->start, page_num * blkdev_pager->num_blocks_per_page);
	io_end_lba = lba_add_offset(io_start_lba, blkdev_pager->num_blocks_per_page);

	/*
	 * Special case: see if we're accessing
	 * an incomplete page of data at the end of the range
	 * of blocks covered by this pager.
	 * If so, set io_end_lba such that
	 * we avoid reading/writing past end of range.
	 */
	range_end_lba = lba_add_offset(blkdev_pager->start, blkdev_pager->num_blocks);
	if (lba_compare(range_end_lba, io_end_lba) < 0) {
		io_end_lba = range_end_lba;
	}

	/* Do the IO! */
	rc = rw_func(blkdev_pager->dev, io_start_lba, lba_num_blocks_in_range(io_start_lba, io_end_lba), buf);

	return rc;
}

static int blockdev_pager_read_page(struct vm_pager *pager, void *buf, u32_t page_num)
{
	return blockdev_pager_rw_page(pager, buf, page_num, &blockdev_read_sync);
}

static int blockdev_pager_write_page(struct vm_pager *pager, void *buf, u32_t page_num)
{
	return blockdev_pager_rw_page(pager, buf, page_num, &blockdev_write_sync);
}

struct vm_pager_ops s_blockdev_pager_ops = {
	.read_page = &blockdev_pager_read_page,
	.write_page = &blockdev_pager_write_page,
};

/*
 * Create a vm_pager that pages to/from a (range of) a block device.
 */
int blockdev_pager_create(struct blockdev *dev, lba_t start, u32_t num_blocks, struct vm_pager **p_pager)
{
	int rc = 0;
	struct blockdev_pager *blkdev_pager = 0;
	struct vm_pager *pager;
	u32_t dev_num_blocks;
	blocksize_t dev_block_size;

	dev_num_blocks = blockdev_get_num_blocks(dev);
	dev_block_size = blockdev_get_block_size(dev);

	/* sanity check */
	KASSERT(range_is_power_of_two(blocksize_size(dev_block_size)));
	KASSERT(range_is_power_of_two(PAGE_SIZE));

	/* make sure a valid range of blocks is specified */
	if (!lba_is_range_valid(start, num_blocks, dev_num_blocks)) {
		rc = EINVAL;
		goto done;
	}

	/* for now, don't attempt to handle cases where the
	 * device block size is greater than the page size */
	if (blocksize_size(dev_block_size) > PAGE_SIZE) {
		rc = EINVAL;
		goto done;
	}

	/* things look good */
	blkdev_pager = mem_alloc(sizeof(struct blockdev_pager));
	blkdev_pager->dev = dev;
	blkdev_pager->start = start;
	blkdev_pager->num_blocks = num_blocks;
	blkdev_pager->num_blocks_per_page = PAGE_SIZE / blocksize_size(dev_block_size);

	rc = vm_pager_create(&s_blockdev_pager_ops, blkdev_pager, &pager);
	if (rc != 0) {
		goto done;
	}

	/* success! */
	*p_pager = pager;

done:
	if (rc != 0) {
		mem_free(blkdev_pager);
	}
	return rc;
}
