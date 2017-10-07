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
#ifndef __ARCH_I386_KERNEL_PCI__PCI_CONFIG_H
#define __ARCH_I386_KERNEL_PCI__PCI_CONFIG_H 1

#include <sys/io.h>
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static inline uint16_t pci_config_readw(uint8_t bus, uint8_t slot, 
	uint8_t func, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = func;
	uint16_t tmp = 0;

	address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
		(offset & 0xfc) | (uint32_t)0x80000000);

	outl_p(PCI_CONFIG_ADDRESS, address);
	tmp = (uint16_t)((inl_p(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffff);
	return tmp;
}

static inline uint32_t pci_config_readl(uint8_t bus, uint8_t slot, 
	uint8_t func, uint8_t offset)
{
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = func;
	uint16_t tmp = 0;

	address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
		(offset & 0xfc) | (uint32_t)0x80000000);

	outl_p(PCI_CONFIG_ADDRESS, address);
	return inl_p(PCI_CONFIG_DATA);
}

static inline void pci_config_writel(uint8_t bus, uint8_t slot, 
	uint8_t func, uint8_t offset, uint32_t data)
{
	uint32_t address;
	uint32_t lbus = bus;
	uint32_t lslot = slot;
	uint32_t lfunc = func;
	uint16_t tmp = 0;

	address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
		(offset & 0xfc) | (uint32_t)0x80000000);

	outl_p(PCI_CONFIG_ADDRESS, address);
	outl_p(PCI_CONFIG_DATA, data);
}

#endif