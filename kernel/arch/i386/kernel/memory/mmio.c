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
#include <kernel/memory/mmio.h>
#include <kernel/memory/page.h>
#include <kernel/memory/memory_manager.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/initcall.h>
#include <sys/spinlock.h>

static uint8_t*   kernel_mmio_start;
static uint8_t*   kernel_mmio_limit;
static uint8_t*   kernel_mmio_ptr;
static spinlock_t kernel_mmio_lock;

static int mmio_init(void) {
	kernel_mmio_start = (uint8_t*)(0xC0000000 + 0x20000000);
	kernel_mmio_limit = (uint8_t*)(0xFFC00000 - (4096 * 1024));
	kernel_mmio_ptr   = kernel_mmio_start;
	kernel_mmio_lock = SPINLOCK_UNLOCKED;

	return INIT_OK;
}

arch_initcall(mmio_init);

void* mmio_map(void* phys_addr, size_t size) {
	unsigned long flags;
	uintptr_t address;
	uintptr_t num_pages;
	uintptr_t i;
	uintptr_t offset    = (uintptr_t)phys_addr % PAGE_SIZE;
	void*     virt_addr = NULL;
	size               += offset;
	address             = (uintptr_t)phys_addr - offset;

	if(size % PAGE_SIZE != 0) {
		size += PAGE_SIZE - (size % PAGE_SIZE);
	}

	num_pages = size / PAGE_SIZE;

	spinlock_acquire_irqsave(&kernel_mmio_lock, &flags);

	if(kernel_mmio_ptr + size > kernel_mmio_limit) {
		spinlock_release_irqload(&kernel_mmio_lock, &flags);
		return NULL;
	}

	virt_addr = kernel_mmio_ptr;
	kernel_mmio_ptr += size;

	for(i = 0; i < num_pages; i++) {
		uint8_t* phys_offs = ((uint8_t*)address)   + i * PAGE_SIZE;
		uint8_t* virt_offs = ((uint8_t*)virt_addr) + i * PAGE_SIZE;
		mm_map_page(phys_offs, virt_offs, 0x03);
	}

	spinlock_release_irqload(&kernel_mmio_lock, &flags);

	return ((uint8_t*)virt_addr) + offset;
}
void  mmio_unmap(void __attribute__((unused)) * phys_addr, size_t __attribute__((unused)) size) {
}
