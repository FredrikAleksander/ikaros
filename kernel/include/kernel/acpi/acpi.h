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
#ifndef __ARCH_I386_KERNEL_ACPI__ACPI_H
#define __ARCH_I386_KERNEL_ACPI__ACPI_H 1

#include <stdint.h>

struct rsdp_desc {
	char     signature[8];
	uint8_t  checksum;
	char     oemid[6];
	uint8_t  revision;
	uint32_t rsdt_address;
} __attribute__ ((packed));

typedef struct rsdp_desc   rsdp_desc_t;

struct rsdp_desc20 {
	rsdp_desc_t rdsp;
	uint32_t    length;
	uint64_t    xsdt_address;
	uint8_t     extended_checksum;
	uint8_t     reserved[3];
} __attribute__ ((packed));

typedef struct rsdp_desc20 rsdp_desc20_t;

struct acpi_table_header {
	char     signature[4];
	uint32_t length;
	uint8_t  revision;
	uint8_t  checksum;
	char     oemid[6];
	char     oemtableid[8];
	uint32_t oemrevision;
	uint8_t  asl_compiler_id;
	uint32_t asl_compiler_revision;
} __attribute__ ((packed));

typedef struct acpi_table_header acpi_table_header_t;

struct acpi_mcfg_entry {
	uint64_t offset;
	uint16_t segment_group;
	uint8_t  pci_bus_start;
	uint8_t  pci_bus_end;
	uint32_t reserved;
} __attribute__ ((packed));

typedef struct acpi_mcfg_entry acpi_mcfg_entry_t;

struct acpi_table_mcfg {
	char              signature[4];
	uint32_t          length;
	uint8_t           revision;
	uint8_t           checksum;
	char              oemid[6];
	char              oemtableid[8];
	uint32_t          oemrevision;
	uint8_t           asl_compiler_id;
	uint32_t          asl_compiler_revision;
	acpi_mcfg_entry_t entries[0];
} __attribute__ ((packed));

typedef struct acpi_table_mcfg acpi_table_mcfg_t;


#ifdef __cplusplus
extern "C" {
#endif

uintptr_t          acpi_virtual_base(void);
void               acpi_init(void);
rsdp_desc_t*       acpi_get_rsdp(void);

#ifdef __cplusplus
}
#endif

#endif
