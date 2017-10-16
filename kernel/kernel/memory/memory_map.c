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
 #include <kernel/memory/memory_map.h>
 #include <string.h>
 #include <stdio.h>

 #define MAX_MEMORY_MAP_SIZE 32

 static memory_map_t _memory_map[MAX_MEMORY_MAP_SIZE];
 static size_t       _memory_map_size;

 static memory_map_t _memory_map_available[MAX_MEMORY_MAP_SIZE];
 static size_t       _memory_map_available_size;

 void memory_map_initialize(void) {
	 _memory_map_size = 0;
	 _memory_map_available_size = 0;
 }

 void memory_map_add_region(uintptr_t addr, uintptr_t length, uint32_t type) {
	memory_map_t* mmap;
	memory_map_t* mmap_available;

	if(_memory_map_size == MAX_MEMORY_MAP_SIZE) {
		return;
	}
	mmap = &_memory_map[_memory_map_size];
	mmap->addr   = addr;
	mmap->length = length;
	mmap->type   = type;
	_memory_map_size++;

	if(type == MEMORY_MAP_TYPE_AVAILABLE) {
		mmap_available = &_memory_map_available[_memory_map_available_size];
		mmap_available->addr = addr;
		mmap_available->length = length;
		mmap_available->type = type;
		_memory_map_available_size++;
	}
 }

memory_map_t* memory_map_begin(void) {
	 return _memory_map;
}

memory_map_t* memory_map_end(void) {
	 return &_memory_map[_memory_map_size];
}

memory_map_t* memory_map_available_begin(void) {
	return _memory_map_available;
}

 memory_map_t* memory_map_available_end(void) {
	 return &_memory_map_available[_memory_map_available_size];
 }

 static inline const char* _memory_map_type_str(uint32_t type) {
	 switch(type) {
		case MEMORY_MAP_TYPE_AVAILABLE:
			 return "Available";
		case MEMORY_MAP_TYPE_ACPI:
			return "ACPI";
		case MEMORY_MAP_TYPE_HIBERNATE:
			return "Hibernate";
		case MEMORY_MAP_TYPE_INVALID:
			return "Invalid";
		default:
			return "Reserved";
	 }
 }

void memory_map_print(void) {
	memory_map_t* tmmap;
	for(tmmap = memory_map_begin(); tmmap != memory_map_end(); ++tmmap) {
		printf("0x%08x - 0x%08x %s\n", (unsigned)tmmap->addr, (unsigned)tmmap->length, _memory_map_type_str(tmmap->type));
	}
 }

void memory_map_available_print(void) {
	memory_map_t* tmmap;
	for(tmmap = memory_map_available_begin(); tmmap != memory_map_available_end(); ++tmmap) {
		printf("0x%08x - 0x%08x %s\n", (unsigned)tmmap->addr, (unsigned)tmmap->length, _memory_map_type_str(tmmap->type));
	}
 }
