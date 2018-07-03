/*
 * GeekOS - x86 memory constants
 */

#ifndef ARCH_MEM_H
#define ARCH_MEM_H

/* page size constants */
#define PAGE_POWER 12
#define PAGE_SIZE (1 << PAGE_POWER)

/* bitwise-and of an address with PAGE_MASK yields the address
 * of the page containing that address */
#define PAGE_MASK (~0UL << PAGE_POWER)

/* start of ISA hole (640K) */
#define ISA_HOLE_START 0xA0000

/* end of ISA hole (1 MB) */
#define ISA_HOLE_END 0x100000

/* Stack for initial kernel thread is just above the ISA hole */
#define KERN_STACK 0x100000

#endif /* ARCH_MEM_H */
