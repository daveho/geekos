/*
 * GeekOS - x86 boot constants
 */

#ifndef ARCH_BOOT_H
#define ARCH_BOOT_H

/* Multiboot magic words */
#define MB_HEADER_MAGIC 0x1BADB002
#define MB_LOADER_MAGIC 0x2BADB002

/* Flags in multiboot header */
#define MB_FLAG_PAGEALIGN   (1 << 0)
#define MB_FLAG_MEMMAP      (1 << 1)
#define MB_FLAG_VIDMODE     (1 << 2)

#endif /* ARCH_BOOT_H */
