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
#include <kernel/console/console.h>
#include <kernel/input/keyboard.h>
#ifdef CONFIG_DEBUG_SERIAL
#include <kernel/debug/serial_debug.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static console_t* console;

static inline uint8_t console_entry_color(enum console_color fg, enum console_color bg) {
	return fg | bg << 4;
}

static inline enum console_color bg_from_console_color(uint8_t color) {
	return (color & 0xf0) >> 4;
}

static inline enum console_color fg_from_console_color(uint8_t color) {
	return (color & 0x0f);
}

static inline uint16_t console_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

void __console_redraw(void) {
	if(console->flags & CONSOLE_FLAG_DIRTY) {
		console->ops->redraw(console);
		console->flags &= ~CONSOLE_FLAG_DIRTY;
	}
}

void __console_initialize(console_t* cons) {
	uint16_t i, j;

	console = cons;
	cons->row    = 0;
	cons->column = 0;
	cons->attrs  = 0x07;

	for(i = 0; i < console->rows; i++) {
		for(j = 0; j < console->columns; j++) {
			cons->buffer[i * console->columns + j].ch    = ' ';
			cons->buffer[i * console->columns + j].attrs = cons->attrs;
		}
	}

	cons->flags |= CONSOLE_FLAG_DIRTY;
	if(cons->flags & CONSOLE_FLAG_AUTODRAW) {
		__console_redraw();
	}
}

static inline int console_parameter_terminator(int pos) {
	for(; pos < console->escape_code_length; pos++) {
		if(console->escape_code_sequence[pos] == ';' || console->escape_code_sequence[pos] == 'm') {
			return pos;
		}
	}
	return -1;
}

static enum console_color console_ansi_to_vga(uint8_t ansi) {
	switch(ansi) {
		case 0: /* Black */
			return CONS_COLOR_BLACK;
		case 1: /* Red */
			return CONS_COLOR_RED;
		case 2: /* Green */
			return CONS_COLOR_GREEN;
		case 3: /* Yellow */
			return CONS_COLOR_BROWN;
		case 4: /* Blue */
			return CONS_COLOR_BLUE;
		case 5: /* Magenta */
			return CONS_COLOR_MAGENTA;
		case 6: /* Cyan */
			return CONS_COLOR_CYAN;
		case 7: /* White */
			return CONS_COLOR_LIGHT_GREY;
		default:
			return 0xD;

	}
}

static inline void console_set_foreground_color_ansi(uint8_t color) {
	enum console_color bg = bg_from_console_color(console->attrs);
	uint8_t bright_bit = fg_from_console_color(color) & 0x08;
	uint8_t clr = console_ansi_to_vga(color);
	if(bright_bit != 0)
		clr |= 8;
	console->attrs = console_entry_color(clr, bg);
}

static inline void console_set_foreground_color(uint8_t color) {
	enum console_color bg = bg_from_console_color(console->attrs);
	console->attrs = console_entry_color(color, bg);
}

static inline void console_set_background_color_ansi(uint8_t color) {
	enum console_color fg = fg_from_console_color(console->attrs);
	console->attrs = console_entry_color(fg, console_ansi_to_vga(color));
}

static inline void console_set_background_color(uint8_t color) {
	enum console_color fg = fg_from_console_color(console->attrs);
	console->attrs = console_entry_color(fg, color);
}

static inline int console_sgr_consume_parameter(int pos) {
	int end;
	int value;

	if(pos >= console->escape_code_length)
		return -1;

	end = console_parameter_terminator(pos);
	if(end == -1)
		return -1;
	memset(console->escape_code_parameter, 0, 32);
	memcpy(console->escape_code_parameter, &console->escape_code_sequence[pos], end - pos);
	value = atoi((const char*)console->escape_code_parameter);
	if(value >= 30 && value <= 37) {
		/* Set foreground color */
		console_set_foreground_color_ansi((uint8_t)(value - 30));
	}
	else if(value >= 40 && value <= 47) {
		/* Set background color */
		console_set_background_color_ansi((uint8_t)(value - 40));
	}
	else if(value == 1) {
		/* Bright */
		console_set_foreground_color(fg_from_console_color(console->attrs) | 0x08);
	}
	else if(value == 22) {
		/* Normal */
		console_set_foreground_color(fg_from_console_color(console->attrs) & 0x07);
	}
	return end;
}


