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
#include <kernel/irq/pic.h>

void pic_init(uint8_t pic1, uint8_t pic2) {
	//uint8_t  a1, a2;
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

	// Configure PIT.
	// Desired result is a timer that increments the system tick count
	// every millisecond. Because this isnt possible to get exactly right,
	// just keep track of all PIT ticks and convert to milliseconds on demand
}
