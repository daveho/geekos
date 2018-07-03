/*
 * GeekOS - control transfer from boot loader to kernel
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

/*
 * This should be the first object file linked into the kernel,
 * since it contains the multiboot header
 */

#include <arch/boot.h>
#include <arch/mem.h>

/* Multiboot flags: request page-aligned loading and memory map */
#define MB_FLAGS (MB_FLAG_PAGEALIGN | MB_FLAG_MEMMAP)

.text

/* identify the kernel as multiboot-loadable */
multiboot_header:
	.long	MB_HEADER_MAGIC
	.long	MB_FLAGS
	.long	-(MB_HEADER_MAGIC + MB_FLAGS)

/*
 * _entry - the entry point
 * the bootloader will start execution of the kernel here
 */
.globl _entry
_entry:
	/* Set up stack */
	movl	$(KERN_STACK + PAGE_SIZE), %esp

	/* Push address of multiboot_info struct and magic constant */
	pushl	%ebx
	pushl	%eax

	/* Jump into kernel's main method! */
	call	geekos_main

	/* Control should never return from geekos_main() */
1:	jmp	1b
