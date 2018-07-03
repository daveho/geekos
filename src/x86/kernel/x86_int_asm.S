/*
 * GeekOS - x86 interrupt support
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

#include <arch/int.h>
#include <arch/thread.h>
#include <arch/cpu.h>

/* -------------------- Macros and definitions -------------------- */

/*
 * Interrupt handler stub for interrupts without a processor error code.
 * We push a fake error code (0), the interrupt number,
 * and then jump to the common handler code.
 */
#define HANDLER_STUB(inum) \
.align INT_HANDLER_STUB_LEN ; \
	pushl $0 ; \
	pushl $(inum) ; \
	jmp int_handle_interrupt

/* Generate 2 handler stubs. */
#define HANDLER_STUB_X2(inum) \
HANDLER_STUB(inum) ; \
HANDLER_STUB(inum+1)

/* Generate 4 handler stubs. */
#define HANDLER_STUB_X4(inum) \
HANDLER_STUB_X2(inum) ; \
HANDLER_STUB_X2(inum+2)

/* Generate 8 handler stubs. */
#define HANDLER_STUB_X8(inum) \
HANDLER_STUB_X4(inum) ; \
HANDLER_STUB_X4(inum+4)

/*
 * Interrupt handler stub for interrupts with a processor error code.
 * Push the interrupt number and then jump to the common handler code.
 */
#define HANDLER_STUB_ERRCODE(inum) \
.align INT_HANDLER_STUB_LEN ; \
	pushl	$(inum) ; \
	jmp	int_handle_interrupt

/* -------------------- Interrupt handling code -------------------- */

.text

/*
 * Vector of interrupt handler stubs: the IDT entries will point to these.
 * 0-31 are reserved for CPU-generated interrupts.
 * We make 32-63 are available for external hardware interrupts
 * and for software interrupts.
 */
.globl int_handler_stub_vector
.globl int_handler_stub_vector_end
.align INT_HANDLER_STUB_LEN
int_handler_stub_vector:
HANDLER_STUB_X8(0)        /* 0-7 */
HANDLER_STUB_ERRCODE(8)
HANDLER_STUB(9)
HANDLER_STUB_ERRCODE(10)
HANDLER_STUB_ERRCODE(11)
HANDLER_STUB_ERRCODE(12)
HANDLER_STUB_ERRCODE(13)
HANDLER_STUB_ERRCODE(14)
HANDLER_STUB_X2(15)       /* 15-16 */
HANDLER_STUB_ERRCODE(17)
HANDLER_STUB_X2(18)       /* 18-19 */
HANDLER_STUB_X4(20)       /* 20-23 */
HANDLER_STUB_X8(24)       /* 24-31 */
HANDLER_STUB_X8(32)       /* 32-39 */
HANDLER_STUB_X8(40)       /* 40-47 */
HANDLER_STUB_X8(48)       /* 48-55 */
HANDLER_STUB_X8(56)       /* 56-63 */
.align INT_HANDLER_STUB_LEN
int_handler_stub_vector_end:

/*
 * Common interrupt handler code
 */
.globl int_handle_interrupt
.align 8
int_handle_interrupt:
	/* save thread context */
	THREAD_SAVE_REGISTERS             /* save registers of interrupted thread */

	/* make sure we're running in kernel's data segment */
	movw	$KERN_DS, %ax
	movw	%ax, %ds                  /* ensure ds is kernel data segment */
	movw	%ax, %es                  /* ensure es is kernel data segment */

	/*jmp	int_dump_stack*/          /* debugging: dump thread context on stack */

	/* find C interrupt handler function, call it */
	movl	THREAD_SAVED_REG_LEN(%esp), %esi /* store interrupt number in %esi */
	movl	$g_int_handler_table,%eax /* store address of C handler function table in %eax */
	movl	(%eax,%esi,4), %ebx       /* store address of C handler function in %ebx */
	pushl	%esp                      /* push address of thread_context on stack */
	call	*%ebx                     /* call C handler function */
	add	$4, %esp                  /* clear 1 argument from stack */

	/* if preemption is disabled, then current thread keeps running */
	cmpl	$0, g_preemption
	je	1f

	/* see if there is a new thread to run */
	cmpl	$0, g_need_reschedule
	je	1f

	/* clear g_need_reschedule */
	movl	$0, g_need_reschedule

	/* save stack pointer of current thread */
	movl	g_current, %ebp           /* load ptr to current thread into %ebp */
	movl	%esp, THREAD_STACK_PTR_OFFSET(%ebp) /* save stack pointer */

	/* put current thread back on the run queue */
	call	thread_relinquish_cpu     /* current thread is giving up the CPU */
	push	%ebp                      /* push ptr to current thread */
	call	thread_make_runnable      /* put current thread back on the runqueue */
	add	$4, %esp                  /* clear 1 argument from stack */

	/* choose a new thread, switch to its stack */
	call	thread_next_runnable      /* ptr to next runnable thread loaded into %eax */
	movl	THREAD_STACK_PTR_OFFSET(%eax), %esp /* switch to its stack */
	movl	%eax, g_current           /* it is now the current thread */

	/* restore thread context */
1:	THREAD_RESTORE_REGISTERS          /* restore registers of interrupted thread */
	add	$8, %esp                  /* skip interrupt number and error code */
	iret                              /* return from interrupt */

/*
.global int_dump_stack
.align 8
int_dump_stack:
	xorl	%ebp, %ebp
1:	pushl	%esp
	call	int_dump_stack_word
	addl	$8, %esp
	inc	%ebp
	cmpl	$16, %ebp
	jl	1b
2:	nop
	jmp	2b
*/
