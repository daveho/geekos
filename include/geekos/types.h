/*
 * GeekOS - Basic data types
 *
 * Copyright (C) 2001-2008, David H. Hovemeyer
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

#ifndef GEEKOS_TYPES_H
#define GEEKOS_TYPES_H

#ifndef ASM

#include <stdbool.h>
#include <stddef.h>

typedef unsigned long u32_t;
typedef unsigned short u16_t;
typedef unsigned char u8_t;

typedef unsigned long ulong_t;
typedef unsigned int uint_t;

#define OFFSETOF(type, field) \
(((char*) &((type *)0)->field) - ((char*) (type *) 0))

#endif

#endif /* GEEKOS_TYPES_H */
