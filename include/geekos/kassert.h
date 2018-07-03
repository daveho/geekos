/*
 * GeekOS - assertions and kernel panic
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

#ifndef GEEKOS_ASSERT_H
#define GEEKOS_ASSERT_H

#include <geekos/cons.h>

#ifndef ASM

#define HALT() while (1)

#ifndef NDEBUG
#define KASSERT(exp) \
do { \
	if (!(exp)) { \
		cons_printf("Assertion " #exp " failed at %s, line %d", __FILE__, __LINE__); \
		HALT(); \
	} \
} while (0)
#else
#define KASSERT(exp)
#endif

#define PANIC(msg) \
do { \
	cons_printf("panic: " msg); \
	HALT(); \
} while (0)

#define PANIC_IF(exp, msg) \
do { \
	if ((exp)) { \
		cons_printf("panic: " msg); \
		HALT(); \
	} \
} while (0)

#endif /* ASM */

#endif /* GEEKOS_ASSERT_H */
