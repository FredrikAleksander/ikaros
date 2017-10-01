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
#include <kernel/memory_map.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/tty.h>

int read_ps2();
void post_ps2();

void kash_main() {
	for(;;) {
		int ch = read_ps2();
		terminal_putchar(ch);
	}
	scheduler_exit(0);
}

void detect_boot_device() {
	printf("Finding boot media...\n");
	scheduler_exit(0);
}

void kernel_main(const char* __attribute__ ((unused)) cmdline) {
	//terminal_initialize();
#ifndef CONFIG_SILENT
	printf("\e[44;37;1mIKAROS v0.1\e[K\e[40;37m");
	//printf("Memory Map: \n");
	//memory_map_print();
	printf("Command Line: %s\n", cmdline);
#endif
	scheduler_initialize();
	//scheduler_create_thread("kboot", detect_boot_device);
	scheduler_create_thread("kash", kash_main);
	for(;;) {
		scheduler_yield(); // Release control if possible
		// TODO: Do idle work here, like indexing, caching, preallocating etc
	}
}
