/*
 * GeekOS - generic timer functions
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

#include <geekos/timer.h>
#include <geekos/thread.h>

/* number of ticks in one quantum */
#define TIMER_QUANTUM 4

volatile u32_t g_numticks;

/*
 * Process a single timer tick.
 * Called from timer interrupt handler function.
 */
void timer_process_tick(void)
{
	/* update global tick counter and current thread's tick counter */
	++g_numticks;
	g_current->num_ticks++;

	/* if current thread has used an entire quantum, force new thread to be scheduled */
	if (g_current->num_ticks > TIMER_QUANTUM) {
		g_need_reschedule = 1;
	}
}
