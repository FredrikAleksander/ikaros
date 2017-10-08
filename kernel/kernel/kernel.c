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

#include <stdio.h>
#include <kernel/panic.h>
#include <kernel/memory/memory_map.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/scheduler/wait.h>
#include <kernel/irq/timer.h>
#include <kernel/tty.h>
#include <kernel/pci/pci.h>
#include <kernel/ide/ide.h>
#include <kernel/ide/pci_ide.h>
#include <kernel/graphics/bochs_vga.h>
#include <kernel/initcall.h>

int               _ps2_key_counter;
int               _ps2_key;
wait_queue_head_t _ps2_queue;
char              _ps2_buffer[16];
int               _ps2_codelen;

void init_ps2() {
	_ps2_key_counter = 0;
	_ps2_key = 'Z';
	_ps2_codelen = 0;
	wait_queue_init(&_ps2_queue);
}

// Gets called by IRQ1 handler
void post_ps2() {
	// Wake up waiting threads
	_ps2_key_counter++;
	wait_wake_up_interruptible(&_ps2_queue);
}

int read_ps2() {
	int key = _ps2_key_counter;
	WAIT_EVENT_INTERRUPTIBLE(_ps2_queue, key != _ps2_key_counter);
	return _ps2_key;
}

void kash_main() {
	for(;;) {
		int ch = read_ps2();
		terminal_putchar(ch);
	}
	scheduler_exit(0);
}

void detect_boot_device() {
	scheduler_sleep(5000);
	printf("Finding boot media...\n");
	scheduler_exit(0);
}

void kernel_idle() {
	for(;;) {
		scheduler_yield(); // Release control if possible
		// TODO: Do idle work here, like indexing, caching, preallocating etc
	}
}

void kernel_main(const char * cmdline) {
	//terminal_initialize();
#ifndef CONFIG_SILENT
	printf("\e[44;37;1mIKAROS v0.1\e[K\e[40;37m");
	printf("Command Line: %s\n", cmdline);
#endif
	timer_init();
	init_ps2();

	scheduler_initialize();
	// Idle Thread must be present early on, as much driver code uses sleeping,
	// and there must always be a active task
	scheduler_create_thread("idle", kernel_idle);
	scheduler_create_thread("kboot", detect_boot_device);
	scheduler_create_thread("kash", kash_main);

	pci_init();
	invoke_initcall_bus();
	
	invoke_initcall_fs();
	
	ide_init();
	// Initialize PCI IDE Controller
	//pci_ide_init();
	//bochs_vga_init();
	invoke_initcall_devices();
	
	scheduler_exit(0);
}
