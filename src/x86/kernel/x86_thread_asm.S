/*
 * GeekOS - x86 thread support
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

#include <arch/thread.h>
#include <arch/cpu.h>

/*
 * Context switch to a new thread.
 * Assumes that the previous thread has been added to a
 * wait queue or the run queue if appropriate.
 * Params:
 * - the pointer to the new thread
 */
.globl thread_switch_to
.align 8
thread_switch_to:
	/*
	 * Set up thread for later reactivation using an iret instruction.
	 * We start from the current stack:
	 *
	 * [lower addresses]
	 *   ret addr     <-- ESP
	 *   thread_ptr
	 * [higher addresses]
	 *
	 * We rearrange the stack to look like this:
	 *
	 * [lower addresses]
	 *   ret addr     <-- ESP
	 *   cs
	 *   eflags
	 *   thread_ptr
	 * [higher addresses]
	 */
	pushl	%eax		/* save eax */
	movl	4(%esp), %eax	/* get return address */
	movl	%eax, -4(%esp)	/* move return addr down 8 bytes from orig loc */
	addl	$8, %esp
	pushfl			/* put eflags where return address was */
	movl	-4(%esp), %eax	/* restore saved value of eax */
	pushl	$KERN_CS	/* push cs selector */
	sub	$4, %esp	/* point stack ptr at return address */

	/* push fake error code and interrupt number */
	pushl	$0
	pushl	$0

	/* save registers */
	THREAD_SAVE_REGISTERS

	/* store current %esp in stack_ptr field of current thread */
	movl	g_current, %eax
	movl	%esp, THREAD_STACK_PTR_OFFSET(%eax)

	/* TODO: clear num_ticks */

	/* load pointer to new thread into eax, skipping
	   over the thread_context currently on the stack */
	movl	THREAD_CONTEXT_SIZE(%esp), %eax

	/* switch to stack of new thread */
	movl	THREAD_STACK_PTR_OFFSET(%eax), %esp

	/* make the new thread the current thread */
	movl	%eax, g_current

	/* TODO: switch to address space of new thread */

	/* restore registers */ 
	THREAD_RESTORE_REGISTERS

	/* skip interrupt number and error code */
	addl	$8, %esp

	/* return to new thread */
	iret
