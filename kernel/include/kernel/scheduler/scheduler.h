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
#ifndef __KERNEL_SCHEDULER__SCHEDULER_H
#define __KERNEL_SCHEDULER__SCHEDULER_H 1

#include <kernel/scheduler/task.h>
#include <kernel/irq/irq.h>

#ifdef __cplusplus
extern "C" {
#endif

task_t* running_task;

void scheduler_initialize();
void scheduler_create_thread(const char* name, task_entry_point entry_point);
void scheduler_enable_preemption();
void scheduler_disable_preemption();
void scheduler_sleep(uint64_t ms);
void scheduler_yield();
void scheduler_exit(int exit_code);

#ifdef __cplusplus
}
#endif

static inline void spinlock_acquire_irqsave(spinlock_t __attribute__((unused)) * p, unsigned long __attribute__((unused)) *flags) {
	irq_save_local(flags);
	irq_disable();
	spinlock_acquire(p);
}

static inline void spinlock_release_irqload(spinlock_t __attribute__((unused)) * p, unsigned long __attribute__((unused)) *flags) {
	spinlock_release(p);
	irq_load_local(flags);
}

#endif