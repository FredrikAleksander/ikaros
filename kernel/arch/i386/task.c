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
#include <kernel/scheduler/task.h>
#include <stdlib.h>
#include <string.h>

task_t* task_wrap() {
	task_t* task = malloc(sizeof(task_t));
	memset(task, 0, sizeof(task_t));
	asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(task->regs.cr3)::"%eax");
	asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(task->regs.eflags)::"%eax");
	task->state = TASK_RUNNING;
	return task;
}

task_t* task_inherit(const char* name, task_entry_point entry_point, void* stack) {
	task_t* task;
	task = malloc(sizeof(task_t));
	asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(task->regs.cr3)::"%eax");
	asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(task->regs.eflags)::"%eax");
	task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eip = (uint32_t) entry_point;
    task->regs.esp = (uint32_t) stack;
	task->next = 0;
	task->name = name;
	task->state = TASK_RUNNING;
	return task;
}

task_t* task_create(const char* name, task_entry_point entry_point, uint32_t eflags, void* page_dir, void* stack) {
	task_t* task = malloc(sizeof(task_t));
	task->regs.eax = 0;
    task->regs.ebx = 0;
    task->regs.ecx = 0;
    task->regs.edx = 0;
    task->regs.esi = 0;
    task->regs.edi = 0;
    task->regs.eflags = eflags;
    task->regs.eip = (uint32_t) entry_point;
    task->regs.cr3 = (uint32_t) page_dir;
    task->regs.esp = (uint32_t) stack;
	task->next = 0;
	task->name = name;
	task->state = TASK_RUNNING;
	return task;
}

void task_set_state(task_t* task, task_state_t state) {
	task->state = state;
}

int task_signal_pending(task_t __attribute__ ((unused)) * task) {
	return 0;
}