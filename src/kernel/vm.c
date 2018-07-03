/*
 * GeekOS - virtual memory
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

#include <geekos/vm.h>

static void vm_release_frame_ref(struct vm_pagecache *obj, struct frame *frame)
{
	KASSERT(MUTEX_IS_HELD(&obj->lock));
	KASSERT(frame->refcount > 0);
	KASSERT(frame->content != PAGE_PENDING_INIT);

	frame->refcount--;

	/*
	 * If the frame's refcount reaches 0, AND its contents
	 * are not valid due to the failure of the initial pagein,
	 * then eagerly remove it from the vm_pagecache.
	 */
	if (frame->refcount == 0 && frame->content == PAGE_FAILED_INIT) {
		frame_list_remove(&obj->pagelist, frame);
		mem_free_frame(frame);
	}
}

static int vm_alloc_and_page_in(struct vm_pagecache *obj, u32_t page_num, struct frame **p_frame)
{
	int rc;
	struct frame *frame;

	KASSERT(MUTEX_IS_HELD(&obj->lock));

	/* allocate a fresh frame */
	frame = mem_alloc_frame(FRAME_VM_PGCACHE, 1);

	/* append frame to pagelist, mark as having pending I/O */
	frame_list_append(&obj->pagelist, frame);
	frame->content = PAGE_PENDING_INIT;

	/* unlock the vm_pagecache mutex while pagein is being done.
	 * because we set the content to PAGE_PENDING_INIT,
	 * other threads looking for this page will know
	 * its contents aren't initialized yet */
	mutex_unlock(&obj->lock);

	/* page in the data for the frame */
	rc = vm_pagein(obj->pager, page_num, frame);

	/* re-lock the vm_pagecache mutex */
	mutex_lock(&obj->lock);

	/* update frame content based on success/failure of pagein */
	frame->content = (rc == 0) ? PAGE_CLEAN : PAGE_FAILED_INIT;

	/* other threads may be waiting to learn content state */
	cond_broadcast(&obj->cond);

	if (rc == 0) {
		/* success! */
		*p_frame = frame;
	} else {
		/* pagein failed: release reference to frame */
		vm_release_frame_ref(obj, frame);
	}

	return rc;
}

/*
 * Create a vm_pager object.
 *
 * Parameters:
 *   ops - the vm_pager_ops
 *   p   - pointer to private data structure used by the pager
 *   p_pager - where the pointer to the returned vm_pager object should be returned
 */
int vm_pager_create(struct vm_pager_ops *ops, void *p, struct vm_pager **p_pager)
{
	struct vm_pager *pager;

	pager = mem_alloc(sizeof(struct vm_pager));
	pager->ops = ops;
	pager->p = p;

	*p_pager = pager;
	return 0;
}

/*
 * Page in (read) data into given frame.
 */
int vm_pagein(struct vm_pager *pager, u32_t page_num, struct frame *frame)
{
	return pager->ops->read_page(pager, mem_frame_to_pa(frame), page_num);
}

/*
 * Page out (write) data contained in given frame.
 */
int vm_pageout(struct vm_pager *pager, u32_t page_num, struct frame *frame)
{
	return pager->ops->write_page(pager, mem_frame_to_pa(frame), page_num);
}

/*
 * Create a vm_pagecache using the given pager
 * as its underlying data store.
 */
int vm_pagecache_create(struct vm_pager *pager, struct vm_pagecache **p_obj)
{
	struct vm_pagecache *obj;

	obj = mem_alloc(sizeof(struct vm_pagecache));

	mutex_init(&obj->lock);
	cond_init(&obj->cond);
	frame_list_clear(&obj->pagelist);
	obj->pager = pager;

	*p_obj = obj;
	return 0;
}

/*
 * Lock a page in a vm_pagecache.
 * A page cannot be stolen from its vm_pagecache
 * while it is locked.
 *
 * Parameters:
 *   obj - the vm_pagecache
 *   page_num - which page to lock
 *   p_frame - where to return the pointer to the frame containing the page data
 */
int vm_lock_page(struct vm_pagecache *obj, u32_t page_num, struct frame **p_frame)
{
	int rc;
	struct frame *frame;

	mutex_lock(&obj->lock);

	/*
	 * See if page is already present.
	 */
	for (frame = frame_list_get_first(&obj->pagelist);
	     frame != 0;
	     frame = frame_list_next(frame)) {
		if (frame->vm_pgcache_page_num == page_num) {
			frame->refcount++; /* lock the frame! */
			break;
		}
	}

	if (frame == 0) {
		/*
		 * Page not present yet; allocate it and
		 * page in its contents.
		 */
		rc = vm_alloc_and_page_in(obj, page_num, p_frame);
	} else {
		/*
		 * Page is present; make sure its contents
		 * have been initialized.
		 */
		while (frame->content == PAGE_PENDING_INIT) {
			cond_wait(&obj->cond, &obj->lock);
		}

		if (frame->content == PAGE_FAILED_INIT) {
			/* the initial pagein failed; page contents not valid */
			rc = frame->errc;
			vm_release_frame_ref(obj, frame);
		} else {
			/* groovy */
			*p_frame = frame;
		}
	}

	mutex_unlock(&obj->lock);

	return rc;
}

/*
 * Unlock a page in a vm_pagecache.
 *
 * Parameters:
 *   obj - the vm_pagecache
 *   frame - the frame containing the page data
 */
int vm_unlock_page(struct vm_pagecache *obj, struct frame *frame)
{
	int rc = 0;

	mutex_lock(&obj->lock);

	KASSERT(frame->refcount > 0);
	frame->refcount--;

	mutex_unlock(&obj->lock);

	return rc;
}

