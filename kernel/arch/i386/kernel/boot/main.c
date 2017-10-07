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
#include <kernel/acpi/acpi.h>
#include <kernel/boot/multiboot2.h>
#include <kernel/tty.h>
#include <kernel/memory/memory_map.h>
#include <kernel/memory/memory_region.h>
#include <kernel/memory/memory_manager.h>
#include <kernel/panic.h>
#include <kernel/scheduler/tss.h>
#include <kernel/boot/gdt.h>
#include <kernel/irq/irq.h>
#include <kernel/irq/pic.h>
#include <sys/io.h>
#include <sys/elf32.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/scheduler/wait.h>
#include <kernel/memory/paging.h>

#define _TOSTRING(x) #x
#define _TOSTRING_VALUE(x) _TOSTRING(x)

#ifdef CONFIG_IKAROS_CMDLINE
char* default_command_line = _TOSTRING_VALUE(CONFIG_IKAROS_COMMAND_LINE);
#else
char* default_command_line = "init=/bin/sh";
#endif

void kernel_main(const char* command_line);

static inline void vga_disable_cursor()
{
   outb(0x3D4, 0x0A); // LOW cursor shape port to vga INDEX register
   outb(0x3D5, 0x3f); //bits 6-7 must be 0 , if bit 5 set the cursor is disable  , bits 0-4 controll the cursor shape .
}

struct multiboot_boot_header {
	uint32_t total_size;
	uint32_t reserved;
	struct multiboot2_header_tag tags[0];
};

extern long boot_pagedir;
extern long boot_pagetab1;

extern uintptr_t kernel_heap_start;
extern uintptr_t kernel_heap_end;
extern uintptr_t kernel_heap;
extern void*     kernel_malloc_freelist;

extern uintptr_t _kernel_start;
extern uintptr_t _kernel_end;

tss_t          tss;
uint64_t       _gdt[10];
gdt_desc_t     gdt_desc;

void _init_tss() {
	// TODO: Init Task State Segment
}

void _init_gdt() {
	int i;
	gdt_t gdt[4];
	
	// Need to initialize TSS struct before GDT
	memset(&tss, 0, sizeof(tss_t));
	tss.ss = 0x10;
	tss.esp0 = 0; // TODO: Set this to syscall kernel stack base
	tss.trace = sizeof(tss_t);

	gdt[0].base = 0;
	gdt[0].limit = 0;
	gdt[0].type = 0;
	
	gdt[1].base = 0;
	gdt[1].limit = 0xffffffff;
	gdt[1].type = 0x9a;
	
	gdt[2].base = 0;
	gdt[2].limit = 0xffffffff;
	gdt[2].type = 0x92;

	gdt[3].base = (uintptr_t)&tss;
	gdt[3].limit = sizeof(tss_t);
	gdt[3].type = 0x89;

	memset(_gdt, 0, sizeof(uint64_t) * 10);
	for(i = 0; i < 4; i++) {
		gdt_encode((uint8_t*)&_gdt[i], gdt[i]);
	}
	
	gdt_desc.base = (uintptr_t)_gdt;
	gdt_desc.limit = 79;

	void* gdt_addr = &gdt_desc;
	gdt_reload(gdt_addr);
}


