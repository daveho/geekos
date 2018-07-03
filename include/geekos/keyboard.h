/*
 * Keyboard driver
 * Copyright (c) 2001, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.13 $
 * 
 * modified: Matthias Aechtner (2014)
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef GEEKOS_KEYBOARD_H
#define GEEKOS_KEYBOARD_H

#include <geekos/types.h>
#include <arch/keyboard.h>

/* Wait queue for thread(s) waiting for keyboard events. */
extern struct thread_queue s_waitqueue;

/* public functions */
void keyboard_init(void);
bool read_key(u16_t* keycode);
u16_t wait_for_key(void);

#endif  /* GEEKOS_KEYBOARD_H */
