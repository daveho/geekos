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

#include <geekos/string.h>
#include <geekos/types.h>
#include <geekos/thread.h>
#include <geekos/kassert.h>
#include <geekos/mem.h>
#include <geekos/int.h>
#include <arch/cpu.h>

/*
 * Dump the context of a thread context for debugging.
 */
void thread_dump_context(struct thread_context *context)
{
	cons_printf("  eax=%lx ebx=%lx ecx=%lx edx=%lx\n",
		context->eax, context->ebx, context->ecx, context->edx);
	cons_printf("  ebp=%lx esi=%lx edi=%lx\n",
		context->ebp, context->esi, context->edi);
	cons_printf("   ds=%lx  es=%lx  fs=%lx  gs=%lx\n",
		context->ds, context->es, context->fs, context->gs);
	cons_printf("  cs:eip=%lx:%lx\n", context->eip, context->cs);
	cons_printf("  eflags=%lx\n", context->eflags);
	cons_printf("  int=%d err=%lx\n", context->int_num, context->errorcode);
}

/*
 * Thread trampoline function.
 */
static void thread_run(thread_func_t *start_func, ulong_t arg)
{
	int_enable(); /* make sure interrupts are enabled */
	start_func(arg);
	thread_exit(0);
	KASSERT(false); /* not reached */
}

/*
 * Push an operand on a thread's stack.
 */
static void thread_stack_push(struct thread *thread, u32_t value)
{
	thread->stack_ptr -= 4;
	*((u32_t *) thread->stack_ptr) = value;
}

/*
 * Bootstrap a newly created thread for kernel-only execution.
 */
void thread_bootstrap(struct thread *thread, thread_func_t *start_func, ulong_t arg)
{
	/*
	 * We want to set up the thread's kernel stack to look like this:
	 *
	 * [lower addresses]
	 *  +----------+  <-- ESP (points to beginning of thread_context)
	 *  |          |
	 *  | thread_  |
	 *  | context  |  [EIP is &thread_run, ds/es are KERN_DS]
	 *  |          |
	 *  +----------+
	 *  |    0     |  [fake return address]
	 *  +----------+
	 *  | st.func  |  [addr. of start func]
	 *  +----------+
	 *  |   arg    |  [argument to thread_run, pass as arg to thread func]
	 *  +----------+
	 * [higher addresses]
	 *
	 * Basically, we're making it look like the thread was interrupted
	 * just before it got a chance to start executing thread_run().
	 */

	/* set up empty stack */
	thread->stack_ptr = (ulong_t) (((u8_t*) thread->stack) + THREAD_STACK_SIZE);

	/* push thread_run arguments and (fake) return address */
	thread_stack_push(thread, (u32_t) arg);
	thread_stack_push(thread, (u32_t) start_func);
	thread_stack_push(thread, 0);

	/* create the thread_context */
	{
		struct thread_context *context;

		/* push the thread_context and clear it */
		thread->stack_ptr -= sizeof(struct thread_context);
		context = (struct thread_context *) thread->stack_ptr;
		memset(context, '\0', sizeof(struct thread_context));

		/* set up registers */
		context->gs = KERN_DS;
		context->fs = KERN_DS;
		context->es = KERN_DS;
		context->ds = KERN_DS;
		context->eip = (u32_t) &thread_run;
		context->cs = KERN_CS;
		context->eflags = 0UL; /* thread starts with interrupts disabled */
	}
}
