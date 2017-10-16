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
#include <kernel/graphics/bochs_vga.h>
#include <kernel/memory/mmio.h>
#include <stdlib.h>
#include <stdio.h>

void bochs_vga_init(void) {
	bochs_vga_device_t* dev;
	pci_device_t*   pci_device;
	pci_function_t* pci_function;

	uint32_t        bar0; // Framebuffer Address
	uint32_t        bar2; // MMIO
	uint32_t        framebuffer_size;
	uint32_t        mmio_size;

	int found = 0;

	pci_function = NULL;
	pci_device = pci_enumerate_devices(NULL);
	while(pci_device != NULL) {
		pci_function = pci_enumerate_functions(pci_device, NULL);
		while(pci_function != NULL) {
			if(pci_function->vendor_id == 0x1234 && pci_function->device_id == 0x1111) {
				found = 1;
				break;
			}	
			pci_function = pci_enumerate_functions(pci_device, pci_function);
		}
		if(found) {
			break;
		}
		pci_device = pci_enumerate_devices(pci_device);
	}

	if(pci_function == NULL) {
		return;
	}

	bar0 = pci_get_bar0(pci_device->bus, pci_device->number, pci_function->number);
	bar2 = pci_get_bar2(pci_device->bus, pci_device->number, pci_function->number);

	pci_config_writel(pci_device->bus, pci_device->number, pci_function->number, 0x10, 0xFFFFFFFF);
	pci_config_writel(pci_device->bus, pci_device->number, pci_function->number, 0x18, 0xFFFFFFFF);
	framebuffer_size = (~(pci_get_bar0(pci_device->bus, pci_device->number, pci_function->number)&0xFFFFFFF0))+1;
	mmio_size        = (~(pci_get_bar2(pci_device->bus, pci_device->number, pci_function->number)&0xFFFFFFF0))+1;
	pci_config_writel(pci_device->bus, pci_device->number, pci_function->number, 0x10, bar0);
	pci_config_writel(pci_device->bus, pci_device->number, pci_function->number, 0x18, bar2);

	printf("BAR0: 0x%08X\nBAR2: 0x%08X\n", (unsigned)bar0, (unsigned)bar2);
	printf("FB: 0x%08X\nIO: 0x%08X\n", (unsigned)framebuffer_size, (unsigned)mmio_size);

	dev = malloc(sizeof(bochs_vga_device_t));
	dev->fb_size = framebuffer_size;
	dev->io_size = mmio_size;
	dev->pci_function = pci_function;
	dev->fb_phys_addr = (void*)(bar0&0xFFFFFFF0);
	dev->io_phys_addr = (void*)(bar2&0xFFFFFFF0);
	// dev->fb_virt_addr = mmio_map(dev->fb_phys_addr, dev->fb_size);
	// dev->io_virt_addr = mmio_map(dev->io_phys_addr, dev->io_size);

	/*
	outw_p(BOCHS_VBE_DISPI_IOPORT_INDEX, BOCHS_VBE_DISPI_INDEX_ID);
	uint16_t version = inw_p(BOCHS_VBE_DISPI_IOPORT_DATA);
	if(version == BOCHS_VGA_VERSION_1) {
		printf("bochs_vga: Version 1\n");
	}
	else if(version == BOCHS_VGA_VERSION_2) {
		printf("bochs_vga: Version 2\n");
	}
	else if(version == BOCHS_VGA_VERSION_3) {
		printf("bochs_vga: Version 3\n");
	}
	else if(version == BOCHS_VGA_VERSION_4) {
		printf("bochs_vga: Version 4\n");
	}
	else if(version == BOCHS_VGA_VERSION_5) {
		printf("bochs_vga: Version 5\n");
	}
	else if(version == BOCHS_VGA_VERSION_6) {
		printf("bochs_vga: Version 6\n");
	}
	else {
		return;
	}

	version = version - BOCHS_VGA_VERSION_1 + 1;

	// Disable VBE
	outw_p(BOCHS_VBE_DISPI_IOPORT_INDEX, BOCHS_VBE_DISPI_INDEX_ENABLE);
	outw_p(BOCHS_VBE_DISPI_IOPORT_DATA,  BOCHS_VBE_DISPI_DISABLED);

	uint16_t width  = 1024;
	uint16_t height = 768;
	uint16_t bpp    = 8;

	if(version > 2) {
		// Linear Framebuffer Support is available
		bpp = 32;
	}
	else {
		// Fallback
	}

	outw_p(BOCHS_VBE_DISPI_IOPORT_INDEX, BOCHS_VBE_DISPI_INDEX_XRES);
	outw_p(BOCHS_VBE_DISPI_IOPORT_DATA,  width);
	outw_p(BOCHS_VBE_DISPI_IOPORT_INDEX, BOCHS_VBE_DISPI_INDEX_YRES);
	outw_p(BOCHS_VBE_DISPI_IOPORT_DATA,  height);
	outw_p(BOCHS_VBE_DISPI_IOPORT_INDEX, BOCHS_VBE_DISPI_INDEX_BPP);
	outw_p(BOCHS_VBE_DISPI_IOPORT_DATA,  bpp);

	// Reenable VBE, this will apply selected resolution and bitdepth
	outw_p(BOCHS_VBE_DISPI_IOPORT_INDEX, BOCHS_VBE_DISPI_INDEX_ENABLE);
	outw_p(BOCHS_VBE_DISPI_IOPORT_DATA,  BOCHS_VBE_DISPI_ENABLED);*/
}
