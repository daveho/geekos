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

#include <geekos/range.h>

/*
 * Return the minimum of two unsigned values.
 */
unsigned range_umin(unsigned a, unsigned b)
{
	if (a < b) {
		return a;
	} else {
		return b;
	}
}

/*
 * Return the maximum of two unsigned values.
 */
unsigned range_umax(unsigned a, unsigned b)
{
	if (a > b) {
		return a;
	} else {
		return b;
	}
}

/*
 * Return true iff start + num <= total, false otherwise,
 * correctly accounting for possible overflow conditions.
 */
bool range_is_valid_u32(u32_t start, u32_t num, u32_t total)
{
	if (num > total) {
		return false;
	}

	return start <= (total - num);
}

/*
 * Return the number of bits set to 1 in the given value.
 */
int range_bit_count(unsigned val)
{
	int bit_count = 0;

	while (val != 0) {
		bit_count += (val & 1);
		val >>= 1;
	}

	return bit_count;
}

/*
 * Return true iff given value is a power of 2.
 */
bool range_is_power_of_two(unsigned val)
{
	return range_bit_count(val) == 1;
}
