/*
 * Keyboard driver
 * Copyright (c) 2001,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.14 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

/*
 * Information sources:
 * - Chapter 8 of _The Undocumented PC_, 2nd ed, by Frank van Gilluwe,
 *   ISBN 0-201-47950-8.
 * - Pages 400-409 of _The Programmers PC Sourcebook_, by Thom Hogan,
 *   ISBN 1-55615-118-7.
 */

/*
 * Credits:
 * - Peter Gnodde <peter@pcswebdesign.nl> added support for
 *   the CTRL and ALT modifiers
 */

/*
 * Modified:
 * Matthias Aechtner (2014)
 */

/*
 * TODO list:
 * - Right now we're assuming an 83-key keyboard.
 *   Should add support for 101+ keyboards.
 * - Should toggle keyboard LEDs.
 */

#include <geekos/int.h>
#include <geekos/queue.h>
#include <geekos/thread.h>
#include <geekos/keyboard.h>

/* Wait queue for thread(s) waiting for keyboard events. */
/* FIXME struct thread_queue s_waitqueue; */
u16_t s_queue[QUEUE_SIZE];
int s_queue_head, s_queue_tail;


/* Poll for a key event.
 * Returns true if a key is available, false if not.  If a key event
 * is available, it will be stored in the location pointed to by keycode. */
bool read_key(u16_t* keycode)
{
    bool result, iflag;

    iflag = int_begin_atomic();

    result = !is_queue_empty();
    if (result) {
	*keycode = dequeue();
    }

    int_end_atomic(iflag);

    return result;
}

/* Wait for a keycode to arrive.
 * Uses the keyboard wait queue to sleep until a keycode arrives. */
u16_t wait_for_key(void)
{
    bool got_key, iflag;
    u16_t keycode = KEY_UNKNOWN;

    iflag = int_begin_atomic();

    do {
	got_key = !is_queue_empty();
	if (got_key)
	    keycode = dequeue();
	else
	    thread_wait(&s_waitqueue);
    }
    while (!got_key);

    int_end_atomic(iflag);

    return keycode;
}
