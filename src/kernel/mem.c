/*
 * GeekOS - memory allocation
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

#include <geekos/mem.h>
#include <geekos/int.h>
#include <geekos/string.h>
#include <geekos/thread.h>

#define HEAP_SIZE (512*1024)

IMPLEMENT_LIST_CLEAR(frame_list, frame)
IMPLEMENT_LIST_APPEND(frame_list, frame)
IMPLEMENT_LIST_IS_EMPTY(frame_list, frame)
IMPLEMENT_LIST_REMOVE_FIRST(frame_list, frame)
IMPLEMENT_LIST_GET_FIRST(frame_list, frame)
IMPLEMENT_LIST_REMOVE(frame_list, frame)
IMPLEMENT_LIST_NEXT(frame_list, frame)

static ulong_t s_numframes;
static struct frame *s_framelist;
static struct frame_list s_freelist;

static struct thread_queue s_heap_waitqueue;
static struct thread_queue s_frame_waitqueue;

struct scan_region_data {
	bool heap_created;
	unsigned heap_size;
	unsigned avail_pages;
};

static void mem_heap_init(ulong_t start, ulong_t end)
{
	extern char *g_heapstart, *g_heapend;

	g_heapstart = (char *) start;
	g_heapend   = (char *) end;
}

static void mem_set_region_state(ulong_t start, ulong_t end, frame_state_t state)
{
	ulong_t addr;
	KASSERT(start < end);
	for (addr = start; addr < end; addr += PAGE_SIZE) {
		struct frame *frame = &s_framelist[addr / PAGE_SIZE];
		frame->state = state;
		if (state == FRAME_AVAIL) {
			frame_list_append(&s_freelist, frame);
		}
	}
}

static ulong_t mem_scan_region(ulong_t start, ulong_t end, frame_state_t state, void *data_)
{
	struct scan_region_data *data = data_;

	/* try to create kernel heap, if appropriate */
	if (!data->heap_created && state == FRAME_AVAIL && (end - start) >= HEAP_SIZE) {
		mem_set_region_state(start, start + HEAP_SIZE, FRAME_HEAP);
		mem_heap_init(start, start + HEAP_SIZE);
		start += HEAP_SIZE;
		data->heap_created = true;
		data->heap_size = HEAP_SIZE;
	}

	/* set state of all frames in region, and add to freelist if appropriate */
	mem_set_region_state(start, end, state);
	if (state == FRAME_AVAIL) {
		data->avail_pages += (end - start) / PAGE_SIZE;
	}

	return end;
}

void mem_clear_bss(void)
{
	extern char __bss_start, end;
	memset(&__bss_start, '\0', &end - &__bss_start);
}

void mem_init(struct multiboot_info *boot_record)
{
	struct scan_region_data data = { 0 };

	mem_init_segments();
	mem_create_framelist(boot_record, &s_framelist, &s_numframes);
	mem_scan_regions(boot_record, &mem_scan_region, &data);

	PANIC_IF(!data.heap_created, "Couldn't create kernel heap!");

	cons_printf("Memory: %u bytes in heap, %u available pages\n",
		data.heap_size, data.avail_pages);
}

/*
 * Allocate a buffer in the kernel heap.
 * Suspends calling thread until enough memory
 * is available to satisfy the request.
 * The returned buffer is filled with zeroes.
 *
 * Parameters:
 *   size - size in bytes of buffer to allocate
 *
 * Returns:
 *   pointer to allocated, zero-filled buffer
 */
void *mem_alloc(size_t size)
{
	extern void *malloc(size_t);
	void *buf;
	bool iflag;

	iflag = int_begin_atomic();
	while ((buf = malloc(size)) == 0) {
		thread_wait(&s_heap_waitqueue);
	}
	int_end_atomic(iflag);

	/* fill buffer with zeroes */
	memset(buf, '\0', size);

	return buf;
}

/*
 * Free memory allocated with mem_alloc()
 */
void mem_free(void *p)
{
	extern void free(void *);
	extern char *g_heapstart, *g_heapend;
	bool iflag;

	if (!p) {
		return;
	}

	KASSERT(((char *) p) >= g_heapstart && ((char *) p) < g_heapend);

	iflag = int_begin_atomic();

	/* free the memory */
	free(p);

	/* wake up any threads waiting for memory */
	thread_wakeup(&s_heap_waitqueue);

	int_end_atomic(iflag);
}

/*
 * Allocate a physical memory frame.
 * Suspends calling thread until a frame is available.
 */
struct frame *mem_alloc_frame(frame_state_t initial_state, int initial_refcount)
{
	struct frame *frame;
	bool iflag;

	iflag = int_begin_atomic();

	while (frame_list_is_empty(&s_freelist)) {
		thread_wait(&s_frame_waitqueue);
	}

	frame = frame_list_remove_first(&s_freelist);
	frame->state = initial_state;
	frame->refcount = initial_refcount;

	int_end_atomic(iflag);

	return frame;
}

/*
 * Free a physical memory frame allocated with mem_alloc_frame().
 */
void mem_free_frame(struct frame *frame)
{
	bool iflag;

	KASSERT(frame->refcount == 0);

	iflag = int_begin_atomic();

	frame->state = FRAME_AVAIL;
	frame_list_append(&s_freelist, frame);

	/* wake up any threads waiting for a frame */
	thread_wakeup(&s_frame_waitqueue);

	int_end_atomic(iflag);
}

void *mem_frame_to_pa(struct frame *frame)
{
	ulong_t offset = frame - s_framelist;
	return (void *) (offset * PAGE_SIZE);
}

struct frame *mem_pa_to_frame(void *pa)
{
	ulong_t framenum = ((ulong_t) pa) / PAGE_SIZE;
	return &s_framelist[framenum];
}

ulong_t mem_round_to_page(ulong_t addr)
{
	if ((addr & PAGE_MASK) != addr) {
		addr = (addr & PAGE_MASK) + PAGE_SIZE;
	}
	return addr;
}

bool mem_is_page_aligned(ulong_t addr)
{
	return mem_round_to_page(addr) == addr;
}
