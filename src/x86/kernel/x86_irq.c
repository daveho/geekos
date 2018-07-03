/*
 * GeekOS - x86 hardware interrupt support
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

#include <stdbool.h>
#include <geekos/types.h>
#include <geekos/irq.h>
#include <arch/ioport.h>

/*
 * i8259A definitions 
 */
#define ICW1        0x11 /* ICW1 - ICW4 needed, cascade mode, interval=8,
                          * edge triggered. (I think interval is irrelevant
                          * for x86.) */
#define ICW2_MASTER 0x20 /* put IRQs 0-7 at 0x20 (above Intel reserved ints) */
#define ICW2_SLAVE  0x28 /* put IRQs 8-15 at 0x28 */
#define ICW3_MASTER 0x04 /* IR2 connected to slave */
#define ICW3_SLAVE  0x02 /* slave has id 2 */
#define ICW4        0x01 /* 8086 mode, no auto-EOI, non-buffered mode,
                          * not special fully nested mode */

/*
 * Get the master and slave parts of an IRQ mask.
 */
#define MASTER(mask) ((mask) & 0xff)
#define SLAVE(mask) (((mask)>>8) & 0xff)

/*
 * External IRQs range from 32-47
 */
#define FIRST_IRQ 32
#define NUM_IRQS 16
#define VALID_IRQ(irq) ((irq) >= 0 && (irq) < NUM_IRQS)

/*
 * Current IRQ mask.
 */
static irq_mask_t s_irqmask;

/*
 * Initialize i8259A interrupt controllers.
 */
void irq_init(void)
{
	ioport_outb(0x20, ICW1);        /* ICW1 to master */
	ioport_outb(0xA0, ICW1);        /* ICW1 to slave */
	ioport_outb(0x21, ICW2_MASTER); /* ICW2 to master */
	ioport_outb(0xA1, ICW2_SLAVE);  /* ICW2 to slave */
	ioport_outb(0x21, ICW3_MASTER); /* ICW3 to master */
	ioport_outb(0xA1, ICW3_SLAVE);  /* ICW3 to slave */
	ioport_outb(0x21, ICW4);        /* ICW4 to master */
	ioport_outb(0xA1, ICW4);        /* ICW4 to slave */
	ioport_outb(0xA1, 0xFF);        /* mask all ints in slave; OCW1 to slave */
	ioport_outb(0x21, 0xFB);        /* mask all ints but 2 in master; OCW1 to master */

	s_irqmask = 0xfffb;
}

void irq_install_handler(int irq, int_handler_t *handler)
{
	KASSERT(VALID_IRQ(irq));
	int_install_handler(FIRST_IRQ + irq, handler);
}

irq_mask_t irq_get_mask(void)
{
	return s_irqmask;
}

void irq_set_mask(irq_mask_t mask)
{
	u8_t oldmask, newmask;

	oldmask = MASTER(s_irqmask);
	newmask = MASTER(mask);
	if (newmask != oldmask) {
		ioport_outb(0x21, newmask);
	}

	oldmask = SLAVE(s_irqmask);
	newmask = SLAVE(mask);
	if (newmask != oldmask) {
		ioport_outb(0xA1, newmask);
	}

	s_irqmask = mask;
}

#define MODIFY_IRQ_MASK(irq, update) \
do { \
	bool iflag = int_begin_atomic(); \
	irq_mask_t mask = irq_get_mask(); \
	KASSERT(VALID_IRQ(irq)); \
	mask update; \
	irq_set_mask(mask); \
	int_end_atomic(iflag); \
} while (0)

void irq_enable(int irq)
{
	MODIFY_IRQ_MASK(irq, &= ~(1 << irq));
}

void irq_disable(int irq)
{
	MODIFY_IRQ_MASK(irq, |= (1 << irq));
}

void irq_begin(struct thread_context *context)
{
	/* currently a no-op */
}

void irq_end(struct thread_context *context)
{
	int irq = context->int_num - FIRST_IRQ;
	u8_t command = 0x60 | (irq & 0x7);

	KASSERT(VALID_IRQ(irq));

	if (irq < 8) {
		/* Specific EOI to master PIC */
		ioport_outb(0x20, command);
	} else {
		/* Specific EOI to slave PIC, then to master (cascade line) */
		ioport_outb(0xA0, command);
		ioport_outb(0x20, 0x62);
	}
}
