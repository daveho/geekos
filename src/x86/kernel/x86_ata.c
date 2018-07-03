/*
 * GeekOS - ATA (IDE) support
 *
 * Copyright (C) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 * Copyright (C) 2014      Matthias Aechtner 
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

#include <geekos/types.h>
#include <geekos/cons.h>
#include <arch/ioport.h>

/* Registers */
#define ATA_DATA_REG		0x1F0
#define ATA_ERROR_REG		0x1F1
#define ATA_DRIVE_HEAD_REG	0x1F6
#define ATA_STATUS_REG		0x1F7
#define ATA_CMD_REG		0x1F7
#define ATA_DEV_CTRL_REG	0x3F6

/* Bits of Device Control Register */
#define ATA_DCR_NOINTERRUPT	(1 << 1) 
#define ATA_DCR_RESET		(1 << 2)

/* bits of Status Register */
#define ATA_STATUS_DRIVE_BUSY	(1 << 7)
#define ATA_STATUS_DRIVE_DATA_REQUEST	(1 << 3)

/* words from Identify Drive Request (offsets) */
#define	ATA_INDENT_NUM_CYLINDERS	0x01
#define	ATA_INDENT_NUM_HEADS		0x03
#define	ATA_INDENT_NUM_BYTES_TRACK	0x04
#define	ATA_INDENT_NUM_BYTES_SECTOR	0x05
#define	ATA_INDENT_NUM_SECTORS_TRACK 0x06

/* Commands */
#define ATA_CMD_IDENTIFY_DRIVE	0xEC
#define ATA_CMD_DIAGNOSTIC		0x90

static int ata_read_drive_config(int drive)
{
	int status, i;
	u16_t info[256];
	u16_t num_cylinders, num_heads, num_sectors;
	ioport_outb(ATA_DRIVE_HEAD_REG, 0xA0 + (drive << 4));
	ioport_outb(ATA_CMD_REG, ATA_CMD_IDENTIFY_DRIVE);
	while (ioport_inb(ATA_STATUS_REG) & ATA_STATUS_DRIVE_BUSY)
		;
	status = ioport_inb(ATA_STATUS_REG);

	if (status & ATA_STATUS_DRIVE_DATA_REQUEST) {
		for (i = 0; i < 256; ++i) 
			info[i] = ioport_inw(ATA_DATA_REG);
		num_cylinders = info[ATA_INDENT_NUM_CYLINDERS];
		num_heads     = info[ATA_INDENT_NUM_HEADS];
		num_sectors   = info[ATA_INDENT_NUM_SECTORS_TRACK];
		/*num_bytes_per_sector = info[ATA_INDENT_NUM_BYTES_SECTOR];*/
		cons_printf("  Found ATA drive %d", drive);
		cons_printf(": cyl=%d, heads=%d, sectors=%d\n", 
				num_cylinders, num_heads, num_sectors);
	} else
		return -1;
	return 0;
}

void ata_init(void)
{
	int i, error;
	/* Reset the controller and drives */
	ioport_outb(ATA_DEV_CTRL_REG, ATA_DCR_NOINTERRUPT | ATA_DCR_RESET);
	for (i = 0; i < 5; ++i) ioport_inb(ATA_STATUS_REG); /* delay for 500ms */
	ioport_outb(ATA_DEV_CTRL_REG, ATA_DCR_NOINTERRUPT);

	while (ioport_inb(ATA_STATUS_REG) & ATA_STATUS_DRIVE_BUSY)
		;

	ioport_outb(ATA_CMD_REG, ATA_CMD_DIAGNOSTIC);
	while (ioport_inb(ATA_STATUS_REG) & ATA_STATUS_DRIVE_BUSY)
		;
	error = ioport_inb(ATA_ERROR_REG);

    error = ata_read_drive_config(0);
	if (error) cons_printf("Ata_init: No drive found\n");
}
