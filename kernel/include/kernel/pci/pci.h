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
#ifndef __ARCH_I386_KERNEL_PCI__PCI_H
#define __ARCH_I386_KERNEL_PCI__PCI_H

#include <kernel/pci/pci_config.h>

#define PCI_CLASS_UNKNOWN                 0x00
#define PCI_CLASS_MASS_STORAGE_CONTROLLER 0x01
#define PCI_CLASS_NETWORK_CONTROLLER      0x02
#define PCI_CLASS_DISPLAY_CONTROLLER      0x03
#define PCI_CLASS_MULTIMEDIA_CONTROLLER   0x04
#define PCI_CLASS_MEMORY_CONTROLLER       0x05
#define PCI_CLASS_BRIDGE_DEVICE           0x06
#define PCI_CLASS_SIMPLE_COMMUNICATION_CONTROLLER 0x07
#define PCI_CLASS_BASE_SYSTEM_PERIPHERAL  0x08
#define PCI_CLASS_INPUT_DEVICE            0x09
#define PCI_CLASS_DOCKING_STATION         0x0A
#define PCI_CLASS_PROCESSOR               0x0B
#define PCI_CLASS_SERIAL_BUS_CONTROLLER   0x0C
#define PCI_CLASS_WIRELESS_CONTROLLER     0x0D
#define PCI_CLASS_INTELLIGENT_IO          0x0E
#define PCI_CLASS_SATELITTE_COMMS_CONTROLLER 0x0F
#define PCI_CLASS_CRYPTO_CONTROLLER       0x10
#define PCI_CLASS_DATA_ACQ_SIGNAL_PROC_CONTROLLER 0x11
#define PCI_CLASS_UNDEFINED 0xFF

typedef struct pci_function pci_function_t;
typedef struct pci_device   pci_device_t;
typedef struct pci_bus      pci_bus_t;

struct pci_function {
	pci_function_t* prev;
	pci_function_t* next;
	pci_device_t*   device;
	uint8_t         number;
	uint16_t        vendor_id;
	uint16_t        device_id;
	uint8_t         class_code;
	uint8_t         subclass;
	uint8_t         progif;
};

struct pci_device {
	pci_device_t*   prev;
	pci_device_t*   next;
	uint8_t         bus;
	uint8_t         number;
	uint16_t        vendor_id;
	uint16_t        device_id;
	pci_function_t* functions;
};

static inline uint32_t pci_get_class_code(uint8_t bus, uint8_t slot, uint8_t function) {
	return pci_config_readl(bus, slot, function, 0x08);
}

static inline uint8_t pci_get_header_type(uint8_t bus, uint8_t slot, uint8_t function) {
	return pci_config_readw(bus, slot, function, 0x0E) & 0xFF;
}

static inline uint8_t pci_get_secondary_bus_number(uint8_t bus, uint8_t slot, uint8_t function) {
	return (pci_config_readl(bus, slot, function, 0x18) & 0x0000FF00) >> 8;
}

static inline uint32_t pci_get_device_id(uint8_t bus, uint8_t slot, uint8_t function) {
	uint16_t vendor;
	uint16_t device;

	if((vendor = pci_config_readw(bus, slot, function, 0)) != 0xFFFF) {
		device = pci_config_readw(bus, slot, function, 2);
		return (((uint32_t)vendor) << 16) | device;
	}
	return 0xFFFFFFFF;
}

static inline uint32_t pci_get_bar0(uint8_t bus, uint8_t slot, uint8_t func) {
	return pci_config_readl(bus, slot, func, 0x10);
}

static inline uint32_t pci_get_bar1(uint8_t bus, uint8_t slot, uint8_t func) {
	return pci_config_readl(bus, slot, func, 0x14);
}

static inline uint32_t pci_get_bar2(uint8_t bus, uint8_t slot, uint8_t func) {
	return pci_config_readl(bus, slot, func, 0x18);
}

static inline uint32_t pci_get_bar3(uint8_t bus, uint8_t slot, uint8_t func) {
	return pci_config_readl(bus, slot, func, 0x1c);
}

static inline uint32_t pci_get_bar4(uint8_t bus, uint8_t slot, uint8_t func) {
	return pci_config_readl(bus, slot, func, 0x20);
}

static inline uint32_t pci_get_bar5(uint8_t bus, uint8_t slot, uint8_t func) {
	return pci_config_readl(bus, slot, func, 0x24);
}

static inline void pci_set_bar0(uint8_t bus, uint8_t slot, uint8_t func, uint32_t value) {
	pci_config_writel(bus, slot, func,  0x10, value);
}

static inline void pci_set_bar1(uint8_t bus, uint8_t slot, uint8_t func, uint32_t value) {
	pci_config_writel(bus, slot, func,  0x14, value);
}

static inline void pci_set_bar2(uint8_t bus, uint8_t slot, uint8_t func, uint32_t value) {
	pci_config_writel(bus, slot, func,  0x18, value);
}

static inline void pci_set_bar3(uint8_t bus, uint8_t slot, uint8_t func, uint32_t value) {
	pci_config_writel(bus, slot, func,  0x1c, value);
}

static inline void pci_set_bar4(uint8_t bus, uint8_t slot, uint8_t func, uint32_t value) {
	pci_config_writel(bus, slot, func,  0x20, value);
}

static inline void pci_set_bar5(uint8_t bus, uint8_t slot, uint8_t func, uint32_t value) {
	pci_config_writel(bus, slot, func,  0x24, value);
}


#ifdef __cplusplus
extern "C" {
#endif

void            pci_init();
pci_device_t*   pci_enumerate_devices(pci_device_t* current);
pci_function_t* pci_enumerate_functions(pci_device_t* device, pci_function_t* current);

#ifdef __cplusplus
}
#endif

#endif