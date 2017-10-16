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
#ifndef __KERNEL_BLOCK__BLOCK_DEVICE_H
#define __KERNEL_BLOCK__BLOCK_DEVICE_H 1

#include <kernel/compiler.h>
#include <kernel/device.h>
#include <kernel/types.h>
#include <kernel/scheduler/wait.h>
#include <stdint.h>
#include <stddef.h>

#define BLOCK_ACCESS_READ  0
#define BLOCK_ACCESS_WRITE 1

struct block_device;
struct bio_buffer_info;
struct bio_request;
struct bio_request_queue;

typedef struct block_device      block_device_t;
typedef struct bio_buffer_info   bio_buffer_info_t;
typedef struct bio_request       bio_request_t;
typedef struct bio_request_queue bio_request_queue_t;

typedef int (*bio_request_fn_t)(bio_buffer_info_t* buffer);
typedef void (*bio_complete_fn_t)(bio_buffer_info_t *buffer, int status);

struct bio_buffer_info {
	uint32_t ref_count;
	uint64_t block;
	size_t   size;
	uint8_t* data;	
	uint32_t status;
	bio_complete_fn_t complete_fn;
	void*             complete_data;
};

struct bio_request_queue {
	union {
		bio_request_fn_t     request_fn;
		bio_request_queue_t* freelist_next;
	};
	bio_request_t*    request_head;
	spinlock_t        request_list_lock;

	wait_queue_head_t wait_queue;
};

#ifdef __cplusplus
extern "C" {
#endif

int                  bio_buffer_stale(bio_buffer_info_t* buffer);
bio_buffer_info_t*   bio_get_buffer(device_t bio_dev, loff_t block, ssize_t blocksize);
void                 bio_block_access(int access_type, uint32_t flags, bio_buffer_info_t* buffers, size_t buffers_len);
void                 bio_wait_for_buffer(bio_buffer_info_t* buffer);
bio_request_queue_t* bio_create_request_queue(void);
int                  bio_release_request_queue(bio_request_queue_t* queue);

#ifdef __cplusplus
}
#endif

#endif

