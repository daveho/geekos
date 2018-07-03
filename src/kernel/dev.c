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

#include <geekos/dev.h>
#include <geekos/errno.h>
#include <geekos/list.h>
#include <geekos/synch.h>
#include <geekos/mem.h>
#include <geekos/kassert.h>
#include <geekos/string.h>

/* ------------- Implementation ------------- */

struct device;
struct device_list;

/*
 * A device is a particular instance of a hardware device;
 * e.g., "ramdisk0" for the first ramdisk, "ata0" for the
 * first ATA device, etc.
 */
struct device {
	char name[DEV_NAME_MAXLEN + 1];
	dev_type_t type;
	void *devobj; /* the "real" device; a blockdev, chardev, etc. */
	DEFINE_LINK(device_list, device);
};

DECLARE_LIST(device_list, device);
IMPLEMENT_LIST_GET_FIRST(device_list, device)
IMPLEMENT_LIST_APPEND(device_list, device)
IMPLEMENT_LIST_NEXT(device_list, device)

/* linked list of registered devices */
static struct device_list s_devlist;

/* mutex for accessing the device list */
static struct mutex s_devlist_mutex;

/*
 * Register a device.
 */
static int dev_register(dev_type_t type, const char *name, void *devobj)
{
	int rc = 0;
	struct device *dev;

	mutex_lock(&s_devlist_mutex);

	/* make sure that no device with this name is already registered */
	for (dev = device_list_get_first(&s_devlist); dev != 0; dev = device_list_next(dev)) {
		if (strncmp(name, dev->name, DEV_NAME_MAXLEN) == 0) {
			rc = EEXIST;
			goto done;
		}
	}

	/* create device */
	dev = mem_alloc(sizeof(struct device));
	strncpy(dev->name, name, DEV_NAME_MAXLEN);
	dev->name[DEV_NAME_MAXLEN] = '\0';
	dev->type = type;
	dev->devobj = devobj;

	/* append to end of device list */
	device_list_append(&s_devlist, dev);

done:
	mutex_unlock(&s_devlist_mutex);

	return rc;
}

struct dev_find_callback_data {
	dev_type_t type;
	const char *name;
	void *result;
};

/*
 * Callback (for use with dev_enumerate()) to find a device
 * with a particular type and name.  If found, puts the device object
 * in the result field of the callback data struct.
 */
static bool dev_find_callback(dev_type_t type, const char *name, void *devobj, void *data_)
{
	struct dev_find_callback_data *data = data_;

	if (type == data->type && strncmp(name, data->name, DEV_NAME_MAXLEN) == 0) {
		/* found; don't need to continue search */
		data->result = devobj;
		return false;
	} else {
		/* continue search */
		return true;
	}
}

/* ------------- Interface ------------- */

/*
 * Register a block device.
 */
int dev_register_blockdev(const char *name, struct blockdev *dev)
{
	return dev_register(DEV_BLOCK, name, dev);
}

/*
 * Find a block device.
 */
int dev_find_blockdev(const char *name, struct blockdev **p_dev)
{
	struct dev_find_callback_data data = {
		.type = DEV_BLOCK,
		.name = name,
		.result = 0,
	};

	dev_enumerate(&dev_find_callback, &data);

	if (!data.result) {
		return ENODEV;
	}

	*p_dev = data.result;
	return 0;
}

/*
 * Enumerate registered devices.
 */
void dev_enumerate(dev_callback_t *callback, void *data)
{
	struct device *dev;

	mutex_lock(&s_devlist_mutex);

	for (dev = device_list_get_first(&s_devlist); dev != 0; dev = device_list_next(dev)) {
		if (!callback(dev->type, dev->name, dev->devobj, data)) {
			break;
		}
	}

	mutex_unlock(&s_devlist_mutex);
}
