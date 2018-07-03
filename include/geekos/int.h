/*
 * GeekOS - interrupt support
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

#ifndef GEEKOS_INT_H
#define GEEKOS_INT_H

#include <geekos/types.h>
#include <geekos/kassert.h>

#ifndef ASM

struct thread_context;

/*
 * Type of an interrupt handling function.
 * Takes a pointer to the saved thread context on the kernel stack.
 */
typedef void (int_handler_t)(struct thread_context *tcontext);

/* architecture-dependent functions */
void int_init(void);
void int_install_handler(int int_num, int_handler_t *handler);
bool int_enabled(void);
void int_enable__(void);
void int_disable__(void);

/* enable and disable interrupts (detecting improper nesting) */
#define int_enable() \
do { KASSERT(!int_enabled()); int_enable__(); } while (0)
#define int_disable() \
do { KASSERT(int_enabled()); int_disable__(); } while (0)

/* Support for int-atomic regions */
static __inline__ bool int_begin_atomic(void)
{
	bool iflag = int_enabled();
	if (iflag)
		int_disable();
	return iflag;
}

static __inline__ void int_end_atomic(bool iflag)
{
	if (iflag)
		int_enable();
}

#endif /* ASM */

#endif /* GEEKOS_INT_H */
