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
#include <kernel/boot/multiboot2.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/memory_region.h>
#include <kernel/memory/memory_manager.h>
#include <kernel/initcall.h>
#include <kernel/panic.h>
#include <sys/elf32.h>
#include <stdio.h>
#include <string.h>

#define PAGE_SIZE  4096
#define PAGE_SHIFT 12

extern long boot_pagedir;
extern long boot_pagetab1;
extern uintptr_t _kernel_start;
extern uintptr_t _kernel_end;

extern struct multiboot2_tag_elf_sections* elf_sections;

static inline void paging_init_kernel_space(void) {
	uintptr_t entry;
	uintptr_t  page;
	uintptr_t  start;
	uintptr_t  count = 0;
	uintptr_t  offset;
	uintptr_t  end   = 1024;

	for(start = 768; start < end; start++) {
		offset = 0xFFFFF000 + start * 4U;
		entry  = *(volatile uintptr_t*)offset;

		if((entry & 0x01) == 0) {
			// Missing page table, allocate and map
			if(memory_region_alloc_page(&page) != 0) {
				PANIC("Failed to initialize kernel address space\n");
			}
			*((volatile uintptr_t*)offset) = (page << PAGE_SHIFT) | 0x03;
			
			mm_invlpg((void*)(0xFFC00000 + start * 4096));
			memset((uintptr_t*)(0xFFC00000 + start * 4096), 0, 4096);

			count++;
		}
	}	
}

static int paging_earlyinit(void) {
	elf32_shdr_t* shdr;
	uintptr_t offset;
	uintptr_t i, j;
	uintptr_t flags;
	uintptr_t kernel_start;
	uintptr_t kernel_end;
	uintptr_t page_kernel_start;
	uintptr_t page_kernel_end;
	page_t pagedir_page;
	page_t pagetbx_page;
	uintptr_t  pagedir = (uintptr_t)&boot_pagedir;
	uintptr_t  pagetb1 = (uintptr_t)&boot_pagetab1;
	
	kernel_start = (uintptr_t)&_kernel_start;
	kernel_end   = (uintptr_t)&_kernel_end;
	page_kernel_start = (kernel_start - 0xC0000000) >> PAGE_SHIFT;
	page_kernel_end   = ((kernel_end   - 0xC0000000) >> PAGE_SHIFT) + 1;

	if(memory_region_alloc_page(&pagedir_page) != 0) {
		PANIC("Failed to allocate page for page table directory");
	}
	if(memory_region_alloc_page(&pagetbx_page) != 0) {
		PANIC("Failed to allocate page table");
	}
	
	// Map new page directory to 0xddFFF000 so it fits in the existing
	// page table
	offset = pagetb1 + (4 * 1023);
	*((volatile uintptr_t*)offset) = (pagedir_page << PAGE_SHIFT) | 0x03;
	
	// Map page for page table to 0xddFFE000
	offset = pagetb1 + (4 * 1022);
	*((volatile uintptr_t*)offset) = (pagetbx_page << PAGE_SHIFT) | 0x03;

	// Map the old page table to 0xFF000000, this
	// enables access to the page directory at 0xFFFFF000 
	offset = pagedir + (4 * 1023);
	*((volatile uintptr_t*)offset) = ((pagetb1 - 0xC0000000) & 0xFFFFF000) | 0x03;

	// Flush changes
	mm_invlpg((void*)pagedir);
	mm_invlpg((void*)pagetb1);
	mm_invlpg((void*)0xFFFFF000);
	mm_invlpg((void*)0xFFFFE000);

	memset((void*)0xFFFFE000, 0, 4096);
	memset((void*)0xFFFFF000, 0, 4096);

	// Point last entry of page directory to the page directory itself
	offset = 0xFFFFF000 + (4 * 1023);
	*((volatile uintptr_t*)offset) = (pagedir_page << PAGE_SHIFT) | 0x03;

	// Update page directory entry for 0xC0xxxxxx to point
	// to the new page table
	*((volatile uintptr_t*)(0xFFFFF000 + 4 * 768)) = (pagetbx_page << PAGE_SHIFT) | 0x03;

	for(i = 0; i < 1024; i++) {
		flags = 0;
		if(i < page_kernel_start) {
			// Map first 1MB of physical memory to 0xC0000000
			flags |= 0x03;
			*((volatile uintptr_t*)(0xFFFFE000 + i * 4)) = (i << PAGE_SHIFT) | flags;
		}
		else if(i < page_kernel_end) {
			// Virtual Address
			offset = (i << PAGE_SHIFT) + 0xC0000000;
			for(j = 0; j < elf_sections->num; j++) {
				shdr = (elf32_shdr_t*)((uintptr_t)elf_sections->sections + elf_sections->entsize * j);
				if(offset >= shdr->sh_addr && offset < shdr->sh_addr + shdr->sh_size) {
					if(shdr->sh_flags & SHF_ALLOC) {
						flags |= 0x01;
					}
					if(shdr->sh_flags & SHF_WRITE) {
						flags |= 0x02;
					}
					break;
				}
			}			
			*((volatile uintptr_t*)(0xFFFFE000 + i * 4)) = (i << PAGE_SHIFT) | flags;
		}
		else {
			*((volatile uintptr_t*)(0xFFFFE000 + i * 4)) = 0;
		}
	}

	// Write protect the page for the old page directory
	i = (pagedir - 0xC0000000) >> PAGE_SHIFT;
	*((volatile uintptr_t*)(0xFFFFE000 + i * 4))  = (i << PAGE_SHIFT) | 0x01;

	// TODO: Fill kernel address space with empty page tables.
	// All processes should have the kernel in the address space
	// and updating the address spaces of each process as the kernel
	// address space changes is complex. Much better to setup empty page
	// tables, which can be reused in other address spaces, and automatically
	// get updated everywhere

	// Reload CR3 with new page directory
	mm_load_cr3((void*)(pagedir_page << PAGE_SHIFT));

	paging_init_kernel_space();

	return INIT_OK;
}

early_initcall(paging_earlyinit);

