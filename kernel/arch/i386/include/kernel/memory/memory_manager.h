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
#ifndef __ARCH_I386_KERNEL_MEMORY__MEMORY_MANAGER_H
#define __ARCH_I386_KERNEL_MEMORY__MEMORY_MANAGER_H 1

#include <kernel/memory/memory_region.h>
#include <stdint.h>
#include <stdio.h>

static inline void mm_invlpg(uintptr_t addr) {
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
    asm volatile ("invlpg (%0)" : : "b"(addr) : "memory" );
}

static inline void mm_load_cr3(uintptr_t addr) {
	asm volatile ("mov %0, %%eax; mov %%eax, %%cr3" : : "r"(addr) : "%eax");
}

static inline void* mm_get_physaddr(void* vaddr) {
	uintptr_t pdindex = (uintptr_t)vaddr >> 22;
	uintptr_t ptindex = (uintptr_t)vaddr >> 12 & 0x03FF;
	
	uintptr_t* pd = (uintptr_t*)0xFFFFF000;
	// TODO: Check if page directory exists
	uintptr_t* pt = ((uintptr_t*)0xFFC00000) + (0x400 * pdindex);
	// TODO: Check if page table exists

	return (void*)((pt[ptindex] & ~0xFFF) + ((uintptr_t)vaddr & 0xFFF));
}

static inline void mm_map_page(void* paddr, void* vaddr, uint32_t flags) {
	uintptr_t pdindex = (uintptr_t)vaddr >> 22;
	uintptr_t ptindex = (uintptr_t)vaddr >> 12 & 0x03FF;
	page_t    page;

	uintptr_t* pd = (uintptr_t*)0xFFFFF000;
	uintptr_t* pt = ((uintptr_t*)0xFFC00000) + (0x400 * pdindex);

	if(pd[pdindex] == 0) {
		// Need to create page table
		if(memory_region_alloc_page(&page) != 0) {
			// Failed to allocate memory for papge table
		}
		pd[pdindex] = (page << PAGE_SHIFT) | 0x03;
		mm_invlpg(&pd[pdindex]);
		mm_invlpg(pt);
		memset(pt, 0, 4096);
	}
	
	if(pt[ptindex] != 0) {
		// The virtual address is already mapped	
		printf("Address already mapped: Page 0x%08x\n", (unsigned)(pt[ptindex]));
	}
	pt[ptindex] = ((uintptr_t)paddr) | (flags & 0xFFF) | 0x01;
	mm_invlpg((uintptr_t)vaddr);
}

#endif