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
#ifndef __ARCH_I386_KERNEL_GRAPHCIS__BOCHS_VGA_H
#define __ARCH_I386_KERNEL_GRAPHICS__BOCHS_VGA_H 1

#include <kernel/pci/pci.h>
#include <stdint.h>
#include <stddef.h>

#define BOCHS_VGA_VERSION_1 0xB0C0
#define BOCHS_VGA_VERSION_2 0xB0C1
#define BOCHS_VGA_VERSION_3 0xB0C2
#define BOCHS_VGA_VERSION_4 0xB0C3
#define BOCHS_VGA_VERSION_5 0xB0C4
#define BOCHS_VGA_VERSION_6 0xB0C5

#define BOCHS_VBE_DISPI_IOPORT_INDEX 0x01CE
#define BOCHS_VBE_DISPI_IOPORT_DATA  0x01CF

#define BOCHS_VBE_DISPI_INDEX_ID          0
#define BOCHS_VBE_DISPI_INDEX_XRES        1
#define BOCHS_VBE_DISPI_INDEX_YRES        2
#define BOCHS_VBE_DISPI_INDEX_BPP         3
#define BOCHS_VBE_DISPI_INDEX_ENABLE      4
#define BOCHS_VBE_DISPI_INDEX_BANK        5
#define BOCHS_VBE_DISPI_INDEX_VIRT_WIDTH  6
#define BOCHS_VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define BOCHS_VBE_DISPI_INDEX_X_OFFSET    8
#define BOCHS_VBE_DISPI_INDEX_Y_OFFSET    9

#define BOCHS_VBE_DISPI_DISABLED 0
#define BOCHS_VBE_DISPI_ENABLED  1

#define BOCHS_VBE_DISPI_BANK_SIZE_KB 65536
#define BOCHS_VBE_DISPI_BANK_ADDRESS 0xA0000

typedef struct bochs_vga_device {
	pci_function_t* pci_function;
	size_t          fb_size;
	size_t          io_size;
	void*           fb_phys_addr;
	void*           io_phys_addr;
	void*           fb_virt_addr;
	void*           io_virt_addr;
} bochs_vga_device_t;


#ifdef __cplusplus
extern "C" {
#endif __cplusplus

void bochs_vga_init();
void bochs_vga_enable_vbe(bochs_vga_device_t* vga_device);
void bochs_vga_disable_vbe(bochs_vga_device_t* vga_device);
void bochs_vga_set_xres(bochs_vga_device_t* vga_device, uint16_t xres);
void bochs_vga_set_yres(bochs_vga_device_t* vga_device, uint16_t yres);
void bochs_vga_set_bpp(bochs_vga_device_t* vga_device, uint16_t bpp);
void bochs_vga_set_virtual_width(bochs_vga_device_t* vga_device, uint16_t vwidth);
void bochs_vga_set_virtual_height(bochs_vga_device_t* vga_device, uint16_t vheight);

#ifdef __cplusplus
}
#endif

#endif