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

#ifndef GEEKOS_LBA_H
#define GEEKOS_LBA_H

#include <stddef.h>
#include <geekos/types.h>

/* block size data type */
typedef struct { unsigned size; } blocksize_t;
#define INIT_BLOCKSIZE(sz) { .size = (sz) }

/* logical block address type */
typedef struct { u32_t val; } lba_t;

/* blocksize_t functions */
blocksize_t blocksize_from_size(unsigned size);
unsigned blocksize_size(blocksize_t blocksize);

/* lba_t functions */
lba_t lba_from_num(u32_t num);
lba_t lba_add_offset(lba_t start, u32_t offset);
u32_t lba_num(lba_t lba);
bool lba_is_range_valid(lba_t start, u32_t num_blocks, u32_t total_blocks);
size_t lba_block_offset_in_bytes(lba_t lba, blocksize_t block_size);
size_t lba_range_size_in_bytes(u32_t num_blocks, blocksize_t block_size);
size_t lba_get_num_blocks_in_table(blocksize_t block_size, u32_t num_entries, unsigned entry_size);
int lba_compare(lba_t lhs, lba_t rhs);
u32_t lba_num_blocks_in_range(lba_t start, lba_t end);

#endif /* GEEKOS_LBA_H */

