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
#ifndef __KERNEL_MEMORY__MEMORY_MAP_H
#define __KERNEL_MEMORY__MEMORY_MAP_H 1

#include <stddef.h>
#include <stdint.h>

#define MEMORY_MAP_TYPE_AVAILABLE           1
#define MEMORY_MAP_TYPE_ACPI                3
#define MEMORY_MAP_TYPE_HIBERNATE           4
#define MEMORY_MAP_TYPE_INVALID             5

typedef struct _memory_map {
	uintptr_t addr;
	uintptr_t length;
	uint32_t type;
} memory_map_t;

void 		  memory_map_initialize();
void          memory_map_add_region(uintptr_t addr, uintptr_t size, uint32_t type);
memory_map_t* memory_map_begin();
memory_map_t* memory_map_end();
memory_map_t* memory_map_available_begin();
memory_map_t* memory_map_available_end();
void          memory_map_print();
void          memory_map_available_print();

#endif
