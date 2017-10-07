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
#include <kernel/scheduler/scheduler.h>
#include <kernel/scheduler/wait.h>
#include <kernel/irq/timer.h>
#include <kernel/panic.h>
#include <sys/spinlock.h>
#include <stdlib.h>

task_t* tasks;
task_t* tasks_tail;

task_t* running_task;

wait_queue_head_t timer_queue;

void scheduler_initialize() {
	wait_queue_init(&timer_queue);

	tasks = task_wrap("kernel");
	tasks_tail = tasks;
	tasks->next = tasks;
	running_task = tasks;
}

task_t* scheduler_timer() {
	wait_wake_up_interruptible(&timer_queue);
	return NULL;
}

task_t* scheduler_next_running_task(task_t* task) {
	task_t* curr = task->next;

	while(curr != 0 && curr->state != TASK_RUNNING && curr != task) {
		curr = curr->next;
	}

	return curr;
}

void scheduler_yield() {
	task_t* next;
	registers_t* from;

	next = scheduler_next_running_task(running_task);
	if(next == running_task)
		return;

	from         = &running_task->regs;
	running_task = next;

	task_switch(from, &running_task->regs);
}

void scheduler_sleep(uint64_t ms) {
	uint64_t sleep_start = timer_ticks();
	WAIT_EVENT_INTERRUPTIBLE(timer_queue, timer_range_to_ms(sleep_start, timer_ticks()) >= ms);
}

static inline void tasks_insert(task_t* task) {
	task->next       = tasks;
	tasks_tail->next = task;
	tasks_tail       = task;
}
static inline void tasks_remove(task_t* task) {
	task_t* curr;
	task_t* prev;

	if(task == tasks) {
		tasks_tail->next = task->next;
		tasks = task->next;
		return;
	}

	prev = tasks;
	curr = tasks->next;

	while(curr != tasks_tail && curr != task) {
		prev = curr;
		curr = curr->next;
	}

	if(curr == task) {
		prev->next = curr->next;
		if(curr == tasks_tail) {
			tasks_tail = prev;
		}
	}
}

static inline void scheduler_remove_task(task_t* task) {
	task_t* prev = tasks_tail;
	task_t* curr = tasks;

	if(task == tasks) {
		tasks_tail->next = task->next;
		tasks = task;
	}

	while(curr != tasks_tail) {
		if(curr == task) {
			prev->next = curr->next;
			if(curr == tasks) {
				tasks = prev;
				return;
			}
		}
		prev = curr;
		curr = curr->next;
	}
}

void scheduler_exit(int __attribute__ ((unused)) exit_code) {
	task_t* from;
	task_t* next;

	if(running_task->next == 0 || running_task->next == running_task) {
		// TODO: Shutdown system
		PANIC("SHUTDOWN");
	}

	from = running_task;
	next = running_task->next;
	running_task = running_task->next;

	tasks_remove(from);
	task_switch(&from->regs, &next->regs);
}

void scheduler_create_thread(const char* name, task_entry_point entry_point) {
	// TODO: Create a 'janitor' thread, that works when all tasks are idle
	// which preallocates tasks by preparing stacks and page directories. By
	// having tasks ready with all of kernel space and a stack already mapped, 
	// allows for lightweight task spawning by just returning a task from
	// the list
	task_t*  task;
	uint8_t* stack = malloc(4096); // BAD BAD IDEA. Replace quickly!

	task     = task_inherit(name, entry_point, stack + 4096);
	tasks_insert(task);
}