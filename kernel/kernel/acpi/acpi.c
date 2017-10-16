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
#include <stdio.h>

#define ACPI_1_RSDP_LEN 20

// static int acpi_verify_checksum(acpi_table_header_t* table) {
// 	uint8_t i;
// 	uint8_t checksum = 0;

// 	for(i = 0; i < table->length; i++) {
// 		checksum += ((uint8_t*)table)[i];
// 	}

// 	return checksum;
// }

static void acpi1_init(rsdp_desc_t* rsdp) {
	//acpi_table_header_t* header;
	uint8_t i;
	uint8_t checksum = 0;

	printf("ACPI 1.0: 0x%08X \n", (unsigned)rsdp->rsdt_address);

	for(i = 0; i < ACPI_1_RSDP_LEN; i++) {
		checksum += ((uint8_t*)rsdp)[i];
	}

	if(checksum != 0) {
		printf("Invalid ACPI table checksum!\n");
	}

	// header = (acpi_table_header_t*)(rsdp->rsdt_address + acpi_virtual_base());
	// if(acpi_verify_checksum(header) != 0) {
	// 	printf("Invalid ACPI table checksum!\n");
	// }
}

static void acpi2_init(rsdp_desc20_t* rsdp) {
	printf("ACPI 2.0: 0x%08X \n", (unsigned)rsdp);
}

void acpi_init(void) {
	rsdp_desc_t* acpi_rsdp = acpi_get_rsdp();
	if(acpi_rsdp->revision == 0) {
		acpi1_init(acpi_rsdp);
	}
	else {
		acpi2_init((rsdp_desc20_t*)acpi_rsdp);
	}
}

