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

#include <geekos/queue.h>
#include <geekos/irq.h>
#include <arch/ioport.h>
#include <arch/ps2.h>
#include <geekos/keyboard.h> /* implement keyboard_init */
#include <arch/keyboard.h>

#define KEYB_IRQ 1

/* High bit in scan code is set when key is released */
#define KB_KEY_RELEASE 0x80

/* Each keyboard event generates a 16 bit code.
 * - The low 10 bits indicate which key was used.
 * - If bit 8 (KEY_SPECIAL_FLAG) is 0, then the low 8 bits contain
 *   the ASCII code.
 * - The flags indicate the shift/alt/control state,
 *   and whether the event was a make or release.
 */

#define KEY_SHIFT_FLAG   0x1000
#define KEY_ALT_FLAG     0x2000
#define KEY_CTRL_FLAG    0x4000
#define KEY_RELEASE_FLAG 0x8000

/* Special key codes */
#define KEY_LCTRL   _SPECIAL(13)
#define KEY_RCTRL   _SPECIAL(14)
#define KEY_LSHIFT  _SPECIAL(15)
#define KEY_RSHIFT  _SPECIAL(16)
#define KEY_LALT    _SPECIAL(17)
#define KEY_RALT    _SPECIAL(18)
#define KEY_PRINTSCRN _SPECIAL(19)
#define KEY_CAPSLOCK _SPECIAL(20)
#define KEY_NUMLOCK _SPECIAL(21)
#define KEY_SCRLOCK _SPECIAL(22)
#define KEY_SYSREQ  _SPECIAL(23)

/* ASCII codes for which there is no convenient C representation */
#define ASCII_ESC 0x1B
#define ASCII_BS  0x08

/* Current shift state. */
#define LEFT_SHIFT  0x01
#define RIGHT_SHIFT 0x02
#define LEFT_CTRL   0x04
#define RIGHT_CTRL  0x08
#define LEFT_ALT    0x10
#define RIGHT_ALT   0x20
#define SHIFT_MASK  (LEFT_SHIFT | RIGHT_SHIFT)
#define CTRL_MASK   (LEFT_CTRL | RIGHT_CTRL)
#define ALT_MASK    (LEFT_ALT | RIGHT_ALT)
static unsigned s_shiftstate = 0;
#define KEY_ESCAPE  0xE0

/* Translate from scan code to key code, when shift is not pressed. */
static const u16_t s_scan_table_no_shift[] = {
    KEY_UNKNOWN, ASCII_ESC, '1', '2',   /* 0x00 - 0x03 */
    '3', '4', '5', '6',                 /* 0x04 - 0x07 */
    '7', '8', '9', '0',                 /* 0x08 - 0x0B */
    '-', '=', ASCII_BS, '\t',           /* 0x0C - 0x0F */
    'q', 'w', 'e', 'r',                 /* 0x10 - 0x13 */
    't', 'y', 'u', 'i',                 /* 0x14 - 0x17 */
    'o', 'p', '[', ']',                 /* 0x18 - 0x1B */
    '\r', KEY_LCTRL, 'a', 's',          /* 0x1C - 0x1F */
    'd', 'f', 'g', 'h',                 /* 0x20 - 0x23 */
    'j', 'k', 'l', ';',                 /* 0x24 - 0x27 */
    '\'', '`', KEY_LSHIFT, '\\',        /* 0x28 - 0x2B */
    'z', 'x', 'c', 'v',                 /* 0x2C - 0x2F */
    'b', 'n', 'm', ',',                 /* 0x30 - 0x33 */
    '.', '/', KEY_RSHIFT, KEY_PRINTSCRN, /* 0x34 - 0x37 */
    KEY_LALT, ' ', KEY_CAPSLOCK, KEY_F1, /* 0x38 - 0x3B */
    KEY_F2, KEY_F3, KEY_F4, KEY_F5,     /* 0x3C - 0x3F */
    KEY_F6, KEY_F7, KEY_F8, KEY_F9,     /* 0x40 - 0x43 */
    KEY_F10, KEY_NUMLOCK, KEY_SCRLOCK, KEY_KPHOME,  /* 0x44 - 0x47 */
    KEY_KPUP, KEY_KPPGUP, KEY_KPMINUS, KEY_KPLEFT,  /* 0x48 - 0x4B */
    KEY_KPCENTER, KEY_KPRIGHT, KEY_KPPLUS, KEY_KPEND,  /* 0x4C - 0x4F */
    KEY_KPDOWN, KEY_KPPGDN, KEY_KPINSERT, KEY_KPDEL,  /* 0x50 - 0x53 */
    KEY_SYSREQ, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN,  /* 0x54 - 0x57 */
    KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN     /* 0x58 - 0x60 */
};
#define SCAN_TABLE_SIZE (sizeof(s_scan_table_no_shift) / sizeof(u16_t))

/* Translate from scan code to key code, when shift *is* pressed.
 * Keep this in sync with the unshifted table above!
 * They must be the same size. */
