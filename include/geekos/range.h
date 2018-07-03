/*
 * GeekOS - range checking and other integer math operations
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

#ifndef RANGE_H
#define RANGE_H

#include <geekos/types.h>

unsigned range_umin(unsigned a, unsigned b);
unsigned range_umax(unsigned a, unsigned b);

bool range_is_valid_u32(u32_t start, u32_t num, u32_t total);
int range_bit_count(unsigned val);
bool range_is_power_of_two(unsigned val);

#endif /* RANGE_H */

