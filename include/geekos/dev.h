/*
 * GeekOS devices
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

#ifndef GEEKOS_DEV_H
#define GEEKOS_DEV_H

#include <geekos/types.h>

/* maximum length of a device name */
#define DEV_NAME_MAXLEN 31

/* device type */
typedef enum { DEV_CHAR, DEV_BLOCK } dev_type_t;

/*
 * Callback function for enumerating devices; invoked with devlist mutex held.
 * Returns true if enumeration should continue, false if not.
 */
typedef bool (dev_callback_t)(dev_type_t type, const char *name, void *devobj, void *data);

struct blockdev;

/* functions */
int dev_register_blockdev(const char *name, struct blockdev *dev);
int dev_find_blockdev(const char *name, struct blockdev **p_dev);
void dev_enumerate(dev_callback_t *callback, void *data);

#endif /* GEEKOS_DEV_H */
