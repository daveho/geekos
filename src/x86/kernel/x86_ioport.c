/*
 * GeekOS - x86 port IO
 *
 * Copyright (C) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 *
 * modified: Matthias Aechtner (2014)
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

#include <arch/ioport.h>

u8_t ioport_inb(u16_t port)
{
	u8_t value;
	__asm__ __volatile__ ("inb %w1, %b0" : "=a" (value) : "Nd" (port));
	return value;
}

u16_t ioport_inw(u16_t port)
{
	u16_t value;
	__asm__ __volatile__ ("inw %w1, %w0" : "=a" (value) : "Nd" (port));
	return value;
}

u32_t ioport_inl(u16_t port)
{
	u32_t value;
	__asm__ __volatile__ ("inl %w1, %0" : "=a" (value) : "Nd" (port));
	return value;
}

void ioport_outb(u16_t port, u8_t value)
{
	__asm__ __volatile__ ("outb %b0, %w1" : : "a" (value), "Nd" (port));
}

void ioport_outw(u16_t port, u16_t value)
{
	__asm__ __volatile__ ("outw %w0, %w1" : : "a" (value), "Nd" (port));
}

void ioport_outl(u16_t port, u32_t value)
{
	__asm__ __volatile__ ("outl %0, %w1" : : "a" (value), "Nd" (port));
}

void ioport_delay(void)
{
    u8_t value = 0;
    __asm__ __volatile__ ("outb %0, $0x80" : : "a" (value));
}