static inline void console_clear_to_end_of_line(void) {
	uint16_t i;
	uint16_t index = console->row * console->columns;
	
	if(console->row == console->rows)
		return;
	
	for(i = console->column; i < console->columns; i++) {
		console->buffer[index + i].ch = ' ';
		console->buffer[index + i].attrs = console->attrs;
	}
	console->row++;
	console->column = 0;
}

static inline void console_sgr(void) {
	int pos = 1;
	while(pos > 0) {
		pos = console_sgr_consume_parameter(pos+1);
	}
}

static void __console_putch(char c) {
	unsigned char uc = c;
	uint16_t i;
	uint16_t j;
	uint16_t index;

// #ifdef CONFIG_DEBUG_SERIAL
// 	serial_debug_putch((char)uc);
// #endif

	if(c == 0x1B) {
		memset(console->escape_code_sequence, 0, 32);
		console->escape_code_sequence[0] = 0x1B;
		console->escape_code_length = 1;
		return;
	}

	if(console->escape_code_length > 0) {
		if(console->escape_code_length == 1) {
			if(c == '[') {
				console->escape_code_sequence[console->escape_code_length] = c;
				console->escape_code_length++;
			} 
			else {
				console->escape_code_length = 0;
				return;
			}
		}
		else if(c >= 64 && c <= 126) {
			console->escape_code_sequence[console->escape_code_length] = c;
			console->escape_code_length++;
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
					console_clear_to_end_of_line();
#ifdef CONFIG_DEBUG_SERIAL
					serial_debug_putch('\n');
#endif
					break;
				case 'm': /* SGR - Select Graphic Rendition. Set text attributes (incl color) */
					console_sgr();
					break;
				default:
					break;
			}
			console->escape_code_length = 0;
		}
		else
		{
			console->escape_code_sequence[console->escape_code_length] = c;
			console->escape_code_length++;
		}
	}
	else {
		if(console->row == console->rows) {
			for(i = 1; i < console->rows; i++) {				
				for(j = 0; j < console->columns; j++) {
					console->buffer[((i-1) * console->columns) + j] =
						console->buffer[(i * console->columns) + j];
				}
			}
			for(i = 0; i < console->columns; i++) {
				index = (console->rows - 1) * console->columns + i;
				console->buffer[index].ch = ' ';
				console->buffer[index].attrs = console->attrs;
			}
			console->column = 0;
			console->row--;
		}

		if(c == '\n') {
			console->column = 0;
			console->row++;

#ifdef CONFIG_DEBUG_SERIAL
			serial_debug_putch('\n');
#endif
		} else {
#ifdef CONFIG_DEBUG_SERIAL
			serial_debug_putch((char)uc);
#endif
			console->buffer[console->row * console->columns + console->column].ch = uc;
			console->buffer[console->row * console->columns + console->column].attrs = console->attrs;
			
			if (++console->column == console->columns) {
				console->column = 0;
				console->row++;
			}
		}
	}
}

void console_putch(char ch) {
	__console_putch(ch);
	console->flags |= CONSOLE_FLAG_DIRTY;
	if(console->flags & CONSOLE_FLAG_AUTODRAW) {
		__console_redraw();
	}
}

void console_write(const char* buf, size_t buf_len) {
	size_t i;
	for(i = 0; i < buf_len; i++) {
		__console_putch(buf[i]);
	}
	console->flags |= CONSOLE_FLAG_DIRTY;
	if(console->flags & CONSOLE_FLAG_AUTODRAW) {
		__console_redraw();
	}
}

void console_writestr(const char* str) {
	int len = strlen(str);
	console_write(str, len);
}

int console_getch(void) {
	return kb_getch();
}

void console_debug(void) {
	console->ops->redraw(console);
}