static const u16_t s_scan_table_with_shift[] = {
    KEY_UNKNOWN, ASCII_ESC, '!', '@',   /* 0x00 - 0x03 */
    '#', '$', '%', '^',                 /* 0x04 - 0x07 */
    '&', '*', '(', ')',                 /* 0x08 - 0x0B */
    '_', '+', ASCII_BS, '\t',           /* 0x0C - 0x0F */
    'Q', 'W', 'E', 'R',                 /* 0x10 - 0x13 */
    'T', 'Y', 'U', 'I',                 /* 0x14 - 0x17 */
    'O', 'P', '{', '}',                 /* 0x18 - 0x1B */
    '\r', KEY_LCTRL, 'A', 'S',          /* 0x1C - 0x1F */
    'D', 'F', 'G', 'H',                 /* 0x20 - 0x23 */
    'J', 'K', 'L', ':',                 /* 0x24 - 0x27 */
    '"', '~', KEY_LSHIFT, '|',          /* 0x28 - 0x2B */
    'Z', 'X', 'C', 'V',                 /* 0x2C - 0x2F */
    'B', 'N', 'M', '<',                 /* 0x30 - 0x33 */
    '>', '?', KEY_RSHIFT, KEY_PRINTSCRN, /* 0x34 - 0x37 */
    KEY_LALT, ' ', KEY_CAPSLOCK, KEY_F1, /* 0x38 - 0x3B */
    KEY_F2, KEY_F3, KEY_F4, KEY_F5,     /* 0x3C - 0x3F */
    KEY_F6, KEY_F7, KEY_F8, KEY_F9,     /* 0x40 - 0x43 */
    KEY_F10, KEY_NUMLOCK, KEY_SCRLOCK, KEY_KPHOME,  /* 0x44 - 0x47 */
    KEY_KPUP, KEY_KPPGUP, KEY_KPMINUS, KEY_KPLEFT,  /* 0x48 - 0x4B */
    KEY_KPCENTER, KEY_KPRIGHT, KEY_KPPLUS, KEY_KPEND,  /* 0x4C - 0x4F */
    KEY_KPDOWN, KEY_KPPGDN, KEY_KPINSERT, KEY_KPDEL,  /* 0x50 - 0x53 */
    KEY_SYSREQ, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN,  /* 0x54 - 0x57 */
    KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN     /* 0x58 - 0x0A */
};

static void keyboard_int_handler(struct thread_context *context)
{
    u8_t status, scan_code;
    unsigned flag = 0;
    bool release = false, shift;
    /* bool specialkey = false; */
    u16_t keycode;

    irq_begin(context);

    status = ioport_inb(PS2_COMMAND);
    ioport_delay();

    if ((status & PS2_OUTPUT_FULL) != 0) {
	/* There is a byte available */
	scan_code = ioport_inb(PS2_DATA);
	ioport_delay();

        if (scan_code == KEY_ESCAPE) {
            /*specialkey = true;*/
	    goto done;
        }
 
   	/*cons_printf("cod=%x%s\n", scan_code, 
                    (scan_code&0x80) ? " [release]" : "",
                    specialkey ? " [special]" : "");*/
   

	if (scan_code & KB_KEY_RELEASE) {
	    release = true;
	    scan_code &= ~(KB_KEY_RELEASE);
	}

	if (scan_code >= SCAN_TABLE_SIZE) {
  	    cons_printf("Unknown scan code: %x\n", scan_code);
	    goto done;
	}

	/* Process the key */
	shift = ((s_shiftstate & SHIFT_MASK) != 0);
	keycode = shift ? s_scan_table_with_shift[scan_code] : s_scan_table_no_shift[scan_code];

	/* Update shift, control and alt state */
	switch (keycode) {
	case KEY_LSHIFT:
	    flag = LEFT_SHIFT;
	    break;
	case KEY_RSHIFT:
	    flag = RIGHT_SHIFT;
	    break;
	case KEY_LCTRL:
	    flag = LEFT_CTRL;
	    break;
	case KEY_RCTRL:
	    flag = RIGHT_CTRL;
	    break;
	case KEY_LALT:
	    flag = LEFT_ALT;
	    break;
	case KEY_RALT:
	    flag = RIGHT_ALT;
	    break;
	default:
	    goto noflagchange;
	}

	if (release)
	    s_shiftstate &= ~(flag);
	else
	    s_shiftstate |= flag;
			
	/*
	 * Shift, control and alt keys don't have to be
	 * queued, flags will be set!
	 */
	goto done;

noflagchange:
	/* Format the new keycode */
	if (shift)
	    keycode |= KEY_SHIFT_FLAG;
	if ((s_shiftstate & CTRL_MASK) != 0)
	    keycode |= KEY_CTRL_FLAG;
	if ((s_shiftstate & ALT_MASK) != 0)
	    keycode |= KEY_ALT_FLAG;
	if (release)
	    keycode |= KEY_RELEASE_FLAG;
		
	/* Put the keycode in the buffer */
	enqueue(keycode);

	/* Wake up event consumers */
	thread_wakeup(&s_waitqueue);

	/*
	 * Pick a new thread upon return from interrupt
	 * (hopefully the one waiting for the keyboard event)
	 */
	g_need_reschedule = true;
    }

done:
    irq_end(context);
}

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

void keyboard_init(void)
{
	if (int_enabled()) int_disable();
	cons_printf("Initialize keyboard ............");
        
        x86_ps2_init();

	/* Start out with no shift keys enabled. */
	s_shiftstate = 0;

	/* Buffer is initially empty. */
	s_queue_head = s_queue_tail = 0;

	/* Install interrupt handler */
        irq_install_handler(KEYB_IRQ, &keyboard_int_handler);
	irq_enable(KEYB_IRQ);

	int_enable();
	g_preemption = true;
	cons_printf(".... [OK]\n");
}

