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
#include <kernel/initcall.h>
#include <kernel/debug/serial_debug.h>
#include <kernel/drivers/debug/pc_serial_debug.h>
#include <sys/io.h>

#define PORT 0x3f8   /* COM1 */

static void pc_serial_debug_install(struct serial_debug_driver __attribute__((unused)) const * driver, 
	struct serial_debug_device __attribute__((unused)) * device)
{
	outb_p(PORT + 1, 0x00);    // Disable all interrupts
	outb_p(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outb_p(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb_p(PORT + 1, 0x00);    //                  (hi byte)
	outb_p(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
	outb_p(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outb_p(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static void pc_serial_debug_remove(struct serial_debug_driver __attribute__((unused)) const * driver,
	struct serial_debug_device __attribute__((unused)) * device)
{
}

static void pc_serial_debug_putch(struct serial_debug_device __attribute__((unused)) * device,
	char __attribute__((unused)) ch)
{
	while((inb(PORT + 5) & 0x20) == 0);
	outb(PORT, ch);
}

void __pc_serial_debug_receive(void) {
	while(inb(PORT + 5) & 1) {
		// Read
		inb(PORT);
	}
	// if(inb(PORT + 5) & 0x20) {
	// 	// Send
	// }
}

const struct serial_debug_driver pc_serial_debug_driver = {
	.install = pc_serial_debug_install,
	.remove = pc_serial_debug_remove,
	.putch = pc_serial_debug_putch
};

extern int __init_pc_serial_debug(void);
int __init_pc_serial_debug(void) {
#ifdef CONFIG_DEBUG_SERIAL
	serial_debug_register_driver(&pc_serial_debug_driver);
#endif
	return INIT_OK;
}
