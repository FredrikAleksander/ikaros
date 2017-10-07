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

#ifndef __KERNEL_IDE__IDE_H
#define __KERNEL_IDE__IDE_H 1

#include <kernel/ata/ata.h>
#include <stdint.h>

typedef struct ide_controller ide_controller_t;

typedef uint8_t (*ide_read_fn) (ide_controller_t* controller, uint8_t channel, uint8_t reg);
typedef uint8_t (*ide_read_buffer_fn) (ide_controller_t* controller, uint8_t channel, uint8_t reg, uint32_t* buffer, uint32_t size);
typedef void    (*ide_write_fn)(ide_controller_t* controller, uint8_t channel, uint8_t reg, uint8_t data);

struct ide_controller_ops {
	ide_read_fn  read;
	ide_write_fn write;
	ide_read_buffer_fn read_buffer;
};

struct ide_controller {
	struct ide_controller*    prev;
	struct ide_controller*    next;
	struct ide_controller_ops ops;
};

typedef struct ide_device {
	uint8_t reserved;
	uint8_t channel;
	uint8_t drive;
	uint16_t type;
	uint16_t signature;
	uint32_t command_sets;
	uint32_t size;
	char     model[41];
} ide_device_t;

#ifdef __cplusplus
extern "C" {
#endif

void              ide_init();
void              ide_add_controller(ide_controller_t* controller);
void              ide_del_controller(ide_controller_t* controller);
ide_controller_t* ide_enumerate_controllers(ide_controller_t* current);
ide_device_t*     ide_enumerate_devices(ide_device_t* current);

#ifdef __cplusplus
}
#endif

#endif