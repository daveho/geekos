/*
 * PS/2 initialization
 *
 * Copyright (C) 2014, Matthias Aechtner
 *
 * credits: wiki.osdev.org "8042" PS/2 Controller
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
#include <arch/ps2.h>

/* PS/2 Controller Commands */
#define	PS2_DISABLE_FIRST_PORT 0xAD
#define PS2_DISABLE_SECOND_PORT 0xA7
#define	PS2_ENABLE_FIRST_PORT 0xAE
#define PS2_ENABLE_SECOND_PORT 0xA8

void x86_ps2_init(void)
{
        /* don't let devices send data at the wrong time and mess up initialisation */
        ioport_outb(PS2_COMMAND, PS2_DISABLE_FIRST_PORT);
        ioport_outb(PS2_COMMAND, PS2_DISABLE_SECOND_PORT); /* ignored if not supported */

        /* flush device buffer */
        while (ioport_inb(PS2_COMMAND) & PS2_OUTPUT_FULL) {
            ioport_inb(PS2_DATA);
        }

        /* finilize PS/2 initialization by reenabling devices */
        ioport_outb(PS2_COMMAND, PS2_ENABLE_FIRST_PORT); 
        ioport_outb(PS2_COMMAND, PS2_ENABLE_SECOND_PORT); /* ignored if unsupported? */
}	
