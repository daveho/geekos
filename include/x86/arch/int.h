/*
 * GeekOS - x86 register save/restore code
 *
 * Copyright (C) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 */

#ifndef ARCH_INT_H
#define ARCH_INT_H

/*
 * Keep these definitions up-to-date with thread_context struct
 * in arch/thread.h!
 */

/* Number of bytes consumed by one interrupt handler stub. */
#define INT_HANDLER_STUB_LEN 16

/* Number of interrupts defined */
#define INT_NUM_INTERRUPTS 64

#endif /* ARCH_INT_H */
