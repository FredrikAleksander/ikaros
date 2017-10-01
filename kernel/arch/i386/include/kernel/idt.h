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
#ifndef __ARCH__I386__IDT_H
#define __ARCH__I386__IDT_H 1

#include <kernel/gdt.h>

struct _idt {
	uint16_t base_lower;
	uint16_t selector;
	uint8_t  ist;
	uint8_t  type;
	uint16_t base_higher;
} __attribute__ ((packed));

typedef struct _gdt_desc idt_desc_t;
typedef struct _idt      idt_t;

static inline void idt_reload(idt_desc_t* idt_desc) {
	asm __volatile__
	(
	   "lidt (%0)"
	   :
	   : "r" (idt_desc)
	);
}

static inline void idt_encode(idt_t* idt, void* base, uint16_t selector, uint8_t type) {
	idt->base_lower  = ((uintptr_t)base) & 0xFFFF;
	idt->selector    = selector;
	idt->ist         = 0;
	idt->type        = type;
	idt->base_higher = (((uintptr_t)base) & 0xFFFF0000) >> 16;
}

#endif