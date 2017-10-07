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
#include <kernel/irq/timer.h>
#include <kernel/scheduler/scheduler.h>
#include <stddef.h>

static uint64_t        timer_freq;
static uint64_t        timer_ticks_freq; // Numbe of ticks at timer_freq

void timer_init() {
	timer_freq = 18;
	timer_ticks_freq = 0;
}

static inline uint64_t ticks_to_ms(uint64_t ticks) {
	return (ticks * 1000) / timer_freq;
}

uint64_t timer_ticks() {
	return timer_ticks_freq;
}
uint64_t timer_frequency() {
	return timer_freq;
}
uint64_t timer_milliseconds() {
	return ticks_to_ms(timer_ticks_freq);
}
uint64_t timer_range_to_ms(uint64_t __attribute__((unused)) ticks_start, uint64_t __attribute__((unused)) ticks_end) {
	return ticks_to_ms(ticks_end - ticks_start);
}

void* timer_handler(void __attribute__ ((unused)) * ctx) {
	task_t* next;
	timer_ticks_freq++;
	next = scheduler_timer();
	if(next != NULL) {
	}
	return NULL;
}