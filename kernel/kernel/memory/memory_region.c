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
#include <kernel/memory/memory_region.h>
#include <kernel/memory/memory_map.h>
#include <string.h>
#include <stdio.h>

static memory_region_t  _initial_memory_region;
static memory_region_t* _root_memory_region;
static memory_region_t* _last_free_region;

static void _memory_region_acquire() {
	// TODO: Spinlock...
}

static void _memory_region_release() {
	// TODO: Spinlock...
}

void memory_region_acquire() { _memory_region_acquire(); }
void memory_region_release() { _memory_region_release(); }

void memory_region_init() {
}

void memory_region_early_init(uintptr_t base, uintptr_t length) {
	uintptr_t max_size = MEMORY_BITMAP_SIZE * 32;

	_memory_region_acquire();
	_initial_memory_region.next   = 0;
	_initial_memory_region.base   = base;
	_initial_memory_region.length = length > max_size ? max_size : length;
	memset(_initial_memory_region.bitmap, 0, MEMORY_BITMAP_SIZE);
	_root_memory_region = &_initial_memory_region;
	_last_free_region = _root_memory_region;
	_memory_region_release();
}

memory_region_t* memory_region_enumerate() {
	return _root_memory_region;
}

int memory_region_alloc_page(page_t* page) {
	memory_region_t* region;
	_memory_region_acquire();
	region = memory_region_find_and_set(_last_free_region, 0, page);
	if(region == 0 && _last_free_region != _root_memory_region) {
		region = memory_region_find_and_set(_root_memory_region, _last_free_region, page);
	}
	if(region == 0) {
		_memory_region_release();
		return -1;
	}
	_last_free_region = region;
	_memory_region_release();
	return 0;
}

void memory_region_dealloc_page(page_t page) {
	_memory_region_acquire();
	memory_region_setbit(_root_memory_region, page, 0);
	_memory_region_release();
}

// void memory_region_add(uintptr_t base, uintptr_t length) {
// 	if(length > 2048) {
// 		// Split
// 	}
// }