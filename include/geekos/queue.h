/*
 * Queue for Keyboard driver
 * Copyright (c) 2001, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.13 $
 *
 * modified: Matthias Aechtner (2014)
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef GEEKOS_QUEUE_H
#define GEEKOS_QUEUE_H

#include <geekos/kassert.h>
#include <geekos/types.h>

/* Queue for keycodes, in case they arrive faster than consumer
 * can deal with them. */
#define QUEUE_SIZE 256
#define QUEUE_MASK 0xff
#define NEXT(index) (((index) + 1) & QUEUE_MASK)

extern u16_t s_queue[QUEUE_SIZE];
extern int s_queue_head, s_queue_tail;

/* inline functions*/
static __inline__ bool is_queue_full(void)
{
    return NEXT(s_queue_tail) == s_queue_head;
}

static __inline__ bool is_queue_empty(void)
{
    return s_queue_head == s_queue_tail;
}


static __inline__ void enqueue(u16_t item)
{
    if (!is_queue_full()) {
	s_queue[s_queue_tail] = item;
	s_queue_tail = NEXT(s_queue_tail);
    }
}

static __inline__ u16_t dequeue(void)
{
    u16_t result;
    KASSERT(!is_queue_empty());
    result = s_queue[s_queue_head];
    s_queue_head = NEXT(s_queue_head);
    return result;
}

#endif
