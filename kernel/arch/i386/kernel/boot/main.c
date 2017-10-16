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
#include <kernel/initcall.h>
#include <kernel/memory/memory_map.h>
#include <kernel/memory/memory_region.h>
#include <kernel/memory/memory_manager.h>
#include <kernel/memory/mmio.h>
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
#include <kernel/kernel.h>
#include <kernel/console/fbconsole.h>
#include <kernel/drivers/console/vga_console.h>
#include <kernel/input/keyboard.h>

#define _TOSTRING(x) #x
#define _TOSTRING_VALUE(x) _TOSTRING(x)

#ifdef CONFIG_IKAROS_CMDLINE
static char* default_command_line = _TOSTRING_VALUE(CONFIG_IKAROS_COMMAND_LINE);
#else
static char* default_command_line = "init=/bin/sh";
#endif

void kernel_main(const char* command_line);

struct multiboot_boot_header {
	uint32_t total_size;
	uint32_t reserved;
	struct multiboot2_header_tag tags[0];
};

extern uintptr_t kernel_heap_start;
extern uintptr_t kernel_heap_end;
extern uintptr_t kernel_heap;
extern void*     kernel_malloc_freelist;

extern uintptr_t _kernel_start;
extern uintptr_t _kernel_end;

extern struct multiboot2_tag_elf_sections*  elf_sections;

struct multiboot2_tag_elf_sections*  elf_sections;

extern void _init();
extern void _fini();

extern int __init_pc_serial_debug(void);

// TODO: Clean up this horrible mess. Split function, replace all absolute constants etc. 
// Also, alot of code here can be made portable with X86_64 with little effort, 
// move this to shared x86 code
static void _multiboot2_main(struct multiboot_boot_header* info) {
	char* cmdline = default_command_line;
	struct multiboot2_header_tag*        tag = info->tags;
	struct multiboot2_mmap_entry*        mmap;
	struct multiboot2_tag_string*        cmdline_tag  = NULL;
	struct multiboot2_tag_basic_meminfo* mem_tag      = NULL;
	struct multiboot2_tag_mmap*          mmap_tag     = NULL;
	struct multiboot2_tag_framebuffer*   framebuffer  = NULL;
	void* fb_virtual_address = NULL;
	uintptr_t fb_size = 0;

	uintptr_t kernel_start;
	uintptr_t kernel_end;
	memory_map_t* memory_map;
	memory_region_t* region;
	uintptr_t min_size;
	uintptr_t max_size;
	uintptr_t size;
	uintptr_t tags_end = (uintptr_t)info + info->total_size;
	uintptr_t offset   = 0;
	size_t entry_size;
	size_t num_entries;

	elf_sections = NULL;
	kernel_start = (uintptr_t)&_kernel_start;
	kernel_end   = (uintptr_t)&_kernel_end;

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
			case MULTIBOOT2_TAG_TYPE_FRAMEBUFFER:
				framebuffer = (struct multiboot2_tag_framebuffer*)tag;
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

	// TODO: Check if we booted via EFI, if so, use framebuffer console
	if(mem_tag == NULL) {
		return;
	}
	if(mmap_tag == NULL) {
		return;
	}

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
	while(region != NULL) {
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

	_init();

	invoke_initcall_early();
	memory_region_init();
	__init_pc_serial_debug();
	invoke_initcall_arch();

	//asm volatile("hlt");

	if(framebuffer) {
		fb_size = framebuffer->common.framebuffer_pitch *
			framebuffer->common.framebuffer_height *
			(framebuffer->common.framebuffer_bpp >> 3);
		fb_virtual_address = mmio_map(((void*)(uintptr_t)framebuffer->common.framebuffer_addr), fb_size);

		if(framebuffer->common.framebuffer_type == 2) {
			// Text Mode
			__init_vga_console(fb_virtual_address,
			framebuffer->common.framebuffer_width,
			framebuffer->common.framebuffer_height);
		}
		else {
			console_framebuffer_t* fb;
			size_t size = sizeof(console_framebuffer_t);
			if(framebuffer->common.framebuffer_type == 0) {
				size += framebuffer->framebuffer_palette_num_colors * sizeof(struct multiboot2_color);
			}
			fb = malloc(size);
			memset(fb, 0, sizeof(console_framebuffer_t));
			
			fb->width  = framebuffer->common.framebuffer_width;
			fb->height = framebuffer->common.framebuffer_height;
			fb->pitch  = framebuffer->common.framebuffer_pitch ? framebuffer->common.framebuffer_pitch : framebuffer->common.framebuffer_height;
			fb->bpp    = framebuffer->common.framebuffer_bpp;
			fb->data   = fb_virtual_address;
			uint8_t bpp_padded = fb->bpp;
			if(bpp_padded % 8 != 0) {
				bpp_padded += 8 - (bpp_padded % 8);
			}
			bpp_padded <<= 3;
			fb->private = malloc(bpp_padded * 256);
			memset(fb->private, 0, bpp_padded * 256);

			if(framebuffer->common.type == 0) {
				memcpy(fb->palette, framebuffer->framebuffer_palette,
					framebuffer->framebuffer_palette_num_colors * sizeof(struct multiboot2_color));
			}
			else {
				fb->flags |= FB_CONSOLE_FLAG_RGB;
				fb->format.red_mask_size   = framebuffer->framebuffer_red_mask_size;
				fb->format.red_position    = framebuffer->framebuffer_red_field_position;
				fb->format.green_mask_size = framebuffer->framebuffer_green_mask_size;
				fb->format.green_position  = framebuffer->framebuffer_green_field_position;
				fb->format.blue_mask_size  = framebuffer->framebuffer_blue_mask_size;
				fb->format.blue_position   = framebuffer->framebuffer_blue_field_position;
			}
			__init_fb_console(fb);
		}
	}
	else {
		__init_null_console();
	}
	
	kernel_main(cmdline);

	_fini();
}

extern void _main(void* mbd, unsigned int magic);

void _main(void* mbd, unsigned int magic) {
	if(magic == 0x36D76289) {
		_multiboot2_main((struct multiboot_boot_header*)mbd);
	}
}
