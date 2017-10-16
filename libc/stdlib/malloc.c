/* 
Copyright (c) 2017, Fredrik A. Kristiansen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the IKAROS Project.                            
*/

#include <stdlib.h>
#include <stdio.h>
#ifdef __is_libk // libk
#include <kernel/compiler.h>
#include <kernel/memory/memory_region.h>
#include <kernel/memory/memory_manager.h>
#include <kernel/initcall.h>
#include <kernel/panic.h>
#include <kernel/scheduler/scheduler.h>

typedef struct _malloc_freelist {
	struct _malloc_freelist* next;
	size_t                   size;
#ifdef CONFIG_DEBUG_MALLOC
	// When debugging, compute a checksum on malloc,
	// and when freeing, verify it still checks out.
	// If any part of the freelist entry has changed,
	// it should fail, and that indicates there is code
	// with a out of bounds memory access bug.
	uint32_t                 checksum;
#endif
} malloc_freelist_t;

static uintptr_t kernel_heap_start;
static uintptr_t kernel_heap_end;
static uintptr_t kernel_heap;
static void*     kernel_malloc_freelist;
extern uintptr_t _kernel_end;
static spinlock_t heap_lock;
static spinlock_t freelist_lock;

static int malloc_init(void) {
	uintptr_t kernel_end   = (uintptr_t)&_kernel_end;

	kernel_heap_start = kernel_end;
	if(kernel_heap_start % PAGE_SIZE != 0) {
		kernel_heap_start += PAGE_SIZE - (kernel_heap_start % PAGE_SIZE);
	}
	kernel_heap_end = kernel_heap_start;
	kernel_heap     = kernel_heap_start;
	kernel_malloc_freelist = NULL;
	heap_lock = SPINLOCK_UNLOCKED;
	return INIT_OK;
}

early_initcall(malloc_init);

// Easy checksum for freelist entries.
#define COMPUTE_CHECKSUM(x) (((uint32_t)x->next) ^ ((uint32_t)x->size))

static inline int __must_hold(heap_lock) heap_grow(void) {
	// Allocate a page, and map it to virtual memory starting at kernel_heap_end
	page_t page;
	int    err;
	if((err = memory_region_alloc_page(&page)) != 0) {
		// Failed to allocate page
		return err;
	}
	mm_map_page((void*)(page << PAGE_SHIFT), (void*)kernel_heap_end, 0x03);
	kernel_heap_end += PAGE_SIZE;
	return 0;
}

static inline void freelist_acquire(unsigned long* flags) {
	spinlock_acquire_irqsave(&freelist_lock, flags);
}

static inline void freelist_release(unsigned long* flags) {
	spinlock_release_irqload(&freelist_lock, flags);
}

static inline void heap_acquire(unsigned long* flags) {
	spinlock_acquire_irqsave(&heap_lock, flags);
}

static inline void heap_release(unsigned long* flags) {
	spinlock_release_irqload(&heap_lock, flags);
}

#define MALLOC_ALIGN_BYTES 16

