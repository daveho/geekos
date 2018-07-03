/*
 * GeekOS - x86 CPU support
 *
 * Copyright (c) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
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

#include <arch/cpu.h>

.text

/* Load GDTR to install a new GDT. */
.globl x86_load_gdtr
.align 8
x86_load_gdtr:
	/* install the new GDT */
	movl	4(%esp), %eax
	lgdt	(%eax)

	/* reload segment registers */
	movw	$KERN_DS, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss
	ljmp	$KERN_CS, $here
here:	ret

/* Load IDTR to install a new IDT. */
.globl x86_load_idtr
.align 8
x86_load_idtr:
	movl	4(%esp), %eax
	lidt	(%eax)
	ret
