/*
 * GeekOS - hardware interrupt support
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

#ifndef GEEKOS_IRQ_H
#define GEEKOS_IRQ_H

#include <geekos/int.h>
#include <geekos/thread.h>
#include <arch/irq.h>

/* Architecture-dependent functions */
void irq_init(void);
void irq_install_handler(int irq, int_handler_t *handler);
irq_mask_t irq_get_mask(void);
void irq_set_mask(irq_mask_t mask);
void irq_enable(int irq);
void irq_disable(int irq);
void irq_begin(struct thread_context *context);
void irq_end(struct thread_context *context);

#endif /* GEEKOS_IRQ_H */
