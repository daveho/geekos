/*
 * GeekOS - x86 CPU support
 * Copyright (c) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 */

/*
 * Information sources:
 * - Protected Mode Software Architecture by Tom Shanley, ISBN 020155447X.
 * - Intel IA-32 manual, Vol. 3
 */

#ifndef ARCH_CPU_H
#define ARCH_CPU_H

#ifndef ASM

#include <geekos/types.h>

struct x86_tss {
	/*
	 * Link to nested task.  For example, if an interrupt is handled
	 * by a task gate, the link field will contain the selector for
	 * the TSS of the interrupted task.
	 */
	u16_t link;
	u16_t reserved1;

	/* Stacks for privilege levels.  esp0/ss0 specifies the kernel stack. */
	u32_t esp0;
	u16_t ss0;
	u16_t reserved2;
	u32_t esp1;
	u16_t ss1;
	u16_t reserved3;
	u32_t esp2;
	u16_t ss2;
	u16_t reserved4;

	/* Page directory register. */
	u32_t cr3;

	/* General processor registers. */
	u32_t eip;
	u32_t eflags;
	u32_t eax;
	u32_t ecx;
	u32_t edx;
	u32_t ebx;
	u32_t esp;
	u32_t ebp;
	u32_t esi;
	u32_t edi;

	/* Segment registers and padding. */
	u16_t es;
	u16_t reserved5;
	u16_t cs;
	u16_t reserved6;
	u16_t ss;
	u16_t reserved7;
	u16_t ds;
	u16_t reserved8;
	u16_t fs;
	u16_t reserved9;
	u16_t gs;
	u16_t reserved10;

	/* GDT selector for the LDT descriptor. */
	u16_t ldt;
	u16_t reserved11;

	/*
	 * The debug trap bit causes a debug exception upon a switch
	 * to the task specified by this TSS.
	 */
	uint_t debug_trap : 1;
	uint_t reserved12 : 15;

	/* Offset in the TSS specifying where the io map is located. */
	u16_t io_map_base;
};

/*
 * A segment descriptor is an entry in the GDT or an LDT.
 */
struct x86_segment_descriptor {
	u32_t words[2];
};

/*
 * An interrupt gate is an entry in the IDT.
 */
struct x86_interrupt_gate {
	u16_t offset_low          ;
	u16_t segment_selector    ;
	uint_t reserved        : 5;
	uint_t signature       : 8;
	uint_t dpl             : 2;
	uint_t present         : 1;
	u16_t offset_high         ;
};

/*
 * CPUID information.
 */
struct x86_cpuid_info {
	/* function 0: vendor id string */
	char vendor_id[12];

	/* function 1: processor signature in eax register */
	struct {
		unsigned stepping_id     : 4;
		unsigned model_number    : 4;
		unsigned family_code     : 4;
		unsigned type            : 2;
		unsigned unused1         : 2;
		unsigned extended_model  : 4;
		unsigned extended_family : 8;
		unsigned unused2         : 4;
	} proc_sig;

	/* function 1: feature flags in edx register */
	struct {
		unsigned fpu : 1;
		unsigned vme : 1;
		unsigned de : 1;
		unsigned pse : 1;
		unsigned tsc : 1;
		unsigned msr : 1;
		unsigned pae : 1;
		unsigned mce : 1;
		unsigned cx8 : 1;
		unsigned apic : 1;
		unsigned reserved1 : 1;
		unsigned sep : 1;
		unsigned mtrr : 1;
		unsigned pge : 1;
		unsigned mca : 1;
		unsigned cmov : 1;
		unsigned pat : 1;
		unsigned pse36 : 1;
		unsigned psn : 1;
		unsigned clfsh : 1;
		unsigned reserved2 : 1;
		unsigned ds : 1;
		unsigned acpi : 1;
		unsigned mmx : 1;
		unsigned fxsr : 1;
		unsigned sse : 1;
		unsigned sse2 : 1;
		unsigned ss : 1;
		unsigned htt : 1;
		unsigned tm : 1;
		unsigned ia64 : 1;
		unsigned pbe : 1;
	} feature_info_edx;

	/* function 1: feature flags in ecx register */
	struct {
		unsigned sse3 : 1;
		unsigned reserved1 : 1;
		unsigned dtes64 : 1;
		unsigned monitor : 1;
		unsigned ds_cpl : 1;
		unsigned vmx : 1;
		unsigned smx : 1;
		unsigned est : 1;
		unsigned tm2 : 1;
		unsigned ssse3 : 1;
		unsigned cnxt_id : 1;
		unsigned reserved2 : 2;
		unsigned cx16 : 1;
		unsigned xtpr : 1;
		unsigned pdcm : 1;
		unsigned reserved3 : 2;
		unsigned dca : 1;
		unsigned sse4_1 : 1;
		unsigned sse4_2 : 1;
		unsigned reserved4 : 11;
	} feature_info_ecx;
};

/* initialize segment descriptors */
void x86_seg_init_null(struct x86_segment_descriptor *desc,
	u32_t base, u32_t num_pages, int priv);
void x86_eg_init_code(struct x86_segment_descriptor *desc,
	u32_t base, u32_t num_pages, int priv);
void x86_seg_init_data(struct x86_segment_descriptor *desc,
	u32_t base, u32_t num_pages, int priv);
void x86_seg_init_tss(struct x86_segment_descriptor *desc, struct x86_tss *tss);

/* initialize interrupt gate */
void x86_init_int_gate(struct x86_interrupt_gate *gate, ulong_t addr, int dpl);

/* initialize GDT */
void x86_seg_init_gdt(void);
void x86_load_gdtr(u16_t *limit_and_base);

/* initialize IDT */
void x86_load_idtr(u16_t *limit_and_base);

/* load TSS */
void x86_load_tr(struct x86_segment_descriptor *tss_desc);

/* get/set eflags */
u32_t x86_get_eflags(void);
void x86_set_eflags(u32_t eflags);

/* get/set cr0 */
u32_t x86_get_cr0(void);
void x86_set_cr0(u32_t cr0);

/* get/set cr3 (the page directory base physical address) */
u32_t x86_get_cr3(void);
void x86_set_cr3(u32_t cr3);

/* get/set cr4 */
u32_t x86_get_cr4(void);
void x86_set_cr4(u32_t cr4);

/* CPUID */
bool x86_cpuid(struct x86_cpuid_info *cpuid_info);
#endif

#define PRIV_KERN 0
#define PRIV_USER 3

#define SEL_GDT 0
#define SEL_LDT (1 << 2)

#define SELECTOR(index, table, rpl) \
((((index) & 0x1FFF) << 3) | (table) | ((rpl) & 3))

/* selectors for kernel code/data */
#define KERN_CS SELECTOR(1, SEL_GDT, 0)
#define KERN_DS SELECTOR(2, SEL_GDT, 0)

/*
 * Bits in eflags register.
 */
#define EFLAGS_IF              (1 << 9)   /* interrupts enabled */
#define EFLAGS_CPUID_SUPPORTED (1 << 21)  /* CPUID instruction supported */

/*
 * Bits in cr0 register.
 */
#define CR0_PG        (1 << 31)    /* enable paging */

/*
 * Bits in cr4 register.
 */
#define CR4_PSE        (1 << 4)    /* page size extensions */

#endif /* ARCH_CPU_H */