void* malloc(size_t num_bytes) {
	unsigned long flags;
	uintptr_t size;
	uintptr_t header_size = sizeof(malloc_freelist_t);
	uintptr_t num_bytes_aligned;

	malloc_freelist_t* freelist_prev;
	malloc_freelist_t* freelist;
	malloc_freelist_t* freelist_next;

	freelist_prev = NULL;
	freelist = kernel_malloc_freelist;

	if(header_size % MALLOC_ALIGN_BYTES != 0) {
		header_size +=MALLOC_ALIGN_BYTES - header_size % MALLOC_ALIGN_BYTES;
	}

	num_bytes_aligned = num_bytes;
	if(num_bytes_aligned % MALLOC_ALIGN_BYTES != 0) {
		num_bytes_aligned += MALLOC_ALIGN_BYTES - num_bytes_aligned % MALLOC_ALIGN_BYTES;
	}

	// If possible, reuse existing memory
	freelist_acquire(&flags);
	while(freelist != NULL) {
		size = freelist->size ^ 0xF;
		if(size >= num_bytes_aligned) {
			// Found a free block with available space
			if(size - num_bytes_aligned > 16) {
				// Enough space is available to split
				// up the freelist entry
				freelist_next = (struct _malloc_freelist*)((char*)freelist + size);
				freelist_next->next = freelist->next;
				freelist_next->size = size - num_bytes_aligned - MALLOC_ALIGN_BYTES;
				freelist->size = num_bytes_aligned;
#ifdef CONFIG_DEBUG_MALLOC
				freelist->checksum = COMPUTE_CHECKSUM(freelist);
#endif
				if(freelist_prev != NULL) {
					freelist_prev->next = freelist_next;
				}
				else {
					kernel_malloc_freelist = freelist_next;
				}
				freelist_release(&flags);
				return ((char*)freelist) + header_size;
			}
			else {
				if(freelist_prev != NULL) {
					freelist_prev->next = freelist->next;
				}
				freelist->size = num_bytes_aligned;
#ifdef CONFIG_DEBUG_MALLOC
				freelist->checksum = COMPUTE_CHECKSUM(freelist);
#endif
				freelist_release(&flags);
				return ((char*)freelist) + header_size;
			}
		}
		freelist_prev = freelist;
		freelist = freelist->next;
	}
	freelist_release(&flags);

	// Need to get memory from the heap
	size = header_size + num_bytes_aligned;
	heap_acquire(&flags);
	while(kernel_heap_end - kernel_heap < size) {
		if(heap_grow() != 0) {
			heap_release(&flags);
			return NULL;
		}
	}
	freelist = (malloc_freelist_t*)kernel_heap;
	kernel_heap += size;
	heap_release(&flags);
	freelist->next = NULL;
	freelist->size = num_bytes_aligned;
#ifdef CONFIG_DEBUG_MALLOC
	freelist->checksum = COMPUTE_CHECKSUM(freelist);
#endif
	return ((char*)freelist) + header_size;
}

// IMPROVEMENT: Keep track of number of freelist entries. Use it to compare 
// with heap size and occasionally try to compact the free list based on a 
// heuristic that gets updated based on previous runs, generating a error 
// correcting cycle for constantly improving the compaction rate.
void free(void* mem) {
	unsigned long lflags;
	malloc_freelist_t* prev;
	malloc_freelist_t* other;
	malloc_freelist_t* other_end;
	malloc_freelist_t* freelist_end;
	malloc_freelist_t* freelist;
	uint8_t flags;
	uintptr_t header_size = sizeof(malloc_freelist_t);
	if(header_size % MALLOC_ALIGN_BYTES != 0) {
		header_size += MALLOC_ALIGN_BYTES - (header_size % MALLOC_ALIGN_BYTES);
	}
	freelist = (malloc_freelist_t*)(((char*)mem) - header_size);
#ifdef CONFIG_DEBUG_MALLOC
	if(freelist->checksum != COMPUTE_CHECKSUM(freelist)) {
		PANIC("free: Invalid checksum, memory corruption detected");
	}
#endif
	if((flags = freelist->size & 0xF) != 0) {
		// TODO: For memory mapped files in the future
		return;
	}
	freelist_acquire(&lflags);
	prev  = NULL;
	other = kernel_malloc_freelist;

	while(other != NULL) {
		freelist_end = (malloc_freelist_t*)(((char*)freelist) + freelist->size + header_size);
		other_end    = (malloc_freelist_t*)(((char*)other) + other->size + header_size);

		if(freelist_end == other) {
			freelist->size += header_size + other->size;
			freelist->next = other->next;
			if(prev == NULL) {
				kernel_malloc_freelist = freelist;
			}
			else {
				prev->next = freelist;
			}
			freelist_release(&lflags);
			return; // TODO: Repeat process
		}
		else if(freelist == other_end) {
			other->size += header_size + freelist->size;
			freelist_release(&lflags);
			return;
		}

		prev  = other;
		other = other->next;
	}
	
	freelist->next = kernel_malloc_freelist;
	kernel_malloc_freelist = freelist;
	freelist_release(&lflags);
}

#else // libc

void* malloc(size_t __attribute__((unused)) num_bytes) {
	return NULL;
}

void free(void* __attribute__((unused)) mem) {
	(void)mem;
}
#endif
