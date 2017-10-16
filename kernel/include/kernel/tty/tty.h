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
#ifndef __KERNEL_TTY__TTY_H
#define __KERNEL_TTY__TTY_H 1

#include <kernel/fs/fs.h>
#include <stdint.h>

struct tty_operations;
struct tty_driver;
struct tty_line_discipline;
struct tty_device;

typedef struct tty_operations      tty_operations_t;
typedef struct tty_driver          tty_driver_t;
typedef struct tty_line_discipline tty_line_discipline_t;
typedef struct tty_device          tty_device_t;

// Low Level driver
struct tty_driver {
	const char* driver_name;
	const char* name;

	int major;
	int minor;
	int minor_num;

	tty_device_t** devices;

	void* driver_state;
	const tty_operations_t* ops;
};

struct tty_operations {
	void (*initialize)(tty_driver_t* driver, tty_device_t* device);
	void (*cleanup)(tty_driver_t* driver, tty_device_t* device);

	int  (*open)(tty_device_t* tty, file_t* fp);
	void (*close)(tty_device_t* tty, file_t* fp);

	void (*write)(tty_device_t*, const uint8_t* buffer, int count);
	void (*putch)(tty_device_t*, uint8_t ch);
};

#endif
