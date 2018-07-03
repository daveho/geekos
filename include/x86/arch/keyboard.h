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

#ifndef ARCH_KEYB_H
#define ARCH_KEYB_H

#define KEY_KEYPAD_FLAG  0x0200
#define KEY_SPECIAL_FLAG 0x0100

#define _SPECIAL(num) (KEY_SPECIAL_FLAG | (num))

/* Special key codes */
#define KEY_UNKNOWN _SPECIAL(0)
#define KEY_F1      _SPECIAL(1)
#define KEY_F2      _SPECIAL(2)
#define KEY_F3      _SPECIAL(3)
#define KEY_F4      _SPECIAL(4)
#define KEY_F5      _SPECIAL(5)
#define KEY_F6      _SPECIAL(6)
#define KEY_F7      _SPECIAL(7)
#define KEY_F8      _SPECIAL(8)
#define KEY_F9      _SPECIAL(9)
#define KEY_F10     _SPECIAL(10)
#define KEY_F11     _SPECIAL(11)
#define KEY_F12     _SPECIAL(12)

/* Keypad keys */
#define KEYPAD_START 128
#define _KEYPAD(num) (KEY_KEYPAD_FLAG | KEY_SPECIAL_FLAG | (num+KEYPAD_START))
#define KEY_KPHOME  _KEYPAD(0)
#define KEY_KPUP    _KEYPAD(1)
#define KEY_KPPGUP  _KEYPAD(2)
#define KEY_KPMINUS _KEYPAD(3)
#define KEY_KPLEFT  _KEYPAD(4)
#define KEY_KPCENTER _KEYPAD(5)
#define KEY_KPRIGHT _KEYPAD(6)
#define KEY_KPPLUS  _KEYPAD(7)
#define KEY_KPEND   _KEYPAD(8)
#define KEY_KPDOWN  _KEYPAD(9)
#define KEY_KPPGDN  _KEYPAD(10)
#define KEY_KPINSERT _KEYPAD(11)
#define KEY_KPDEL   _KEYPAD(12)

#endif  /* ARCH_KEY_H */