// TODO: Clean up this horrible mess. Split function, replace all absolute constants etc. 
// Also, alot of code here can be made portable with X86_64 with little effort, 
// move this to shared x86 code
void _multiboot2_main(struct multiboot_boot_header* info) {
	char* cmdline = default_command_line;
	struct multiboot2_header_tag*        tag = info->tags;
	struct multiboot2_mmap_entry*        mmap;
	struct multiboot2_tag_string*        cmdline_tag  = 0;
	struct multiboot2_tag_basic_meminfo* mem_tag      = 0;
	struct multiboot2_tag_mmap*          mmap_tag     = 0;
	struct multiboot2_tag_elf_sections*  elf_sections = 0;

	uintptr_t kernel_start;
	uintptr_t kernel_end;
	uintptr_t page_kernel_start;
	uintptr_t page_kernel_end;

	elf32_shdr_t* shdr;
	memory_map_t* memory_map;
	memory_region_t* region;
	uintptr_t min_size;
	uintptr_t max_size;
	uintptr_t size;
	uintptr_t tags_end = (uintptr_t)info + info->total_size;
	uintptr_t offset   = 0;
	uintptr_t i, j;
	uintptr_t flags;
	size_t entry_size;
	size_t num_entries;

	kernel_start = (uintptr_t)&_kernel_start;
	kernel_end   = (uintptr_t)&_kernel_end;
	page_kernel_start = (kernel_start - 0xC0000000) >> PAGE_SHIFT;
	page_kernel_end   = ((kernel_end   - 0xC0000000) >> PAGE_SHIFT) + 1;

	vga_disable_cursor();

	while((offset = (uintptr_t)tag) < tags_end) {
		switch(tag->type) {
			case MULTIBOOT2_TAG_TYPE_CMDLINE:
				cmdline_tag = (struct multiboot2_tag_string*)tag;
				cmdline = cmdline_tag->string;
				break;
			case MULTIBOOT2_TAG_TYPE_BASIC_MEMINFO:
				mem_tag = (struct multiboot2_tag_basic_meminfo*)tag;
				break;
			case MULTIBOOT2_TAG_TYPE_MMAP:
				mmap_tag = (struct multiboot2_tag_mmap*)tag;
				break;
			case MULTIBOOT2_TAG_TYPE_ELF_SECTIONS:
				elf_sections = (struct multiboot2_tag_elf_sections*)tag;
				break;
			default:
				break;
		}
		offset += tag->size;
		if(offset % 8 != 0) {
			offset += 8 - (offset % 8);
		}
		tag = (struct multiboot2_header_tag*)offset;
	}
	if(mem_tag == 0) {
		printf("Missing basic memory info\n");
		return;
	}
	if(mmap_tag == 0) {
		printf("Missing memory map\n");
		return;
	}
	//printf("Basic Memory:\n  Lower = %10uKB\n  Upper = %10uKB\n", mem_tag->mem_lower, mem_tag->mem_upper);

	// Initialize Memory Map
	entry_size = mmap_tag->entry_size;
	if(entry_size % 8 != 0) {
		entry_size += 8 - (entry_size % 8);
	}
	num_entries = (mmap_tag->size - 16) / entry_size;
	for(offset = 0; offset < num_entries; offset++) {
		mmap = (struct multiboot2_mmap_entry*)((char*)mmap_tag->entries + entry_size * offset);
		memory_map_add_region(mmap->addr, mmap->len, mmap->type);
	}

	// Initialize Memory Region
	// Do the initialization from upper memory.
	// Only up to 64 MB will be initialized initially. The rest
	// will be initialized lazily
	for(memory_map = memory_map_available_begin(); memory_map != memory_map_available_end(); ++memory_map) {
		if(memory_map->addr >= 0x00100000 && memory_map->type == 1) {
			// Found candidate
			max_size = (MEMORY_BITMAP_SIZE << 3) << PAGE_SHIFT;
			if(max_size > memory_map->length) {
				size = memory_map->length;
			}
			else {
				size = max_size;
			}
			memory_region_early_init(memory_map->addr >> PAGE_SHIFT, size >> PAGE_SHIFT);
 		}
	}

	min_size = (kernel_start - 0xC0000000) >> PAGE_SHIFT;
	max_size = (kernel_end   - 0xC0000000) >> PAGE_SHIFT;

	size = max_size - min_size;

	memory_region_acquire();
	region = memory_region_enumerate();
	while(region != 0) {
		if(memory_region_overlap(region, min_size, max_size)) {
			// Mark used bits
			if(region->base > min_size) {
				offset = region->base;
			}
			else {
				offset = min_size;
			}
			if(region->base + region->length < max_size) {
				tags_end = region->base + region->length;
			}
			else {
				tags_end = max_size;
			}
			for(;offset < tags_end; offset++) {
				memory_region_setbit(region, offset, 1);
			}
		}
		region = region->next;
	}
	memory_region_release();

	uintptr_t  pagedir = (uintptr_t)&boot_pagedir;
	uintptr_t  pagetb1 = (uintptr_t)&boot_pagetab1;
	page_t pagedir_page;
	page_t pagetbx_page;

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
	mm_invlpg(pagedir);
	mm_invlpg(pagetb1);
	mm_invlpg(0xFFFFF000);
	mm_invlpg(0xFFFFE000);

	memset(0xFFFFE000, 0, 4096);
	memset(0xFFFFF000, 0, 4096);

	// Point last entry of page directory to the page directory itself
	offset = 0xFFFFF000 + (4 * 1023);
	*((volatile uintptr_t*)offset) = (pagedir_page << PAGE_SHIFT) | 0x03;

	// Update page directory entry for 0xC0xxxxxx to point
	// to the new page table
	*((volatile uintptr_t*)(0xFFFFF000 + 4 * 768)) = (pagetbx_page << PAGE_SHIFT) | 0x03;
	
	// Map the kernel into the new page table at 0xFFFFE000,
	// using ELF sections to determine permissions
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
	mm_load_cr3(pagedir_page << PAGE_SHIFT);

	paging_init();
	memory_region_init();

	// Need to initialize the kernel heap. Simple stuff
	kernel_heap_start = kernel_end;
	if(kernel_heap_start % PAGE_SIZE != 0) {
		kernel_heap_start += PAGE_SIZE - (kernel_heap_start % PAGE_SIZE);
	}
	kernel_heap_end = kernel_heap_start;
	kernel_heap     = kernel_heap_start;
	kernel_malloc_freelist = 0;

	// Memory Management should be working now. Paging is setup properly
	// and safely for the kernel, and general purpose memory allocation
	// (malloc/free) should work.

	//init_ps2();

	_init_gdt();
	_init_tss();
	pic_init(0x20, 0x28);
	irq_init();

	acpi_init(0);

	// rsdp_desc_t* t = acpi_get_rsdp();
	// if(t != 0) {
	// 	printf("Found ACPI root system descriptor table\n");
	// }

	outb(PIC1_DATA, 0xFC);
	outb(PIC2_DATA, 0xFF);

	asm volatile( "sti" );

	kernel_main(cmdline);
}

void _main(void* mbd, unsigned int magic) {
	terminal_initialize();

	if(magic == 0x36D76289) {
		_multiboot2_main((struct multiboot_boot_header*)mbd);
	}
}