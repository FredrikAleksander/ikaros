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
#include <kernel/drivers/console/vga_console.h>
#include <kernel/console/consolefont.h>
#include <kernel/console/console.h>
#include <kernel/scheduler/scheduler.h>
#include <sys/spinlock.h>
#include <sys/io.h>
#include <stdlib.h>
#include <string.h>

static uint16_t cursor_column;
static uint16_t cursor_row;
static int      cursor_visible;

static spinlock_t vga_lock;

static inline void vga_enable_cursor(void)
{
	cursor_visible = 1;
	outb_p(0x3D4, 0x0A); // LOW cursor shape port to vga INDEX register
	outb_p(0x3D5, 0x1F); //bits 6-7 must be 0 , if bit 5 set the cursor is disable  , bits 0-4 controll the cursor shape .
}

static inline void vga_disable_cursor(void)
{
   cursor_visible = 0;
   outb_p(0x3D4, 0x0A); // LOW cursor shape port to vga INDEX register
   outb_p(0x3D5, 0x3F); //bits 6-7 must be 0 , if bit 5 set the cursor is disable  , bits 0-4 controll the cursor shape .
}

static void vga_set_cursor(uint16_t x, uint16_t y) {
	cursor_column = x;
	cursor_row = y;

	uint16_t pos = y * 80 + x;

	outb_p(0x3D4, 0x0F);
	outb_p(0x3D5, (uint8_t)(pos & 0xFF));
	outb_p(0x3D4, 0x0E);
	outb_p(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void vga_console_redraw(struct console* cons) {
	int show_cursor = cons->flags & CONSOLE_FLAG_SHOW_CURSOR ? 1 : 0;
	if(cons->column != cursor_column || cons->row != cursor_row) {
		vga_set_cursor(cons->column, cons->row);	
	}
	
	if(show_cursor != cursor_visible) {
		if(show_cursor) {
			vga_enable_cursor();
		}
		else {
			vga_disable_cursor();
		}
	}
	
}

const struct console_operations vga_console_ops = {
	.redraw = vga_console_redraw
};

void __init_vga_console(void* vga_memory, int columns, int rows) {
	vga_lock = SPINLOCK_UNLOCKED;
	console_t* cons = malloc(sizeof(console_t));
	cursor_column   = 0;
	cursor_row      = 0;
	cursor_visible  = 1;
	memset(cons, 0, sizeof(console_t));
	cons->columns = columns;
	cons->rows = rows;
	cons->buffer = (console_char_t*)vga_memory;
	cons->flags = CONSOLE_FLAG_AUTODRAW | CONSOLE_FLAG_SHOW_CURSOR;
	cons->ops = &vga_console_ops;
	__console_initialize(cons);
}
