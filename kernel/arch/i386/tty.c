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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/initcall.h>


static int test_initcall() {
	printf("Initcall!\n");
	return 0;
}

early_initcall(test_initcall);

#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xC00B8000;

static size_t        terminal_row;
static size_t        terminal_column;
static uint8_t       terminal_color;
static uint16_t*     terminal_buffer;
static unsigned char escape_code_sequence[32];
static unsigned char escape_code_parameter[32];
static int           escape_code_length;

void terminal_initialize(void) {
	escape_code_length = 0;
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

static inline void _terminal_scroll(int x) {
	const size_t row_dst_index = VGA_WIDTH * x;
	const size_t row_src_index = VGA_WIDTH * (x+1);
	unsigned int i;

	for(i = 0; i < VGA_WIDTH; i++) {
		terminal_buffer[row_dst_index + i] = terminal_buffer[row_src_index + i];
	}
}

static inline int _terminal_parameter_terminator(int pos) {
	for(; pos < escape_code_length; pos++) {
		if(escape_code_sequence[pos] == ';' || escape_code_sequence[pos] == 'm') {
			return pos;
		}
	}
	return -1;
}

enum vga_color _terminal_ansi_to_vga(uint8_t ansi) {
	switch(ansi) {
		case 0: /* Black */
			return VGA_COLOR_BLACK;
		case 1: /* Red */
			return VGA_COLOR_RED;
		case 2: /* Green */
			return VGA_COLOR_GREEN;
		case 3: /* Yellow */
			return VGA_COLOR_BROWN;
		case 4: /* Blue */
			return VGA_COLOR_BLUE;
		case 5: /* Magenta */
			return VGA_COLOR_MAGENTA;
		case 6: /* Cyan */
			return VGA_COLOR_CYAN;
		case 7: /* White */
			return VGA_COLOR_LIGHT_GREY;
		default:
			return 0xD;

	}
}

static inline void _terminal_set_foreground_color_ansi(uint8_t color) {
	enum vga_color bg = bg_from_vga_color(terminal_color);
	uint8_t bright_bit = fg_from_vga_color(color) & 0x08;
	uint8_t clr = _terminal_ansi_to_vga(color);
	if(bright_bit != 0)
		clr |= 8;
	terminal_color = vga_entry_color(clr, bg);
}

static inline void _terminal_set_foreground_color(uint8_t color) {
	enum vga_color bg = bg_from_vga_color(terminal_color);
	terminal_color = vga_entry_color(color, bg);
}

static inline void _terminal_set_background_color_ansi(uint8_t color) {
	enum vga_color fg = fg_from_vga_color(terminal_color);
	terminal_color = vga_entry_color(fg, _terminal_ansi_to_vga(color));
}

static inline void _terminal_set_background_color(uint8_t color) {
	enum vga_color fg = fg_from_vga_color(terminal_color);
	terminal_color = vga_entry_color(fg, color);
}

static inline int _terminal_sgr_consume_parameter(int pos) {
	int end;
	int value;

	if(pos >= escape_code_length)
		return -1;

	end = _terminal_parameter_terminator(pos);
	if(end == -1)
		return -1;
	memset(escape_code_parameter, 0, 32);
	memcpy(escape_code_parameter, &escape_code_sequence[pos], end - pos);
	value = atoi((const char*)escape_code_parameter);
	if(value >= 30 && value <= 37) {
		/* Set foreground color */
		_terminal_set_foreground_color_ansi((uint8_t)(value - 30));
	}
	else if(value >= 40 && value <= 47) {
		/* Set background color */
		_terminal_set_background_color_ansi((uint8_t)(value - 40));
	}
	else if(value == 1) {
		/* Bright */
		_terminal_set_foreground_color(fg_from_vga_color(terminal_color) | 0x08);
	}
	else if(value == 22) {
		/* Normal */
		_terminal_set_foreground_color(fg_from_vga_color(terminal_color) & 0x07);
	}
	return end;
}

static inline void _terminal_clear_to_end_of_line() {
	if(terminal_row == VGA_HEIGHT)
		return;
	
	size_t index = terminal_row * VGA_WIDTH;
	unsigned int i;
	
	for(i = terminal_column; i < VGA_WIDTH; i++) {
		terminal_buffer[index + i] = vga_entry(' ', terminal_color);
	}
	terminal_row++;
	terminal_column = 0;
}

static inline void _terminal_sgr() {
	int pos = 1;
	while(pos > 0) {
		pos = _terminal_sgr_consume_parameter(pos+1);
	}
}

void terminal_putchar(char c) {
	unsigned char uc = c;
	unsigned int i;
	size_t index;

	if(c == 0x1B) {
		memset(escape_code_sequence, 0, 32);
		escape_code_sequence[0] = 0x1B;
		escape_code_length = 1;
		return;
	}

	if(escape_code_length > 0) {
		if(escape_code_length == 1) {
			if(c == '[') {
				escape_code_sequence[escape_code_length] = c;
				escape_code_length++;
			} 
			else {
				escape_code_length = 0;
				return;
			}
		}
		else if(c >= 64 && c <= 126) {
			escape_code_sequence[escape_code_length] = c;
			escape_code_length++;
			switch(c) {
				/*case 'A':
					break;
				case 'B':
					break;
				case 'C':
					break;
				case 'D':
					break;
				case 'E':
					break;
				case 'F':
					break;
				case 'G':
					break;
				case 'H':
					break;
				case 'J':
					break;
				
				case 'S':
					break;
				case 'T':
					break;
				case 's':
					break;
				case 'u':
					break;*/
				case 'K': /* Clear to end of line */
					_terminal_clear_to_end_of_line();
					break;
				case 'm': /* SGR - Select Graphic Rendition. Set text attributes (incl color) */
					_terminal_sgr();
					break;
				default:
					break;
			}
			escape_code_length = 0;
			//printf("fdfdsf");
		}
		else
		{
			escape_code_sequence[escape_code_length] = c;
			escape_code_length++;
		}
	}
	else {
		if(terminal_row == VGA_HEIGHT) {
			for(i = 0; i < VGA_HEIGHT - 1; i++) {
				_terminal_scroll(i);
			}
			for(i = 0; i < VGA_WIDTH; i++) {
				index = (VGA_HEIGHT - 1) * VGA_WIDTH + i;
				terminal_buffer[index] = vga_entry(' ', terminal_color);
			}
			terminal_column = 0;
			terminal_row--;
		}

		if(c == '\n') {
			terminal_column = 0;
			terminal_row++;
		} else {
			terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
			if (++terminal_column == VGA_WIDTH) {
				terminal_column = 0;
				terminal_row++;
			}
		}
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}
