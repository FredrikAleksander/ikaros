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
#include <kernel/ide/pci_ide.h>
#include <kernel/pci/pci.h>
#include <sys/io.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct pci_ide_channel_regs {
	uint16_t base;
	uint16_t ctrl;
	uint16_t bm_ide;
	uint8_t  nien;
} pci_ide_channel_regs_t;

typedef struct pci_ide_controller {
	ide_controller_t*         prev;
	ide_controller_t*         next;
	struct ide_controller_ops ops;
	pci_ide_channel_regs_t    channels[2];
} pci_ide_controller_t;

void pci_ide_write(pci_ide_controller_t* controller, uint8_t channel, uint8_t reg, uint8_t data) {
	if(reg > ATA_REG_COMMAND && reg < ATA_REG_CONTROL) {
		pci_ide_write(controller, channel, ATA_REG_CONTROL, 0x80 | controller->channels[channel].nien);
	}
	if(reg < ATA_REG_SECCOUNT1) {
		outb(controller->channels[channel].base + reg - 0x00, data);
	}
	else if(reg < ATA_REG_CONTROL) {
		outb(controller->channels[channel].base + reg - 0x06, data);
	}
	else if(reg < 0x0E) {
		outb(controller->channels[channel].ctrl + reg - 0x0A, data);
	}
	else if(reg < 0x16) {
		outb(controller->channels[channel].bm_ide + reg - 0x0E, data);
	}
	if(reg > ATA_REG_COMMAND && reg < ATA_REG_CONTROL) {
		pci_ide_write(controller, channel, ATA_REG_CONTROL, controller->channels[channel].nien);
	}
}

uint8_t pci_ide_read(pci_ide_controller_t* controller, uint8_t channel, uint8_t reg) {
	uint8_t result = 0;
	if(reg > ATA_REG_COMMAND && reg < ATA_REG_CONTROL) {
		pci_ide_write(controller, channel, ATA_REG_CONTROL, 0x80 | controller->channels[channel].nien);
	}
	if(reg < ATA_REG_SECCOUNT1) {
		result = inb(controller->channels[channel].base + reg - 0x00);
	}
	else if(reg < ATA_REG_CONTROL) {
		result = inb(controller->channels[channel].base + reg - 0x06);
	}
	else if(reg < 0x0E) {
		result = inb(controller->channels[channel].ctrl + reg - 0x0A);
	}
	else if(reg < 0x16) {
		result = inb(controller->channels[channel].bm_ide + reg - 0x0E);
	}
	if(reg > ATA_REG_COMMAND && reg < ATA_REG_CONTROL)
	{
		pci_ide_write(controller, channel, ATA_REG_CONTROL, controller->channels[channel].nien);
	}
	return result;
}

static inline void pci_buffer_read(uint16_t reg, uint32_t* buffer, uint32_t size) {
	for(uint32_t i = 0; i < size; i++) {
		buffer[i] = inl(reg);
	}
}

uint8_t pci_ide_read_buffer(pci_ide_controller_t* controller, uint8_t channel, uint8_t reg, uint32_t* buffer, uint32_t size) {
	uint8_t result = 0;
	if(reg > ATA_REG_COMMAND && reg < ATA_REG_CONTROL) {
		pci_ide_write(controller, channel, ATA_REG_CONTROL, 0x80 | controller->channels[channel].nien);
	}
	if(reg < ATA_REG_SECCOUNT1) {
		pci_buffer_read(controller->channels[channel].base + reg - 0x00, (uint32_t*)buffer, size);
		//result = inb(controller->channels[channel].base + reg - 0x00);
	}
	else if(reg < ATA_REG_CONTROL) {
		pci_buffer_read(controller->channels[channel].base + reg - 0x06, (uint32_t*)buffer, size);
		//result = inb(controller->channels[channel].base + reg - 0x06);
	}
	else if(reg < 0x0E) {
		pci_buffer_read(controller->channels[channel].base + reg - 0x0A, (uint32_t*)buffer, size);
		//result = inb(controller->channels[channel].ctrl + reg - 0x0A);
	}
	else if(reg < 0x16) {
		pci_buffer_read(controller->channels[channel].base + reg - 0x0E, (uint32_t*)buffer, size);
		//result = inb(controller->channels[channel].bm_ide + reg - 0x0E);
	}
	if(reg > ATA_REG_COMMAND && reg < ATA_REG_CONTROL)
	{
		pci_ide_write(controller, channel, ATA_REG_CONTROL, controller->channels[channel].nien);
	}
	return result;
}

void pci_ide_initialize_controller(pci_function_t __attribute__((unused)) * pci) {
	uint32_t bar0, bar1, bar2, bar3, bar4;
	uint8_t bus, slot, func;
	uint8_t irq_channel0;
	uint8_t irq_channel1;
	pci_ide_controller_t* controller = malloc(sizeof(pci_ide_controller_t));

	bus = pci->device->bus;
	slot = pci->device->number;
	func = pci->number;

	bar0 = pci_get_bar0(bus, slot, func) & 0xFFFFFFFC;
	bar1 = pci_get_bar1(bus, slot, func) & 0xFFFFFFFC;
	bar2 = pci_get_bar2(bus, slot, func) & 0xFFFFFFFC;
	bar3 = pci_get_bar3(bus, slot, func) & 0xFFFFFFFC;
	bar4 = pci_get_bar4(bus, slot, func) & 0xFFFFFFFC;

	controller->prev = NULL;
	controller->next = NULL;
	controller->ops.read =  (ide_read_fn)pci_ide_read;
	controller->ops.write = (ide_write_fn)pci_ide_write;
	controller->ops.read_buffer = (ide_read_buffer_fn)pci_ide_read_buffer;
	controller->channels[ATA_PRIMARY].base     = bar0 ? bar0 : 0x1F0;
	controller->channels[ATA_PRIMARY].ctrl     = bar1 ? bar1 : 0x3F6;
	controller->channels[ATA_PRIMARY].bm_ide   = bar4;
	controller->channels[ATA_SECONDARY].base   = bar2 ? bar2 : 0x170;
	controller->channels[ATA_SECONDARY].ctrl   = bar3 ? bar3 : 0x376;
	controller->channels[ATA_SECONDARY].bm_ide = bar4 + 8;

	pci_ide_write(controller, ATA_PRIMARY, ATA_REG_CONTROL, 2);
	pci_ide_write(controller, ATA_SECONDARY, ATA_REG_CONTROL, 2);

	ide_add_controller((ide_controller_t*)controller);
}

void pci_ide_init() {
	uint32_t tmp;
	uint8_t class_code;
	uint8_t subclass;
	pci_device_t*   pci_device;
	pci_function_t* pci_function;

	pci_function = NULL;
	pci_device = pci_enumerate_devices(NULL);
	while(pci_device != NULL) {
		pci_function = pci_enumerate_functions(pci_device, NULL);
		while(pci_function != NULL) {
			tmp = pci_get_class_code(pci_device->bus, pci_device->number, pci_function->number);
			subclass   = (tmp & 0xFF0000) >> 16;
			class_code = (tmp & 0xFF000000) >> 24;

			if(class_code == PCI_CLASS_MASS_STORAGE_CONTROLLER && subclass == 0x01) {
				pci_ide_initialize_controller(pci_function);
			}
			pci_function = pci_enumerate_functions(pci_device, pci_function);
		}
		pci_device = pci_enumerate_devices(pci_device);
	}
}