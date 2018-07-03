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

#ifndef GEEKOS_BLOCKDEV_PAGER_H
#define GEEKOS_BLOCKDEV_PAGER_H

#include <geekos/lba.h>

struct blockdev;
struct vm_pager;

int blockdev_pager_create(struct blockdev *dev, lba_t start, u32_t num_blocks, struct vm_pager **p_pager);

#endif /* GEEKOS_BLOCKDEV_PAGER_H */
