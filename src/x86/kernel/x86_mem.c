/*
 * GeekOS - x86 page and heap memory management
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

#include <geekos/string.h>
#include <geekos/kassert.h>
#include <geekos/mem.h>
#include <arch/cpu.h>

/* symbol defined by the linker specifying the kernel code+data end address */
extern char end;

/* -------------------- Private -------------------- */

/* important memory addresses and sizes */
struct x86mem_layout {
	ulong_t kernel_end;          /* kernel end code/data address, page-aligned */
	ulong_t memsize_kb;          /* total KB of memory */
	ulong_t numframes;           /* total frames of memory */
	ulong_t framelist_numframes; /* number of frames needed to store the framelist */
};

/* Compute important addresses and sizes from the multiboot info struct */
static void x86mem_init_layout(struct multiboot_info *boot_record, struct x86mem_layout *layout)
{
	layout->kernel_end = mem_round_to_page((ulong_t) &end);
	layout->memsize_kb = 1024 + boot_record->mem_upper;
	layout->numframes = layout->memsize_kb / (PAGE_SIZE/1024);
	layout->framelist_numframes =
		mem_round_to_page(layout->numframes * sizeof(struct frame)) / PAGE_SIZE;
}

/* -------------------- Public -------------------- */

void mem_init_segments(void)
{
	x86_seg_init_gdt();
	/*cons_printf("Initialized kernel GDT\n");*/
}

/* create (uninitialized) array of frame structs, 1 per frame of physical memory */
void mem_create_framelist(struct multiboot_info *boot_record, struct frame **framelist,
	ulong_t *numframes)
{
	struct x86mem_layout layout;

	x86mem_init_layout(boot_record, &layout);
	*framelist = (struct frame *) layout.kernel_end;
	memset(*framelist, '\0', layout.framelist_numframes * PAGE_SIZE);
	*numframes = layout.numframes;
}

/* scan all regions of physical memory, assigning a state to each region */
void mem_scan_regions(struct multiboot_info *boot_record,
	scan_reg_func_t *scan_reg_func, void *data)
{
	struct x86mem_layout layout;
	ulong_t addr = 0UL;

	x86mem_init_layout(boot_record, &layout);

	/* preserve the BIOS data area */
	addr = scan_reg_func(addr, PAGE_SIZE, FRAME_UNUSED, data);
	/* available low memory */
	addr = scan_reg_func(addr, ISA_HOLE_START, FRAME_AVAIL, data);
	/* ISA hole */
	addr = scan_reg_func(addr, ISA_HOLE_END, FRAME_HW, data);
	/* initial kernel stack */
	addr = scan_reg_func(addr, ISA_HOLE_END+PAGE_SIZE, FRAME_KSTACK, data);
	/* kernel code/data and framelist structure */
	addr = scan_reg_func(addr, layout.kernel_end + layout.framelist_numframes, FRAME_KERN, data);
	/* available high memory */
	addr = scan_reg_func(addr, layout.numframes * PAGE_SIZE, FRAME_AVAIL, data);
}
