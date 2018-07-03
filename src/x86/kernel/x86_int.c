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

#include <geekos/types.h>
#include <geekos/kassert.h>
#include <geekos/int.h>
#include <arch/cpu.h>
#include <arch/thread.h>
#include <arch/int.h>

int_handler_t *g_int_handler_table[INT_NUM_INTERRUPTS];

static struct x86_interrupt_gate s_idt[INT_NUM_INTERRUPTS];

static void int_unexpected_int_handler(struct thread_context *context)
{
	cons_printf("Unexpected interrupt %d\n", context->int_num);
	thread_dump_context(context);
	HALT();
}

void int_init(void)
{
	extern char int_handler_stub_vector, int_handler_stub_vector_end;
	int num_handler_stubs = (&int_handler_stub_vector_end - &int_handler_stub_vector);
	int i;
	u16_t limit_and_base[3];

	PANIC_IF(num_handler_stubs / INT_HANDLER_STUB_LEN != INT_NUM_INTERRUPTS,
		"Interrupt handler stub vector has unexpected size");

	/* create and load IDT */
	for (i = 0; i < INT_NUM_INTERRUPTS; i++) {
		int stub_offset = i * INT_HANDLER_STUB_LEN;
		x86_init_int_gate(&s_idt[i],
			(ulong_t) (&int_handler_stub_vector + stub_offset), PRIV_KERN);
	}
	limit_and_base[0] = sizeof(s_idt);
	limit_and_base[1] = ((ulong_t) s_idt) & 0xFFFF;
	limit_and_base[2] = ((ulong_t) s_idt) >> 16;
	x86_load_idtr(limit_and_base);

	/* initialize C interrupt handler function table */
	for (i = 0; i < INT_NUM_INTERRUPTS; i++) {
		g_int_handler_table[i] = &int_unexpected_int_handler;
	}
}

void int_install_handler(int int_num, int_handler_t *handler)
{
	KASSERT(int_num >= 0 && int_num < INT_NUM_INTERRUPTS);
	g_int_handler_table[int_num] = handler;
}

bool int_enabled(void)
{
	u32_t eflags;
	eflags = x86_get_eflags();
	return (eflags & EFLAGS_IF);
}

void int_enable__(void)
{
	__asm__ __volatile__ ("sti");
}

void int_disable__(void)
{
	__asm__ __volatile__ ("cli");
}

#if 0
void int_dump_stack_word(u32_t *addr, u32_t word)
{
	cons_printf("word: %p:%lx\n", addr, word);
}
#endif
