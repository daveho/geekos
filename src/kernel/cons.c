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

#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>
#include <geekos/cons.h>
#include <geekos/int.h>

#define MAXDIGITS 39

static struct console *s_console;

static void cons_ultoa(char *buf, unsigned long value)
{
	int i, ndigits = 0;

	/* generate digits in reverse order */
	do {
		buf[ndigits++] = (value % 10) + '0';
		value /= 10;
	} while (value > 0);

	/* reverse digits to put them in the correct order */
	for (i = 0; i < ndigits/2; ++i) {
		int swap_pos = ndigits - (i+1);
		char t = buf[i];
		buf[i] = buf[swap_pos];
		buf[swap_pos] = t;
	}

	/* terminate the string */
	buf[ndigits] = '\0';
}

static void cons_ltoa(char *buf, long value)
{
	unsigned raw;

	/* handle negative values by converting to positive magnitude */
	if (value < 0) {
		*buf++ = '-';
		raw = (unsigned) -value;
	} else {
		raw = (unsigned) value;
	}

	cons_ultoa(buf, raw);
}

static void cons_ltox(char *buf, long value)
{
	int i;
	for (i = 28; i >= 0; i -= 4) {
		*buf++ = "0123456789ABCDEF"[(value >> i) & 0xF];
	}
	*buf = '\0';
}

int cons_init(void)
{
	int rc;

	rc = cons_getdefault(&s_console);
	if (rc != 0) {
		goto done;
	}

	s_console->ops->clear(s_console);

done:
	return rc;
}

void cons_clear(void)
{
	bool iflag = int_begin_atomic();
	s_console->ops->clear(s_console);
	int_end_atomic(iflag);
}

int cons_numrows(void)
{
	int numrows;
	bool iflag;

	iflag = int_begin_atomic();
	numrows = s_console->ops->numrows(s_console);
	int_end_atomic(iflag);

	return numrows;
}

int cons_numcols(void)
{
	int numcols;
	bool iflag;

	iflag = int_begin_atomic();
	numcols = s_console->ops->numcols(s_console);
	int_end_atomic(iflag);

	return numcols;
}

int cons_getx(void)
{
	int x;
	bool iflag;

	iflag = int_begin_atomic();
	x = s_console->ops->getx(s_console);
	int_end_atomic(iflag);

	return x;
}

int cons_gety(void)
{
	int y;
	bool iflag;

	iflag = int_begin_atomic();
	y = s_console->ops->gety(s_console);
	int_end_atomic(iflag);

	return y;
}

void cons_movecurs(int row, int col)
{
	bool iflag = int_begin_atomic();
	s_console->ops->movecurs(s_console, row, col);
	int_end_atomic(iflag);
}

void cons_putchar(int ch)
{
	bool iflag = int_begin_atomic();
	s_console->ops->putchar(s_console, ch);
	int_end_atomic(iflag);
}

void cons_write(const char *str)
{
	bool iflag = int_begin_atomic();
	s_console->ops->write(s_console, str);
	int_end_atomic(iflag);
}

void cons_printf(const char *fmt, ...)
{
	va_list args;
	char buf[MAXDIGITS + 2];
	bool is_long, iflag;

	va_start(args, fmt);

	iflag = int_begin_atomic();

	while (*fmt != '\0') {
		switch (*fmt) {
		case '%':
			if (!*++fmt) goto done;

			if (*fmt == 'l') {
				is_long = true;
				if (!*++fmt) goto done;
			} else if (*fmt == 'p') {
				is_long = true;
			} else {
				is_long = false;
			}

			switch (*fmt) {
			case 'd':
				cons_ltoa(buf, is_long ? va_arg(args, long) : va_arg(args, int));
				cons_write(buf);
				break;

			case 'u':
				cons_ultoa(buf, is_long
					? va_arg(args, unsigned long) : va_arg(args, unsigned));
				cons_write(buf);
				break;

			case 'x':
			case 'p':
				cons_ltox(buf, is_long ? va_arg(args, long) : va_arg(args, int));
				cons_write(buf);
				break;

			case 'c':
				cons_putchar(va_arg(args, int) & 0xFF);
				break;

			case 's':
				cons_write(va_arg(args, const char *));
				break;

			default:
				cons_putchar(*fmt);
				break;
			}
			break;

		default:
			cons_putchar(*fmt);
			break;
		}
		++fmt;
	}


done:
	int_end_atomic(iflag);
	va_end(args);
}
