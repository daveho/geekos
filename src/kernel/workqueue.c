/*
 * GeekOS - work queues
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

#include <geekos/workqueue.h>
#include <geekos/thread.h>
#include <geekos/int.h>
#include <geekos/mem.h>
#include <geekos/kassert.h>

struct workqueue_item;

struct workqueue_item {
	void (*callback)(void *);
	void *data;
	struct workqueue_item *next;
};

/* queue of workqueue items */
struct workqueue_item *s_workqueue_head, *s_workqueue_tail;

/* wait queue in which the workqueue thread waits for new items */
struct thread_queue s_waitqueue;

/*
 * Work queue thread: wait for items to arrive and invoke each item's callback.
 */
static void workqueue_thread(ulong_t arg)
{
	while (true) {
		struct workqueue_item *item;

		int_disable();

		/* wait for an item to arrive */
		while (s_workqueue_head == 0) {
			thread_wait(&s_waitqueue);
		}

		/* Get the next item */
		item = s_workqueue_head;
		s_workqueue_head = s_workqueue_head->next;
		if (s_workqueue_head == 0) {
			s_workqueue_tail = 0;
		}

		int_enable();

		/* do the work and delete the item */
		item->callback(item->data);
		mem_free(item);
	}
}

/*
 * Initialize work queue.
 */
void workqueue_init(void)
{
	thread_create(&workqueue_thread, 0UL, THREAD_DETACHED);
}

/*
 * Add an item to be processed by the work queue.
 */
void workqueue_schedule_work(void (*callback)(void *), void *data)
{
	bool iflag;
	struct workqueue_item *item;

	/* create new item */
	item = mem_alloc(sizeof(struct workqueue_item));
	item->callback = callback;
	item->data = data;
	item->next = 0;

	iflag = int_begin_atomic();

	/* add to tail of queue */
	if (s_workqueue_head == 0) {
		s_workqueue_head = s_workqueue_tail = item;
	} else {
		KASSERT(s_workqueue_tail != 0);
		s_workqueue_tail->next = item;
		s_workqueue_tail = item;
	}

	/* notify workqueue thread */
	thread_wakeup_one(&s_waitqueue);

	int_end_atomic(iflag);
}
