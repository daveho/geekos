/*
 * GeekOS - x86 virtual memory support
 * Copyright (C) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 * Copyright (c) 2003 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
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

#ifndef ARCH_VM_H
#define ARCH_VM_H

#define VM_NUM_PAGE_TABLE_ENTRIES  1024
#define VM_NUM_PAGE_DIR_ENTRIES    1024

#define VM_PT_SPAN_POWER        22
#define VM_PT_SPAN              (1 << VM_PT_SPAN_POWER)  /* Number of bytes of VM spanned by a page table */

/* index of entry for given virtual address in page directory */
#define VM_PAGE_DIR_INDEX(vaddr)      (((vaddr) >> 22) & 0x3ff)

/* index of entry for given virtual address in page table */
#define VM_PAGE_TABLE_INDEX(vaddr)    (((vaddr) >> 12) & 0x3ff)

/* base address of a physical page */
#define VM_PAGE_BASE_ADDR(addr)       ((addr) >> 12)

/*
 * Bits for flags field of pde_t and pte_t.
 */
#define VM_WRITE   1     /* Memory is writable */
#define VM_USER    2     /* Memory is accessible to user code */
#define VM_NOCACHE 8     /* Memory should not be cached */
#define VM_READ    0     /* Memory can be read (ignored for x86) */
#define VM_EXEC    0     /* Memory can be executed (ignored for x86) */

/*
 * Page directory entry datatype.
 * If marked as present, it specifies the physical address
 * and permissions of a page table.
 */
typedef struct {
	unsigned present     : 1;
	unsigned flags       : 4;
	unsigned accesed     : 1;
	unsigned reserved    : 1;
	unsigned page_size   : 1; /* if 1, refers to a 4M page */
	unsigned global_page : 1;
	unsigned kernel_info : 3;
	unsigned base_addr   : 20;
} pde_t;

/*
 * Page table entry datatype.
 * If marked as present, it specifies the physical address
 * and permissions of a page of memory.
 */
typedef struct {
	unsigned present       : 1;
	unsigned flags         : 4;
	unsigned accesed       : 1;
	unsigned dirty         : 1;
	unsigned pte_attribute : 1;
	unsigned global_page   : 1;
	unsigned kernel_info   : 3;
	unsigned base_addr     : 20;
} pte_t;

/*
 * Datatype representing the hardware error code
 * pushed onto the stack by the processor on a page fault.
 * The error code is stored in the "errorcode" field
 * of the thread_context struct.
 */
typedef struct {
	unsigned protection_violation : 1;
	unsigned write_fault          : 1;
	unsigned user_mode_fault      : 1;
	unsigned reserved_bit_fault   : 1;
	unsigned reserved             : 28;
} faultcode_t;

#endif /* ARCH_VM_H */
