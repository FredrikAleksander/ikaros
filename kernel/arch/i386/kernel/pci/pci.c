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
#include <kernel/pci/pci.h>
#include <stdio.h>
#include <stdlib.h>

const char* pci_class_names[] = {
	"Unknown",
	"Mass Storage Controller",
	"Network Controller",
	"Display Controller",
	"Multimedia Controller",
	"Memory Controller",
	"Bridge Device",
	"Simple Communication Controller",
	"Base System Peripheral",
	"Input Device",
	"Docking Station",
	"Processor",
	"Serial Bus Controller",
	"Wireless Controller",
	"Intelligent I/O",
	"Satellite Communications Controller",
	"Cryptography Controller",
	"Data Acquisition and Signal Processing Controller",
};

static inline const char* pci_get_class_name(uint8_t class_code) {
	if(class_code <= PCI_CLASS_DATA_ACQ_SIGNAL_PROC_CONTROLLER) {
		return pci_class_names[class_code];
	}
	return "Undefined";
}

size_t        pci_device_count;
pci_device_t* pci_devices;

void pci_scan_bus(uint8_t bus);

static inline void pci_scan_device_function(pci_device_t* dev, uint8_t function) {
	uint32_t tmp;
	uint8_t class_code;
	uint8_t subclass;
	//uint8_t progif;
	uint8_t secondary_bus;
	pci_function_t* func;
	pci_function_t* prev;
	pci_function_t* curr;
	uint32_t devid = pci_get_device_id(dev->bus, dev->number, function);
	uint32_t device_id = devid & 0xFFFF;
	uint32_t vendor_id = (devid & 0xFFFF0000) >> 16;

	tmp        = pci_get_class_code(dev->bus, dev->number, function);
	subclass   = (tmp & 0xFF0000) >> 16;
	class_code = (tmp & 0xFF000000) >> 24;

	if(class_code == 0x06 && subclass == 0x04) {
		// PCI Bus
		secondary_bus = pci_get_secondary_bus_number(dev->bus, dev->number, function);
		pci_scan_bus(secondary_bus);
	}
	else {
		func = malloc(sizeof(pci_function_t));
		func->device = dev;
		func->number = function;
		func->vendor_id = vendor_id;
		func->device_id = device_id;
		func->next = NULL;

		prev = NULL;
		curr = dev->functions;
		while(curr != NULL) {
			prev = curr;
			curr = curr->next;
		}
		if(prev != NULL) {
			prev->next = func;
			func->prev = prev;
		}
		else {
			dev->functions = func;
		}
	}
}

static inline void pci_scan_device(uint8_t bus, uint8_t device) {
	//uint8_t function = 0;
	pci_device_t* dev;
	pci_device_t* prev;
	pci_device_t* curr;
	uint8_t function = 0;
	uint32_t devid = pci_get_device_id(bus, device, 0);
	uint32_t device_id = devid & 0xFFFF;
	uint32_t vendor_id = (devid & 0xFFFF0000) >> 16;
	uint8_t  header_type;

	if(devid == 0xFFFFFFFF) {
		return;
	}

	dev = malloc(sizeof(pci_device_t));
	dev->prev = NULL;
	dev->next = NULL;
	dev->bus = bus;
	dev->number = device;
	dev->vendor_id = vendor_id;
	dev->device_id = device_id;
	dev->functions = NULL;

	pci_scan_device_function(dev, 0);
	header_type = pci_get_header_type(bus, device, 0);

	// TODO: Enumerate functions
	if((header_type & 0x80) != 0) {
		// Multi function device
		for(function = 1; function < 8; function++) {
			if(pci_get_device_id(bus, device, function) == 0xFFFFFFFF)
				break;
			pci_scan_device_function(dev, function);
		}
	}

	prev = NULL;
	curr = pci_devices;
	while(curr != 0) {
		prev = curr;
		curr = curr->next;
	}
	if(prev != NULL) {
		prev->next = dev;
		dev->prev = prev;
	}
	else {
		pci_devices = dev;
	}

	pci_device_count++;
}

void pci_scan_bus(uint8_t bus) {
	uint8_t device;

	for(device = 0; device < 32; device++) {
		pci_scan_device(bus, device);
	}
}

static inline void pci_scan() {
	uint8_t function;
	uint8_t header_type;

	header_type = pci_get_header_type(0, 0, 0);
	if((header_type & 0x80) == 0) {
		// Single PCI Controller
		pci_scan_bus(0);
	}
	else {
		// Multiple PCI Controllers
		for(function = 0; function < 8; function++) {
			if(pci_get_device_id(0, 0, 0) == 0xFFFFFFFF) {
				break;
			}
			pci_scan_bus(function);
		}
	}
}

void pci_init() {
	pci_device_t*   dev;
	pci_function_t* func;
	pci_devices = NULL;
	pci_scan();

	dev = pci_devices;
	int i = 0;
	while(dev != NULL) {
		func = dev->functions;
		while(func != NULL) {
			printf("PCI Device Function %i: %04X:%04X\n", i, (uint32_t)func->vendor_id, (uint32_t)func->device_id);
			func = func->next;
			i++;
		}
		dev = dev->next;
	}
}

// static inline void pci_scan_device(uint8_t bus, uint8_t device) {
// 	pci_header_t header;
// 	uint8_t  function;
// 	uint32_t id = pci_get_device_id(bus, device);
// 	uint16_t vendor_id = (id & 0xFFFF0000) >> 16;
// 	uint16_t device_id = id & 0x0000FFFF;

// 	if(vendor_id == 0xFFFF) {
// 		return;
// 	}
// 	pci_check_function(bus, device, function);
// }

// static inline void pci_scan_bus(uint8_t bus) {
// 	uint8_t device;

// 	for(device = 0; device < 32; device++) {
// 		pci_scan_device(bus, device);
// 	}
// }