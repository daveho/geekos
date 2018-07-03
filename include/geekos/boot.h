/*
 * GeekOS - kernel bootstrap support
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
 * See http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 *
 * The idea behind this file is that on non-x86 platforms
 * the bootstrap code should still construct at least a rudimentary
 * multiboot_info structure that the kernel can use to initialize
 * memory and essential hardware.
 */

#ifndef GEEKOS_BOOT_H
#define GEEKOS_BOOT_H

#include <geekos/types.h>
#include <arch/boot.h>

/* Flags in multiboot_information struct */
#define MB_INFO_FLAG_MEM         (1 << 0)
#define MB_INFO_FLAG_BOOTDEV     (1 << 1)
#define MB_INFO_FLAG_CMDLINE     (1 << 2)
#define MB_INFO_FLAG_MODS        (1 << 3)
#define MB_INFO_FLAG_AOUT        (1 << 4)
#define MB_INFO_FLAG_ELF         (1 << 5)
#define MB_INFO_FLAG_MMAP        (1 << 6)
#if 0
#define MB_INFO_FLAG_DRIVES      (1 << 7)
#define MB_INFO_FLAGS_CFGTAB     (1 << 8)
#define MB_INFO_FLAGS_BTLDR_NAME (1 << 9)
#define MB_INFO_FLAGS_APMTAB     (1 << 10)
#define MB_INFO_FLAGS_VBETAB     (1 << 11)
#endif

#ifndef ASM

struct multiboot_aout {
	u32_t tabsize;
	u32_t strsize;
	u32_t addr;
	u32_t reserved;
};

struct multiboot_elf {
	u32_t num;
	u32_t size;
	u32_t addr;
	u32_t shndx;
};

struct multiboot_info {
	u32_t flags;
	u32_t mem_lower;
	u32_t mem_upper;
	u32_t boot_device;
	u32_t cmdline;
	u32_t mods_count;
	u32_t mods_addr;
	union {
		struct multiboot_aout aout;
		struct multiboot_elf elf;
	} syms;
	u32_t mmap_length;
	u32_t mmap_addr;
#if 0
	u32_t drives_length;
	u32_t drives_addr;
	u32_t config_table;
	u32_t boot_loader_name;
	u32_t apm_table;
	u32_t vbe_control_info;
	u32_t vbe_mode_info;
	u32_t vbe_mode;
	u32_t vbe_interface_seg;
	u32_t vbe_interface_off;
	u32_t vbe_interface_len;
#endif
};

#endif /* ASM */

#endif /* GEEKOS_BOOT_H */
