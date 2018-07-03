/*
 * GeekOS - kernel threads
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

#ifndef GEEKOS_THREAD_H
#define GEEKOS_THREAD_H

#ifdef KERNEL

#ifndef ASM

#include <geekos/types.h>
#include <arch/thread.h>
#include <geekos/types.h>
#include <geekos/list.h>

struct thread;
struct thread_context;
struct process;

DECLARE_LIST(thread_queue, thread);

/* thread states */
typedef enum {
	THREAD_READY, THREAD_RUNNING, THREAD_WAITING, THREAD_EXITED, THREAD_KILLED
} thread_state_t;

/* thread creation mode: "attached" means parent will wait for child to exit */
typedef enum { THREAD_ATTACHED, THREAD_DETACHED } thread_mode_t;

/*
 * Kernel thread - the basic scheduling unit.
 */
struct thread {
	ulong_t stack_ptr;		/* saved stack pointer (this must be the first field!) */
	volatile u32_t num_ticks;       /* number of ticks thread has been running */
	void *stack;                    /* kernel stack */
	struct thread *parent;          /* parent thread */
	struct process *proc;           /* process the thread belongs to (null for kernel-only) */
	thread_state_t state;           /* state of thread in lifecycle */
	int exitcode;                   /* thread's exit code */
	int refcount;                   /* num threads that will wait for this one */
	struct thread_queue waitqueue;  /* wait queue for thread lifecycle events */
	DEFINE_LINK(thread_queue, thread);
};

extern struct thread *g_current;       /* pointer to current thread */
extern volatile int g_need_reschedule; /* set to 1 when a new thread should be chosen */
extern volatile int g_preemption;      /* set to 1 when preemption is enabled */

/* Type of thread start functions */
typedef void (thread_func_t)(ulong_t arg);

/* Bootstrap main thread, initialize scheduler. */
void thread_init(void);

/* Creating, running, destroying threads. */
struct thread *thread_create(thread_func_t *start_func, ulong_t arg, thread_mode_t mode);
void thread_exit(int exitcode) __attribute__((noreturn));
int thread_join(struct thread *child);

/* Thread synchronization primitives. */
void thread_wait(struct thread_queue *queue);
void thread_park(struct thread_queue *queue);
void thread_wakeup(struct thread_queue *queue);
void thread_wakeup_one(struct thread_queue *queue);
void thread_wait_until(struct thread_queue *queue, bool (*pred)(struct thread *), struct thread *thread);
bool thread_refcount_is_zero(struct thread *thread);
bool thread_not_running(struct thread *thread);
void thread_yield(void);
void thread_relinquish_cpu(void);
struct thread *thread_next_runnable(void);
void thread_make_runnable(struct thread *thread);

/* Pick a thread to run and run it, leaving current thread runnable. */
void thread_schedule(void);

/* Architecture-dependent functions. */
void thread_switch_to(struct thread *thread);
void thread_bootstrap(struct thread *thread, thread_func_t *start_func, ulong_t arg);

#endif /* ifndef ASM */

#endif /* ifdef KERNEL */

#endif /* ifndef GEEKOS_THREAD_H */
