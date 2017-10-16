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
#include <kernel/initcall.h>
#include <string.h>

#define ACPI_VIRTUAL_BASE 0xC0000000

static int rsdp_searched;
static rsdp_desc_t* rsdp_desc;

extern void acpi_early_init(void);

void acpi_early_init(void) {
	rsdp_searched = 0;
	rsdp_desc = NULL;
	rsdp_desc_t* rsdp;

	uintptr_t virtual_base = ACPI_VIRTUAL_BASE;
	uintptr_t offset = 0x000E0000;
	uintptr_t range  = 0x000FFFFF;

	rsdp_searched = 1;

	// Try to find ACPI RSDP Table
	while(offset < range) {
		rsdp = (rsdp_desc_t*)(offset + virtual_base);
		if(memcmp(rsdp->signature, "RSD PTR ", 8) == 0) {
			rsdp_desc = rsdp;
			break;
		}
		offset += 16;
	}
}

uintptr_t acpi_virtual_base(void) {
	return ACPI_VIRTUAL_BASE;
}

rsdp_desc_t* acpi_get_rsdp(void) {
	return rsdp_desc;
}
