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
#ifndef __ARCH__I386_KERNEL_BOOT__GDT_H
#define __ARCH__I386_KERNEL_BOOT__GDT_H 1

#include <stdint.h>
#include <stdlib.h>
#include <kernel/panic.h>

#define GDT_SIZE 8

typedef struct _gdt {
	uintptr_t base;
	uintptr_t limit;
	uint8_t   type;
} gdt_t;

struct _gdt_desc
{
    unsigned short limit;
    uintptr_t base;
} __attribute__((packed));

typedef struct _gdt_desc gdt_desc_t;

static inline void gdt_encode(uint8_t* target, gdt_t source) {
	// Check the limit to make sure that it can be encoded
    if ((source.limit > 65536) && (source.limit & 0xFFF) != 0xFFF) {
        PANIC("Invalid GDT limit");
    }
    if (source.limit > 65536) {
        // Adjust granularity if required
        source.limit = source.limit >> 12;
        target[6] = 0xC0;
    } else {
        target[6] = 0x40;
    }
 
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] |= (source.limit >> 16) & 0xF;
 
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;
	
	target[5] = source.type;
}

static inline void gdt_reload(gdt_desc_t* desc) {
	asm __volatile__
	(
	   "lgdt (%0)\n"
	   "ljmp $0x08, $.i386_lgdt_jump\n"
	   ".i386_lgdt_jump:\n"
	   "movw $0x10, %%ax\n"
	   "movw %%ax, %%ds\n"
	   "movw %%ax, %%es\n"
	   "movw %%ax, %%fs\n"
	   "movw %%ax, %%gs\n"
	   "movw %%ax, %%ss\n"
	   :
	   : "r" (desc)
	);
}

#endif
