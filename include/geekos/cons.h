/*
 * GeekOS - text console
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

#ifndef GEEKOS_CONS_H
#define GEEKOS_CONS_H

#ifndef ASM

#define CONS_TABSIZE 8

struct console;

/* console operations */
struct console_ops {
	void (*clear)(struct console *cons);
	int (*numrows)(struct console *cons);
	int (*numcols)(struct console *cons);
	int (*getx)(struct console *cons);
	int (*gety)(struct console *cons);
	void (*movecurs)(struct console *cons, int row, int col);
	void (*putchar)(struct console *cons, int ch);
	void (*write)(struct console *cons, const char *str);
	void (*cleartoeol)(struct console *cons);
};

/* console base class */
struct console {
	struct console_ops *ops;
	void *p;   /* for use by underlying console implementation */
};

int cons_init(void);
void cons_clear(void);
int cons_numrows(void);
int cons_numcols(void);
int cons_getx(void);
int cons_gety(void);
void cons_movecurs(int row, int col);
void cons_putchar(int ch);
void cons_write(const char *str);
void cons_cleartoeol(void);

void cons_printf(const char *fmt, ...) __attribute__ ((format (printf,1,2)));

#if 0
int cons_create(struct console_ops *ops, void *p, struct console **p_cons);
#endif

/* Architecture-dependent functions */
int cons_getdefault(struct console **p_cons);

#endif /* ASM */

#endif /* GEEKOS_CONS_H */
