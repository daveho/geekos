/*
 * GeekOS - string functions
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

#include <geekos/string.h>
#include <geekos/types.h>
#include <geekos/mem.h>

/*
 * TODO: these could be speeded up with architecture-specific implementations
 */

void memcpy(void *dst, const void *src, size_t n)
{
	u8_t *d = dst;
	const u8_t *s = src;

	while (n > 0) {
		*d++ = *s++;
		--n;
	}
}

void memset(void *buf, int c, size_t n)
{
	u8_t *d = buf;

	while (n > 0) {
		*d++ = c;
		--n;
	}
}

size_t strlen(const char *s)
{
	size_t len = 0;
	while (*s++ != '\0') {
		++len;
	}
	return len;
}

size_t strnlen(const char *s, size_t maxlen)
{
	size_t len = 0;
	while (len < maxlen && *s++ != '\0') {
		++len;
	}
	return len;
}

int strcmp(const char *s1, const char *s2)
{
	for (;;) {
		int cmp = *s1 - *s2;
		if (cmp != 0 || *s1 == '\0' || *s2 == '\0') {
			return cmp;
		}
		++s1;
		++s2;
	}
}

int strncmp(const char *s1, const char *s2, size_t limit)
{
	size_t i = 0;
	while (i < limit) {
		int cmp = *s1 - *s2;
		if (cmp != 0 || *s1 == '\0' || *s2 == '\0') {
			return cmp;
		}
		++s1;
		++s2;
		++i;
	}

	/* limit reached and equal */
	return 0;
}

char *strncpy(char *dest, const char *src, size_t limit)
{
	char *ret = dest;

	while (*src != '\0' && limit > 0) {
		*dest++ = *src++;
		--limit;
	}
	if (limit > 0) {
		*dest = '\0';
	}

	return ret;
}

#if 0
char *strdup(const char *s)
{
	char *dup;
	dup = mem_alloc(strlen(s) + 1);
	strcpy(dup, s);
	return dup;
}
#endif
