/*
 * GeekOS - LBA (logical block address) type and operations
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

#include <geekos/range.h>
#include <geekos/blockdev.h>
#include <geekos/kassert.h>
#include <geekos/lba.h>

blocksize_t blocksize_from_size(unsigned size)
{
	blocksize_t blocksize = { .size = size };
	return blocksize;
}

unsigned blocksize_size(blocksize_t blocksize)
{
	return blocksize.size;
}

/*
 * Create an LBA from a number.
 */
lba_t lba_from_num(u32_t num)
{
	lba_t lba = { .val = num };
	return lba;
}

/*
 * Create an LBA by adding an offset to a start LBA.
 */
lba_t lba_add_offset(lba_t start, u32_t offset)
{
	lba_t result;
	u32_t dest;

	dest = start.val + offset;

	/* Overflow? */
	KASSERT(dest >= start.val);

	result.val = dest;
	return result;
}

/*
 * Get block number from LBA.
 */
u32_t lba_num(lba_t lba)
{
	return lba.val;
}

/*
 * Is the range of blocks described by given start address and
 * number of blocks valid, so that each block in the range
 * has an address less than the total number of blocks?
 */
bool lba_is_range_valid(lba_t start, u32_t num_blocks, u32_t total_blocks)
{
	return range_is_valid_u32(start.val, num_blocks, total_blocks);
}

/*
 * Get the offset in bytes of a block from the start of the
 * block device.
 */
size_t lba_block_offset_in_bytes(lba_t lba, blocksize_t block_size)
{
	return ((size_t) lba.val) * block_size.size;
}

/*
 * Get total size of a range of blocks in bytes.
 */
size_t lba_range_size_in_bytes(u32_t num_blocks, blocksize_t block_size)
{
	return ((size_t) num_blocks) * block_size.size;
}

/*
 * Get the total number of blocks spanned by a table
 * consisting of some number of entries of a specified size.
 *
 * Parameters:
 *   block_size - block size in bytes
 *   num_entries - number of entries in the table
 *   entry_size - size of a single table entry
 */
size_t lba_get_num_blocks_in_table(blocksize_t block_size, u32_t num_entries, unsigned entry_size)
{
	size_t table_size_in_bytes;
	size_t table_size_in_blocks;

	table_size_in_bytes = num_entries * entry_size;
	table_size_in_blocks = table_size_in_bytes / block_size.size;

	if (table_size_in_bytes % block_size.size != 0) {
		table_size_in_blocks++;
	}

	return table_size_in_blocks;
}

/*
 * Compare two LBAs.
 *
 * Parameters:
 *    lhs - left hand LBA
 *    rhs - right hand LBA
 *
 * Returns:
 *    -1 if lhs < rhs, 0 if lhs == rhs, 1 if lhs > rhs
 */
int lba_compare(lba_t lhs, lba_t rhs)
{
	if (lhs.val < rhs.val) {
		return -1;
	} else if (lhs.val > rhs.val) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * Get the number of blocks in given range.
 * Start of range is inclusive, and end of range is exclusive.
 *
 * Paremeters:
 *   start - LBA of start of range (inclusive)
 *   end - LBA of end of range (exclusive)
 *
 * Returns:
 *   Number of blocks in range.
 */
u32_t lba_num_blocks_in_range(lba_t start, lba_t end)
{
	KASSERT(start.val <= end.val);
	return end.val - start.val;
}
