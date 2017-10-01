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
#ifndef __KERNEL_SCHEDULER__TASK_H
#define __KERNEL_SCHEDULER__TASK_H 1

#include <sys/spinlock.h>
#include <kernel/scheduler/registers.h>

typedef enum task_state {
	TASK_RUNNING = 0,
	TASK_INTERRUPTIBLE,
	TASK_UNINTERRUPTIBLE
} task_state_t;

typedef struct task {
	registers_t  regs;
	struct task* next;
	const char*  name;
	task_state_t state;
} task_t;

typedef void (*task_entry_point)();

#ifdef __cplusplus
extern "C" {
#endif

task_t* task_wrap(); // Wrap current thread in a task, only used during early init
task_t* task_inherit(const char* name, task_entry_point entry_point, void* stack);
void    task_set_state(task_t* task, task_state_t state);
task_t* task_create(const char* name, task_entry_point entry_point, uint32_t eflags, void* page_dir, void* stack);
void    task_switch(registers_t* from, registers_t* to);
int     task_signal_pending(task_t* task);

#ifdef __cplusplus
}
#endif

#endif