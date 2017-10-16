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
#include <kernel/acpi/acpi.h>
#include <kernel/boot/gdt.h>
#include <kernel/boot/idt.h>
#include <kernel/irq/irq.h>
#include <kernel/irq/pic.h>
#include <kernel/scheduler/tss.h>
#include <kernel/initcall.h>
#include <kernel/delay.h>
#include <string.h>
#include <stdio.h>

static tss_t          tss;
static uint64_t       _gdt[10];
static gdt_desc_t     gdt_desc;

static void init_tss(void) {
	// TODO: Init Task State Segment
}

static void init_gdt(void) {
	int i;
	gdt_t gdt[4];
	
	// Need to initialize TSS struct before GDT
	memset(&tss, 0, sizeof(tss_t));
	tss.ss = 0x10;
	tss.esp0 = NULL; // TODO: Set this to syscall kernel stack base
	tss.trace = sizeof(tss_t);

	gdt[0].base = 0;
	gdt[0].limit = 0;
	gdt[0].type = 0;
	
	gdt[1].base = 0;
	gdt[1].limit = 0xffffffff;
	gdt[1].type = 0x9a;
	
	gdt[2].base = 0;
	gdt[2].limit = 0xffffffff;
	gdt[2].type = 0x92;

	gdt[3].base = (uintptr_t)&tss;
	gdt[3].limit = sizeof(tss_t);
	gdt[3].type = 0x89;

	memset(_gdt, 0, sizeof(uint64_t) * 10);
	for(i = 0; i < 4; i++) {
		gdt_encode((uint8_t*)&_gdt[i], gdt[i]);
	}
	
	gdt_desc.base = (uintptr_t)_gdt;
	gdt_desc.limit = 79;

	void* gdt_addr = &gdt_desc;
	gdt_reload(gdt_addr);
}

extern void acpi_early_init(void);

static int init_x86(void) {
	init_gdt();
	init_tss();
	pic_init(0x20, 0x28);
	irq_init();
	acpi_early_init();
	timer_init();

	outb(PIC1_DATA, 0xFE);
	outb(PIC2_DATA, 0xFF);
	asm volatile( "sti" );

	// Delay loop calibration. Uses the Linux BogoMIPS method.
	// TODO: Alot of modern hardware has much better ways to
	// achieve sub-millisecond timing and delays.
	// Should support modern and more advanced timing methods,
	// such as:
	//	Invariant TSC
	//  Processor Frequency + RDTSC ( needs more ACPI stuff :S )
	//  HPET
	unsigned long ticks;
	unsigned long loopbit;
	int lps_precision = 8;

	loops_per_jiffy = (1<<12);
	while ((loops_per_jiffy <<= 1) != 0) {
		/* wait for "start of" clock tick */
		ticks = jiffies;
		while (ticks == jiffies)
			/* nothing */;
		/* Go .. */
		ticks = jiffies;
		__delay(loops_per_jiffy);
		ticks = jiffies - ticks;
		if (ticks)
			break;
	}

	loops_per_jiffy >>= 1;
	loopbit = loops_per_jiffy;
	while (lps_precision-- && (loopbit >>= 1)) {
		loops_per_jiffy |= loopbit;
		ticks = jiffies;
		while (ticks == jiffies)
			/* nothing */;
		ticks = jiffies;
		__delay(loops_per_jiffy);
		if (jiffies != ticks)   /* longer than 1 tick */
			loops_per_jiffy &= ~loopbit;
	}
	asm volatile( "cli" );
	
	outb(PIC1_DATA, 0xFC);
	outb(PIC2_DATA, 0xFF);

	asm volatile( "sti" );

	return INIT_OK;
}

arch_initcall(init_x86);
