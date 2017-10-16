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
#include <kernel/scheduler/wait.h>
#include <kernel/scheduler/scheduler.h>
#include <stddef.h>

void wait_queue_init(wait_queue_head_t* queue) {
	queue->next = NULL;
	queue->lock = SPINLOCK_UNLOCKED;
}

void wait_init(wait_queue_t* wq) {
	wq->next = NULL;
	wq->task = running_task;
}

void wait_prepare_to_wait(wait_queue_head_t* queue, wait_queue_t* wq, int state) {
	unsigned long flags;
	spinlock_acquire_irqsave(&queue->lock, &flags);
	task_set_state(running_task, state);
	wait_add_to_queue(queue, wq);
	spinlock_release_irqload(&queue->lock, &flags);
}

void wait_finish(wait_queue_head_t* queue, wait_queue_t* wq) {
	unsigned long flags;
	spinlock_acquire_irqsave(&queue->lock, &flags);
	task_set_state(running_task, TASK_RUNNING);
	wait_remove_from_queue(queue, wq);
	spinlock_release_irqload(&queue->lock, &flags);
}

void wait_add_to_queue(wait_queue_head_t* queue, wait_queue_t* wq) {
	wq->next = queue->next;
	queue->next = wq;
}

void wait_remove_from_queue(wait_queue_head_t* queue, wait_queue_t* wq) {
	wait_queue_t* prev;
	wait_queue_t* curr;

	prev = NULL;
	curr = queue->next;

	while(curr != NULL) {
		if(curr == wq) {
			if(prev != NULL) {
				prev->next = curr->next;
			}
			else {
				queue->next = curr->next;
			}
			return;
		}
	}
}

void wait_wake_up_interruptible(wait_queue_head_t* queue) {
	unsigned long flags;
	spinlock_acquire_irqsave(&queue->lock, &flags);
	wait_queue_t* wq = queue->next;
	while(wq != NULL) {
		task_set_state(wq->task, TASK_RUNNING);
		queue->next = wq->next;
		wq = queue->next;
	}
	spinlock_release_irqload(&queue->lock, &flags);
}
