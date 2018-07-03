/*
 * GeekOS - x86 kernel thread saved context
 *
 * Copyright (C) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 */

#ifndef ARCH_THREAD_H
#define ARCH_THREAD_H

#include <arch/mem.h>
#include <geekos/types.h>

/* kernel thread stack size is one page */
#define THREAD_STACK_SIZE PAGE_SIZE

/* offset of stack pointer field in thread struct
  (must keep in sync with <geekos/thread.h> */
#define THREAD_STACK_PTR_OFFSET	0

/* size of thread_context (must keep in sync with struct below) */
#define THREAD_CONTEXT_SIZE	64

/*
 * Save CPU registers on a context switch or interrupt.
 */
#define THREAD_SAVE_REGISTERS \
	pushl	%eax ; \
	pushl	%ebx ; \
	pushl	%ecx ; \
	pushl	%edx ; \
	pushl	%esi ; \
	pushl	%edi ; \
	pushl	%ebp ; \
	pushl	%ds ; \
	pushl	%es ; \
	pushl	%fs ; \
	pushl	%gs

/*
 * Restore CPU registers when switching to a new thread or
 * returning to interrupted code.
 */
#define THREAD_RESTORE_REGISTERS \
	popl	%gs ; \
	popl	%fs ; \
	popl	%es ; \
	popl	%ds ; \
	popl	%ebp ; \
	popl	%edi ; \
	popl	%esi ; \
	popl	%edx ; \
	popl	%ecx ;\
	popl	%ebx ; \
	popl	%eax

/* Number of bytes occupied by saved registers on the stack */
#define THREAD_SAVED_REG_LEN 44

#ifndef ASM

/*
 * Saved kernel thread context.
 * Contains the values that will be on the stack
 * after an interrupt is handled: in other words,
 * what was happening when the interrupt occurred.
 */
struct thread_context {
	/* Saved processor registers. */
	u32_t gs;
	u32_t fs;
	u32_t es;
	u32_t ds;
	u32_t ebp;
	u32_t edi;
	u32_t esi;
	u32_t edx;
	u32_t ecx;
	u32_t ebx;
	u32_t eax;

	/* Interrupt number. */
	int int_num;

	/*
	 * If the processor doesn't explicitly push an error code,
	 * the interrupt handling code pushes a dummy value so
	 * that the stack layout is the same for all interrupts.
	 */
	u32_t errorcode;

	/* The processor pushes these values onto the stack on an interrupt. */
	u32_t eip;
	u32_t cs;
	u32_t eflags;
};

void thread_dump_context(struct thread_context *context);

#endif

#endif /* ARCH_THREAD_H */
