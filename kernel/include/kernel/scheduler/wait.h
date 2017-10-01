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
#ifndef __KERNEL_SCHEDULER__WAIT_H
#define __KERNEL_SCHEDULER__WAIT_H 1

#include <kernel/scheduler/task.h>
#include <kernel/scheduler/scheduler.h>

typedef struct wait_queue {
	struct wait_queue* next;
	task_t*            task;
} wait_queue_t;

typedef struct wait_queue_head {
	wait_queue_t* next;
	spinlock_t    lock;
} wait_queue_head_t;

#ifdef __cplusplus
extern "C" {
#endif

void wait_queue_init(wait_queue_head_t* queue);
void wait_init(wait_queue_t* wq);
void wait_prepare_to_wait(wait_queue_head_t* queue, wait_queue_t* wq, int state);
void wait_finish(wait_queue_head_t* queue, wait_queue_t* wq);
void wait_add_to_queue(wait_queue_head_t* queue, wait_queue_t* wq);
void wait_remove_from_queue(wait_queue_head_t* queue, wait_queue_t* wq);
void wait_wake_up_interruptible(wait_queue_head_t* queue);

#ifdef __cplusplus
}
#endif

#define __WAIT_EVENT_INTERRUPTIBLE(wq, condition, ret) \
	do { \
		wait_queue_t _wq; \
		wait_init(&_wq); \
		for(;;) { \
			wait_prepare_to_wait(&wq, &_wq, TASK_INTERRUPTIBLE); \
			if(condition) \
				break; \
			if(!task_signal_pending(running_task)) { \
				scheduler_yield(); \
				continue; \
			} \
			ret = -666; \
		} \
		wait_finish(&wq, &_wq); \
	} while (0)

#define WAIT_EVENT_INTERRUPTIBLE(wq, condition) ({\
	int __r = 0; \
	if(!(condition)) { \
		__WAIT_EVENT_INTERRUPTIBLE(wq, condition, __r); \
	} \
	__r; \
})

#endif