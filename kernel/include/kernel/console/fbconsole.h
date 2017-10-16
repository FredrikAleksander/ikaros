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
#ifndef __KERNEL_CONSOLE__FBCONSOLE_H
#define __KERNEL_CONSOLE__FBCONSOLE_H 1

#include <kernel/console/console.h>

#define FB_CONSOLE_FLAG_RGB 1

struct console_fb_palette {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} __attribute__((packed));

struct console_fb_color_fmt {
	uint8_t red_mask_size;
	uint8_t red_position;
	uint8_t green_mask_size;
	uint8_t green_position;
	uint8_t blue_mask_size;
	uint8_t blue_position;
} __attribute__((packed));

typedef struct console_framebuffer {
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint8_t  bpp;
	uint8_t  flags;
	uint8_t* data;
	void*    private;
	uint8_t  palette_num;
	struct console_fb_color_fmt format;
	struct console_fb_palette   palette[0];
} console_framebuffer_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void __init_fb_console(console_framebuffer_t* framebuffer);

#ifdef __cplusplus
}
#endif

#endif
