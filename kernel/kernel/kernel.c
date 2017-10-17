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
#include <kernel/kernel.h>
#include <kernel/acpi/acpi.h>
#include <kernel/panic.h>
#include <kernel/memory/memory_map.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/scheduler/wait.h>
#include <kernel/irq/timer.h>
#include <kernel/pci/pci.h>
#include <kernel/graphics/bochs_vga.h>
#include <kernel/initcall.h>
#include <kernel/input/keyboard.h>
#include <kernel/console/console.h>
#include <sys/unicode.h>


static void kash_main(void) {
	for(;;) {
		int ch = kb_getch();
		console_putch((char)ch);
	}
	scheduler_exit(0);
}

static void kernel_idle(void) {
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
	acpi_init();
	//init_ps2();

	scheduler_initialize();
	// Idle Thread must be present early on, as much driver code uses sleeping,
	// and there must always be a active task
	scheduler_create_thread("idle", kernel_idle);
	
	console_debug();
	pci_init();
	invoke_initcall_bus();
	invoke_initcall_fs();
	invoke_initcall_devices();
	
	scheduler_create_thread("kash", kash_main);

	scheduler_exit(0);
}
