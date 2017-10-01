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
#ifndef __ARCH_I386_KERNEL__PIC_H
#define __ARCH_I386_KERNEL__PIC_H 1

#include <sys/io.h>

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI 0x20

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

static inline void pic_init(int pic1, int pic2) {
	uint8_t  a1, a2;
	//a1 = inb(PIC1_DATA);
	//a2 = inb(PIC2_DATA);

	outb_p(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);
	outb_p(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
	outb_p(PIC1_DATA, pic1);
	outb_p(PIC2_DATA, pic2); 
	outb_p(PIC1_DATA, 4); 
	outb_p(PIC2_DATA, 2);
	outb_p(PIC1_DATA, ICW4_8086);
	outb_p(PIC2_DATA, ICW4_8086);

	//outb(PIC1_DATA, a1);
	//outb(PIC2_DATA, a2);
}

static inline void pic_eoi(uint8_t irq) {
	if(irq >= 8) {
		outb(PIC2_COMMAND, PIC_EOI);
	}
	outb(PIC1_COMMAND, PIC_EOI);
}

static inline void pic_set_mask(uint32_t mask) {
	uint8_t  a1, a2;

	a1 = mask & 0xFF;
	a2 = (mask & 0xFF00) >> 8;

	outb(PIC1_DATA, a1);
	outb(PIC2_DATA, a2);
}

static inline uint32_t pic_get_mask() {
	uint8_t  a1, a2;
	a1 = inb(PIC1_DATA);
	a2 = inb(PIC2_DATA);

	return ((uint32_t)a1) | (((uint32_t)a2) << 8);
}

static inline void pic_setirqmask(uint8_t irq) {
	uint16_t port;
	uint8_t  value;

	if(irq < 8) {
		port = PIC1_DATA;
	} 
	else {
		port = PIC2_DATA;
		irq -= 8;
	}

	value = inb(port) | (1 << irq);
	outb(port, value);
}

static inline void pic_clrirqmask(uint8_t irq) {
	uint16_t port;
	uint16_t value;

	if(irq < 8) {
		port = PIC1_DATA;
	}
	else {
		port = PIC2_DATA;
		irq -= 8;
	}
	value = inb(port) & ~(1 << irq);
	outb(port, value);
}

#endif