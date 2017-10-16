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
#include <kernel/block/block_device.h>
#include <kernel/initcall.h>

// static bio_request_queue_t* request_queue_freelist;
// static spinlock_t           freelist_lock;

// static int earlyinit_block_device() {
// 	request_queue_freelist = NULL;
// 	freelist_lock = SPINLOCK_UNLOCKED;
// }

// early_initcall(earlyinit_block_device);

// bio_buffer_info_t* bio_get_buffer(device_t dev, loff_t block, ssize_t blocksize) {
// 	return NULL;
// }

// #define BIO_MAX_REQUESTS 32

// void bio_block_access(int __attribute__((unused)) access_type, uint32_t __attribute__((unused)) flags, bio_buffer_info_t __attribute__((unused)) * buffers, size_t __attribute__((unused)) buffers_len) {
// }

// void bio_wait_for_buffer(bio_buffer_info_t __attribute__((unused)) * buffer) {
// }

// bio_request_queue_t* bio_create_request_queue(void) {
// 	bio_request_queue_t* result;
// 	uint32_t flags;

// 	spinlock_acquire_irqsave(&freelist_lock, &flags);

// 	if(request_queue_freelist != NULL) {
// 		result = request_queue_freelist;
// 		request_queue_freelist = result->freelist_next;
// 		spinlock_release_irqload(&freelist_lock, &flags);
// 		result->freelist_next = NULL;
// 		result->request_head = NULL;
// 	}
// 	else {
// 		spinlock_release_irqload(&freelist_lock, &flags);
// 		result = malloc(sizeof(request_queue_t));
// 		result->freelist_next = NULL;
// 		result->request_head = NULL;
// 		result->request_list_lock = SPINLOCK_UNLOCKED;
// 		wait_queue_init(&result->wait_queue);
// 	}

// 	return result;
// }

// int bio_release_request_queue(bio_request_queue_t* queue) {
// 	return 0;
// }
