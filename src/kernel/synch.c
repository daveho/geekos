/*
 * GeekOS - thread synchronization
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

#include <geekos/synch.h>
#include <geekos/int.h>
#include <geekos/kassert.h>

/*
 * NOTES:
 * - The GeekOS mutex and condition variable APIs are based on those
 *   in pthreads.
 * - Unlike disabling interrupts, mutexes offer NO protection against
 *   concurrent execution of interrupt handlers.  mutexes and
 *   condition variables should only be used from kernel threads,
 *   with interrupts enabled.
 */

/* ----------------------------------------------------------------------
 * Private functions
 * ---------------------------------------------------------------------- */

/*
 * Lock given mutex.
 * Preemption must be disabled.
 */
static __inline__ void mutex_lock_imp(struct mutex *mutex)
{
	KASSERT(!g_preemption);

	/* Make sure we're not already holding the mutex */
	KASSERT(!MUTEX_IS_HELD(mutex));

	/* wait until the mutex is in an unlocked state */
	while (mutex->state == MUTEX_LOCKED) {
		thread_park(&mutex->waitqueue);
	}

	/* Now it's ours! */
	mutex->state = MUTEX_LOCKED;
	mutex->owner = g_current;
}

/*
 * Unlock given mutex.
 * Preemption must be disabled.
 */
static __inline__ void mutex_unlock_imp(struct mutex *mutex)
{
	KASSERT(!g_preemption);

	/* Make sure mutex was actually acquired by this thread. */
	KASSERT(MUTEX_IS_HELD(mutex));

	/* unlock the mutex. */
	mutex->state = MUTEX_UNLOCKED;
	mutex->owner = 0;

	/*
	 * If there are threads waiting to acquire the mutex,
	 * wake one of them up.  Note that it is legal to inspect
	 * the queue with interrupts enabled because preemption
	 * is disabled, and therefore we know that no thread can
	 * concurrently add itself to the queue.
	 */
	if (!thread_queue_is_empty(&mutex->waitqueue)) {
		int_disable();
		thread_wakeup_one(&mutex->waitqueue);
		int_enable();
	}
}

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

/*
 * Initialize given mutex.
 */
void mutex_init(struct mutex *mutex)
{
	mutex->state = MUTEX_UNLOCKED;
	mutex->owner = 0;
	thread_queue_clear(&mutex->waitqueue);
}

/*
 * Lock given mutex.
 */
void mutex_lock(struct mutex *mutex)
{
	KASSERT(int_enabled());

	g_preemption = false;
	mutex_lock_imp(mutex);
	g_preemption = true;
}

/*
 * Unlock given mutex.
 */
void mutex_unlock(struct mutex *mutex)
{
	KASSERT(int_enabled());

	g_preemption = false;
	mutex_unlock_imp(mutex);
	g_preemption = true;
}

/*
 * Initialize given condition.
 */
void cond_init(struct condition *cond)
{
	thread_queue_clear(&cond->waitqueue);
}

/*
 * wait on given condition (protected by given mutex).
 */
void cond_wait(struct condition *cond, struct mutex *mutex)
{
	KASSERT(int_enabled());

	/* Ensure mutex is held. */
	KASSERT(MUTEX_IS_HELD(mutex));

	/* Turn off scheduling. */
	g_preemption = false;

	/*
	 * Release the mutex, but leave preemption disabled.
	 * No other threads will be able to run before this thread
	 * is able to wait.  Therefore, this thread will not
	 * miss the eventual notification on the condition.
	 */
	mutex_unlock_imp(mutex);

	/*
	 * Atomically reenable preemption and wait in the condition wait queue.
	 * Other threads can run while this thread is waiting,
	 * and eventually one of them will call cond_signal() or cond_broadcast()
	 * to wake up this thread.
	 * On wakeup, preemption is once again disabled.
	 */
	thread_park(&cond->waitqueue);

	/* Reacquire the mutex. */
	mutex_lock_imp(mutex);

	/* Turn scheduling back on. */
	g_preemption = true;
}

/*
 * Wake up one thread waiting on the given condition.
 * The mutex guarding the condition should be held!
 */
void cond_signal(struct condition *cond)
{
	KASSERT(int_enabled());
	int_disable();  /* prevent scheduling */
	thread_wakeup_one(&cond->waitqueue);
	int_enable();  /* resume scheduling */
}

/*
 * Wake up all threads waiting on the given condition.
 * The mutex guarding the condition should be held!
 */
void cond_broadcast(struct condition *cond)
{
	KASSERT(int_enabled());
	int_disable();  /* prevent scheduling */
	thread_wakeup(&cond->waitqueue);
	int_enable();  /* resume scheduling */
}
