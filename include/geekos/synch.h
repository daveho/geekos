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

#ifndef GEEKOS_SYNCH_H
#define GEEKOS_SYNCH_H

#include <geekos/thread.h>

typedef enum { MUTEX_UNLOCKED = 0, MUTEX_LOCKED = 1 } mutex_state_t;

struct mutex {
	mutex_state_t state;
	struct thread *owner;
	struct thread_queue waitqueue;
};

struct condition {
	struct thread_queue waitqueue;
};

void mutex_init(struct mutex *mutex);
void mutex_lock(struct mutex *mutex);
void mutex_unlock(struct mutex *mutex);

void cond_init(struct condition *cond);
void cond_wait(struct condition *cond, struct mutex *mutex);
void cond_signal(struct condition *cond);
void cond_broadcast(struct condition *cond);

#define MUTEX_IS_HELD(mutex) \
	((mutex)->state == MUTEX_LOCKED && (mutex)->owner == g_current)

#endif /* ifndef GEEKOS_SYNCH_H */
