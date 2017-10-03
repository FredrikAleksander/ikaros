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
#ifndef __KERNEL_MEMORY__MEMORY_REGION_H
#define __KERNEL_MEMORY__MEMORY_REGION_H 1

#include <stdint.h>
#include <strings.h>
#include <kernel/memory/page.h>

#define MEMORY_BITMAP_SIZE 2048

typedef struct _memory_region {
	struct _memory_region* next;
	uintptr_t base;
	uintptr_t length;
	uint8_t   bitmap[MEMORY_BITMAP_SIZE];
} memory_region_t;

static inline int memory_region_overlap(memory_region_t* region, page_t min, page_t max) {
	if(max < region->base) {
		return 0;
	}
	if(min > region->base + region->length) {
		return 0;
	}
	return 1;
}

static inline void memory_region_setbit(memory_region_t* region, page_t page, int set) {
	uintptr_t element;
	uintptr_t index;
	while(region != 0 && page >= region->base + region->length) {
		region = region->next;
	}
	if(region != 0 && page >= region->base) {
		page   -= region->base;
		element = page >> 3;
		index   = 1 << (page - (element << 3));

		if(set) {
			region->bitmap[element] |= index;
		}
		else {
			region->bitmap[element] &= ~index;
		}
	}
}

static inline int memory_region_getbit(memory_region_t* region, page_t page) {
	uintptr_t element;
	uintptr_t index;
	while(region != 0 && page >= region->base + region->length) {
		region = region->next;
	}
	if(region != 0 && page > region->base)
	{
		page   -= region->base;
		element = page >> 3;
		index   = 1 << (page - (element << 3));

		if(region->bitmap[element] & index) {
			return 1;
		}
		return 0;
	}
	return -1;
}

static inline memory_region_t* memory_region_find_and_set(memory_region_t* region, memory_region_t* end, page_t* pout) {
	uint32_t i, n, x, y;
	uint32_t* bitmap;

	while(region != 0 && region != end) {
		bitmap = (uint32_t*)region->bitmap;
		n = region->length / 32;
		for(i = 0; i < n; i++) {
			x = ~bitmap[i];
			if(x != 0) {
				y = ffs(x) - 1;
				bitmap[i] |= 1 << y;
				*pout = region->base + i * 32 + y;
				return region;
			}
		}
		region = region->next;
	}
	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

void             memory_region_acquire();
void             memory_region_release();
void             memory_region_early_init(uintptr_t base, uintptr_t length);
void             memory_region_init();
void             memory_region_add(uintptr_t base, uintptr_t length);
memory_region_t* memory_region_enumerate();
int              memory_region_alloc_page(page_t* page);
void             memory_region_dealloc_page(page_t page);

#ifdef __cplusplus
}
#endif

#endif