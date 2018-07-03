/*
 * GeekOS - work queues
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

#ifndef GEEKOS_WORKQUEUE_H
#define GEEKOS_WORKQUEUE_H

/*
 * The workqueue provides a way deferring execution of code
 * until a safer point is reached.  For example, it is used
 * to free memory and resources of a thread after the thread
 * is no longer running.
 */

void workqueue_init(void);
void workqueue_schedule_work(void (*callback)(void *), void *data);

#endif /* ifndef GEEKOS_WORKQUEUE_H */
