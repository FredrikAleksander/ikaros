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

static inline const char* pci_get_class_name(uint8_t class_code, uint8_t subclass, uint8_t progif) {
	switch(class_code) {
		case PCI_CLASS_UNKNOWN:
			return "Unknown";
		case PCI_CLASS_MASS_STORAGE_CONTROLLER:
			switch(subclass) {
				case 0x00:
					return "SCSI Bus Controller";
				case 0x01:
					return "IDE Controller";
				case 0x02:
					return "Floppy Disk Controller";
				case 0x03:
					return "IPI Bus Controller";
				case 0x04:
					return "RAID Controller";
				case 0x05:
					return "ATA Controller";
				case 0x06:
					return "SATA Controller";
				case 0x07:
					return "Serial Attached SCSI";
			}
			return "Mass Storage Controller";
		case PCI_CLASS_NETWORK_CONTROLLER:
			switch(subclass) {
				case 0x00:
					return "Ethernet Controller";
				case 0x01:
					return "Token Ring Controller";
				case 0x02:
					return "FDDI Controller";
				case 0x03:
					return "ATM Controller";
				case 0x04:
					return "ISDN Controller";
				case 0x05:
					return "WorldFip Controller";
				case 0x06:
					return "PICMG 2.14 Multi Computing";
			}
			return "Network Controller";
		case PCI_CLASS_DISPLAY_CONTROLLER:
			switch(subclass) {
				case 0x00:
					if(progif == 0x01) {
						return "8512-Compatible Controller";
					}
					return "VGA-Compatible Controller";
				case 0x01:
					return "XGA Controller";
				case 0x02:
					return "3D Controller";
			}
			return "Display Controller";
		case PCI_CLASS_MULTIMEDIA_CONTROLLER:
			switch(subclass) {
				case 0x00:
					return "Video Device";
				case 0x01:
					return "Audio Device";
				case 0x02:
					return "Computer Telephony Device";
			}
			return "Multimedia Controller";
		case PCI_CLASS_MEMORY_CONTROLLER:
			switch(subclass) {
				case 0x00:
					return "RAM Controller";
				case 0x01:
					return "Flash Controller";
			}
			return "Memory Controller";
		case PCI_CLASS_BRIDGE_DEVICE:
			switch(subclass) {
				case 0x00:
					return "Host Bridge";
				case 0x01:
					return "ISA Bridge";
				case 0x02:
					return "EISA Bridge";
				case 0x03:
					return "MCA Bridge";
				case 0x04:
					return "PCI-to-PCI Bridge";
				case 0x05:
					return "PCMCIA Bridge";
				case 0x06:
					return "NuBus Bridge";
				case 0x07:
					return "CardBus Bridge";
				case 0x08:
					return "RACEway Bridge";
				case 0x09:
					return "PCI-to-PCI Bridge";
				case 0x0A:
					return "InfiniBrand-to-PCI Host Bridge";
			}
			return "Bridge Device";
		case PCI_CLASS_SIMPLE_COMMUNICATION_CONTROLLER:
			switch(subclass) {
				case 0x00:
					switch(progif) {
						case 0x00:
							return "Generic XT-Compatible Serial Controller";
						case 0x01:
							return "16450-Compatible Serial Controller";
						case 0x02:
							return "16550-Compatible Serial Controller";
						case 0x03:
							return "16650-Compatible Serial Controller";
						case 0x04:
							return "16750-Compatible Serial Controller";
						case 0x05:
							return "16850-Compatible Serial Controller";
						case 0x06:
							return "16950-Compatible Serial Controller";
					}
					return "Serial Controller";
				case 0x01:
					switch(progif) {
						case 0x00:
							return "Parallel Port";
						case 0x01:
							return "Bi-Directional Parallel Port";
						case 0x02:
							return "ECP 1.X Compliant Parallel Port";
						case 0x03:
							return "IEEE 1284 Controller";
						case 0x04:
							return "IEEE 1284 Target Device";
					}
					return "Parallel Port";
				case 0x02:
					return "Multiport Serial Controller";
				case 0x03:
					switch(progif) {
						case 0x00:
							return "Generic Modem";
						case 0x01:
							return "Hayes Compatible Modem (16450-Compatible Interface)";
						case 0x02:
							return "Hayes Compatible Modem (16550-Compatible Interface)";
						case 0x03:
							return "Hayes Compatible Modem (16650-Compatible Interface)";
						case 0x04:
							return "Hayes Compatible Modem (16750-Compatible Interface)";
					}
					return "Modem";
				case 0x04:
					return "IEEE 488.1/2 (GPIB) Controller";
				case 0x05:
					return "Smart Card";
			}
			return "Simple Communication Controller";
		case PCI_CLASS_BASE_SYSTEM_PERIPHERAL:
			switch(progif) {
				case 0x00:
					switch(progif) {
						case 0x00:
							return "Generic 8259 PIC";
						case 0x01:
							return "ISA PIC";
						case 0x02:
							return "EISA PIC";
						case 0x10:
							return "I/O APIC Interrupt Controller";
						case 0x20:
							return "I/O(x) APIC Interrupt Controller";
					}
					return "Interrupt Controller";
				case 0x01:
					switch(progif) {
						case 0x00:
							return "Generic 8254 DMA Controller";
						case 0x01:
							return "ISA DMA Controller";
						case 0x02:
							return "EISA DMA Controller";
					}
					return "DMA Controller";
				case 0x02:
					switch(progif) {
						case 0x00:
							return "Generic 8254 System Timer";
						case 0x01:
							return "ISA System Timer";
						case 0x02:
							return "EISA System Timer";
					}
					return "System Timer";
				case 0x03:
					switch(progif) {
						case 0x00:
							return "Generic RTC Controller";
						case 0x01:
							return "ISA RTC Controller";
					}
					return "RTC Controller";
				case 0x04:
					return "Generic PCI Hot-Plug Controller";
			}
			return "Base System Peripheral";
		case PCI_CLASS_INPUT_DEVICE:
			switch(subclass) {
				case 0x00:
					return "Keyboard Controller";
				case 0x01:
					return "Digitizer";
				case 0x02:
					return "Mouse Controller";
				case 0x03:
					return "Scanner Controller";
				case 0x04:
					switch(progif) {
						case 0x00:
							return "Gameport Controller (Generic)";
						case 0x10:
							return "Gameport Controller (Legacy)";
					}
					return "Gameport Controller";
			}
			return "Input Controller";
		case PCI_CLASS_DOCKING_STATION:
			switch(subclass) {
				case 0x00:
					return "Generic Docking Station";
			}
			return "Docking Station";
		case PCI_CLASS_PROCESSOR:
			switch(subclass) {
				case 0x00:
					return "386 Processor";
				case 0x01:
					return "486 Processor";
				case 0x02:
					return "Pentium Processor";
				case 0x10:
					return "Alpha Processor";
				case 0x20:
					return "PowerPC Processor";
				case 0x30:
					return "MIPS Processor";
				case 0x40:
					return "Co-Processor";
			}
		case PCI_CLASS_SERIAL_BUS_CONTROLLER:
			switch(subclass) {
				case 0x00:
					return "IEEE 1394 Controller";
				case 0x01:
					return "ACCESS bus";
				case 0x02:
					return "SSA";
				case 0x03:
					switch(progif) {
						case 0x00:
							return "USB Controller (UHCI)";
						case 0x10:
							return "USB Controller (OHCI)";
						case 0x20:
							return "USB Controller (EHCI)";
						case 0x30:
							return "USB Controller (XHCI)";
						case 0x80:
							return "Unspecified USB Controller";
						case 0xFE:
							return "USB";
					}
					return "USB Controller";
				case 0x04:
					return "Fibre Channel";
				case 0x05:
					return "SMBus";
				case 0x06:
					return "InfiniBand";
				case 0x07:
					switch(progif) {
						case 0x00:
							return "IPMI SMIC Interface";
						case 0x01:
							return "IPMI Kybd Controller Style Interface";
						case 0x02:
							return "IPMI Block Transfer Interface";
					}
				case 0x08:
					return "SERCOS Interface Standard (IEC 61491)";
				case 0x09:
					return "CANbus";
			}
			return "Serial Bus Controller";
		case PCI_CLASS_WIRELESS_CONTROLLER:
			switch(subclass) {
				case 0x00:
					return "iRDA Compatible Controller";
				case 0x01:
					return "Consumer IR Controller";
				case 0x10:
					return "RF Controller";
				case 0x11:
					return "Bluetooth Controller";
				case 0x12:
					return "Broadband Controller";
				case 0x20:
					return "Ethernet Controller (802.11a)";
				case 0x21:
					return "Ethernet Controller (802.11b)";
			}
			return "Wireless Controller";
		case PCI_CLASS_INTELLIGENT_IO:
			if(subclass == 0) {
				if(progif == 0)
					return "Message FIFO";
				return "I20 Architecture";
			}
			return "Intelligent I/O";
		case PCI_CLASS_SATELITTE_COMMS_CONTROLLER:
			switch(subclass) {
				case 0x01:
					return "TV Controller";
				case 0x02:
					return "Audio Controller";
				case 0x03:
					return "Voice Controller";
				case 0x04:
					return "Data Controller";
			}
			return "Satellite Communication Controller";
		case PCI_CLASS_CRYPTO_CONTROLLER:
			switch(subclass) {
				case 0x00:
					return "Network and Computing Encrpytion/Decryption";
				case 0x01:
					return "Entertainment Encryption/Decryption";
			}
			return "Encrpytion/Decryption";
		case PCI_CLASS_DATA_ACQ_SIGNAL_PROC_CONTROLLER:
			switch(subclass) {
				case 0x00:
					return "DPIO Modules";
				case 0x01:
					return "Performance Counters";
				case 0x10:
					return "Communications Syncrhonization Plus Time and Frequency Test/Measurment";
				case 0x20:
					return "Management Card";
			}
			return "Data Acquisition and Signal Processing Controller";
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
	uint8_t progif;
	uint8_t secondary_bus;
	pci_function_t* func;
	pci_function_t* prev;
	pci_function_t* curr;
	uint32_t devid = pci_get_device_id(dev->bus, dev->number, function);
	uint32_t device_id = devid & 0xFFFF;
	uint32_t vendor_id = (devid & 0xFFFF0000) >> 16;

	tmp        = pci_get_class_code(dev->bus, dev->number, function);
	progif     = tmp & 0xFF;
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
		func->class_code = class_code;
		func->subclass = subclass;
		func->progif = progif;
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
			if(pci_get_device_id(bus, device, function) != 0xFFFFFFFF)
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
			printf("PCI Device Function %i: %04X:%04X %s\n", i, (uint32_t)func->vendor_id, (uint32_t)func->device_id, pci_get_class_name(func->class_code, func->subclass, func->progif));
			func = func->next;
			i++;
		}
		dev = dev->next;
	}
}

pci_device_t* pci_enumerate_devices(pci_device_t* current) {
	if(current == NULL) {
		return pci_devices;
	}
	return current->next;
}

pci_function_t* pci_enumerate_functions(pci_device_t* device, pci_function_t* current) {
	if(current == NULL) {
		return device->functions;
	}
	if(current->device != device)
	{
		// TODO: PANIC
		return NULL;
	}
	return current->next;
}