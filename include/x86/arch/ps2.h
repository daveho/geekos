/*
 * GeekOS - x86 port IO
 *
 * author: Matthias Aechtner (2014)
 */

#ifndef ARCH_PS2_H
#define ARCH_PS2_H

/* I/O ports */
#define PS2_DATA 0x60
#define PS2_COMMAND 0x64

/* Bits in status port */
#define PS2_OUTPUT_FULL 0x01

void x86_ps2_init(void);

#endif /* ARCH_PS2_H */
