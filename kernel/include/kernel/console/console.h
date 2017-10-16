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
#ifndef __KERNEL_CONSOLE__CONSOLE_H
#define __KERNEL_CONSOLE__CONSOLE_H 1

#include <sys/unicode.h>
#include <stdint.h>
#include <stddef.h>

#define CONSOLE_FLAG_DIRTY    (1 << 0)
#define CONSOLE_FLAG_AUTODRAW (1 << 1)
#define CONSOLE_FLAG_SHOW_CURSOR (1 << 2)

enum console_color {
	CONS_COLOR_BLACK = 0,
	CONS_COLOR_BLUE = 1,
	CONS_COLOR_GREEN = 2,
	CONS_COLOR_CYAN = 3,
	CONS_COLOR_RED = 4,
	CONS_COLOR_MAGENTA = 5,
	CONS_COLOR_BROWN = 6,
	CONS_COLOR_LIGHT_GREY = 7,
	CONS_COLOR_DARK_GREY = 8,
	CONS_COLOR_LIGHT_BLUE = 9,
	CONS_COLOR_LIGHT_GREEN = 10,
	CONS_COLOR_LIGHT_CYAN = 11,
	CONS_COLOR_LIGHT_RED = 12,
	CONS_COLOR_LIGHT_MAGENTA = 13,
	CONS_COLOR_LIGHT_BROWN = 14,
	CONS_COLOR_WHITE = 15,
};

struct console;
struct console_char {
	uint8_t ch;
	uint8_t attrs;
} __attribute__((packed));

struct console_operations {
	void (*redraw)(struct console*);
};

struct console {
	uint16_t                   columns;
	uint16_t                   rows;
	uint16_t                   column;
	uint16_t                   row;
	uint8_t                    attrs;
	uint32_t                   flags;
	uint8_t                    escape_code_sequence[32];
	uint8_t                    escape_code_parameter[32];
	uint8_t                    escape_code_length;
	struct console_char*       buffer;
	struct console_operations* ops;
	void*                      private;
};

typedef struct console_char console_char_t;
typedef struct console console_t;

#ifdef __cplusplus
extern "C" {
#endif

extern int  console_getch(void);
extern void console_putch(char ch);
extern void console_write(const char* buf, size_t buf_len);
extern void console_writestr(const char* str);
extern void console_debug(void);

// Called by boot code. 
extern void __console_initialize(console_t* cons);
extern void __console_redraw(void);

extern void __init_null_console(void);

#ifdef __cplusplus
}
#endif

#endif
