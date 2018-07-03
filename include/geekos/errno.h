/*
 * GeekOS - error codes
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

#ifndef GEEKOS_ERRNO_H
#define GEEKOS_ERRNO_H

#ifdef KERNEL

#define ENOMEM -1      /* out of memory */
#define EEXIST -2      /* no such file or directory */
#define ENOTDIR -3     /* not a directory */
#define EINVAL -4      /* invalid argument */
#define ENODEV -5      /* no such device */
#define EIO -6         /* input/output error */
#define ENOTSUP -7     /* operation not supported */

#endif

#endif /* GEEKOS_ERRNO_H */
