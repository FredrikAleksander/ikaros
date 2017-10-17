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
#include <kernel/initcall.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/input/keyboard.h>
#include <sys/io.h>
#include <sys/input.h>
#include <sys/spinlock.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SCANCODE_SEQUENCE 16
#define KEYCODE_BUFFER_SIZE 32

typedef struct ps2_keyboard {
	struct ps2_keyboard* next;

	uint8_t    scancode_buffer[MAX_SCANCODE_SEQUENCE];
	uint8_t    scancode_index;
	uint8_t    scancode_set;
} ps2_keyboard_t;

static ps2_keyboard_t* keyboards;

const uint8_t basemap_at2[] = {
	0, // 0x00 ?
	67, // 0x01 F9
	0, // 0x02 ?
	63, // 0x03 F5 
	61, // 0x04 F3
	59, // 0x05 F1
	60, // 0x06 F2
	88, // 0x07 F12
	0, // 0x08 ?
	68, // 0x09 F10
	66, // 0x0A F8
	64, // 0x0B F6
	62, // 0x0C F4
	15, // 0x0D Tab
	41, // 0x0E Backtick
	0, // 0x0F
	0, // 0x10
	56, // 0x11 Left Alt
	42, // 0x12 Left Shift
	0, // 0x13 ?
	29, // 0x14 Left Ctrl
	16, // 0x15 Q
	2, // 0x16 1
	0, // 0x17 ?
	0, // 0x18 ?
	0, // 0x19 ?
	44, // 0x1A Z
	31, // 0x1B S
	30, // 0x1C A
	17, // 0x1D W
	3, // 0x1E 2
	0, // 0x1F ?
	0, // 0x20 ?
	46, // 0x21 C
	45, // 0x22 X
	32, // 0x23 D
	18, // 0x24 E
	5, // 0x25 4
	4, // 0x26 3
	0, // 0x27 ?
	0, // 0x28 ?
	57, // 0x29 Space
	47, // 0x2A V
	33, // 0x2B F
	20, // 0x2C T
	19, // 0x2D R
	6, // 0x2E 5
	0, // 0x2F ?
	0, // 0x30 ?
	49, // 0x31 N
	48, // 0x32 B
	35, // 0x33 H
	34, // 0x34 G
	21, // 0x35 Y
	7, // 0x36 6
	0, // 0x37 ?
	0, // 0x38 ?
	0, // 0x39 ?
	50, // 0x3A M
	36, // 0x3B J
	22, // 0x3C U
	8, // 0x3D 7
	9, // 0x3E 8
	0, // 0x3F ?
	0, // 0x40 ?
	51, // 0x41 Comma
	37, // 0x42 K
	23, // 0x43 I
	24, // 0x44 O
	11, // 0x45 0 (Zero)
	10, // 0x46 9
	0, // 0x47 ?
	0, // 0x48 ?
	52, // 0x49 Period
	53, // 0x4A Forward Slash
	38, // 0x4B L
	39, // 0x4C Semicolon
	25, // 0x4D P
	12, // 0x4E Hyphen
	0, // 0x4F ?
	0, // 0x50 ?
	0, // 0x51 ?
	40, // 0x52 Single Quote
	0, // 0x53 ?
	26, // 0x54 Left Bracket
	13, // 0x55 Equals
	0, // 0x56 ?
	0, // 0x57 ?
	58, // 0x58 Caps Lock
	54, // 0x59 Right Shift
	28, // 0x5A Enter
	27, // 0x5B Right Bracket
	0, // 0x5C 
	43, // 0x5D Backslash
	0, // 0x5E ?
	0, // 0x5F ?
	0, // 0x60 ?
	0, // 0x61 ?
	0, // 0x62 ?
	0, // 0x63 ?
	0, // 0x64 ?
	0, // 0x65 ?
	14, // 0x66 Backspace
	0, // 0x67 ?
	0, // 0x68 ?
	79, // 0x69 Keypad 1
	0, // 0x6A ?
	75, // 0x6B Keypad 4
	71, // 0x6C Keypad 7
	0, // 0x6D ?
	0, // 0x6E ? 
	0, // 0x6F ?
	82, // 0x70 Keypad 0
	83, // 0x71 Keypad .
	80, // 0x72 Keypad 2
	76, // 0x73 Keypad 5
	77, // 0x74 Keypad 6
	72, // 0x75 Keypad 8
	1, // 0x76 Escape
	69, // 0x77 NumLock
	87, // 0x78 F11
	78, // 0x79 Keypad +
	81, // 0x7A Keypad 3
	74, // 0x7B Keypad -
	55, // 0x7C Keypad *
	73, // 0x7D Keypad 9
	70, // 0x7E Scroll Lock
	0, // 0x7F ?
	0, // 0x80 ?
	0, // 0x81 ?
	0, // 0x82 ?
	65, // 0x83 F7 
	0, // 0x84 
	0, // 0x85
	0, // 0x86
	0, // 0x87
	0, // 0x88
	0, // 0x89
	0, // 0x8A
	0, // 0x8B
	0, // 0x8C
	0, // 0x8D
	0, // 0x8E
	0, // 0x8F
	0, // 0x90
	0, // 0x91
	0, // 0x92
	0, // 0x93
	0, // 0x94
	0, // 0x95
	0, // 0x96
	0, // 0x97
	0, // 0x98
	0, // 0x99
	0, // 0x9A
	0, // 0x9B
	0, // 0x9C
	0, // 0x9D
	0, // 0x9E
	0, // 0x9F
	0, // 0xA0
	0, // 0xA1
	0, // 0xA2
	0, // 0xA3
	0, // 0xA4
	0, // 0xA5
	0, // 0xA6
	0, // 0xA7
	0, // 0xA8
	0, // 0xA9
	0, // 0xAA
	0, // 0xAB
	0, // 0xAC
	0, // 0xAD
	0, // 0xAE
	0, // 0xAF
	0, // 0xB0
	0, // 0xB1
	0, // 0xB2
	0, // 0xB3
	0, // 0xB4
	0, // 0xB5
	0, // 0xB6
	0, // 0xB7
	0, // 0xB8
	0, // 0xB9
	0, // 0xBA
	0, // 0xBB
	0, // 0xBC
	0, // 0xBD
	0, // 0xBE
	0, // 0xBF
	0, // 0xC0
	0, // 0xC1
	0, // 0xC2
	0, // 0xC3
	0, // 0xC4
	0, // 0xC5
	0, // 0xC6
	0, // 0xC7
	0, // 0xC8
	0, // 0xC9
	0, // 0xCA
	0, // 0xCB
	0, // 0xCC
	0, // 0xCD
	0, // 0xCE
	0, // 0xCF
	0, // 0xD0
	0, // 0xD1
	0, // 0xD2
	0, // 0xD3
	0, // 0xD4
	0, // 0xD5
	0, // 0xD6
	0, // 0xD7
	0, // 0xD8
	0, // 0xD9
	0, // 0xDA
	0, // 0xDB
	0, // 0xDC
	0, // 0xDD
	0, // 0xDE
	0, // 0xDF
	0, // 0xE0
	0, // 0xE1
	0, // 0xE2
	0, // 0xE3
	0, // 0xE4
	0, // 0xE5
	0, // 0xE6
	0, // 0xE7
	0, // 0xE8
	0, // 0xE9
	0, // 0xEA
	0, // 0xEB
	0, // 0xEC
	0, // 0xED
	0, // 0xEE
	0, // 0xEF
	0, // 0xF0
	0, // 0xF1
	0, // 0xF2
	0, // 0xF3
	0, // 0xF4
	0, // 0xF5
	0, // 0xF6
	0, // 0xF7
	0, // 0xF8
	0, // 0xF9
	0, // 0xFA
	0, // 0xFB
	0, // 0xFC
	0, // 0xFD
	0, // 0xFE
	0  // 0xFF
};
const uint8_t exmap_at2[] = {
	0, // 0x00 
	0, // 0x01 
	0, // 0x02 
	0, // 0x03 
	0, // 0x04 
	0, // 0x05 
	0, // 0x06 
	0, // 0x07 
	0, // 0x08 
	0, // 0x09 
	0, // 0x0A 
	0, // 0x0B 
	0, // 0x0C 
	0, // 0x0D 
	0, // 0x0E 
	0, // 0x0F 
	0, // 0x10 
	0, // 0x11 
	0, // 0x12 
	0, // 0x13 
	0, // 0x14 
	0, // 0x15 
	0, // 0x16 
	0, // 0x17 
	0, // 0x18 
	0, // 0x19 
	0, // 0x1A 
	0, // 0x1B 
	0, // 0x1C 
	0, // 0x1D 
	0, // 0x1E 
	0, // 0x1F 
	0, // 0x20 
	0, // 0x21 
	0, // 0x22 
	0, // 0x23 
	0, // 0x24 
	0, // 0x25 
	0, // 0x26 
	0, // 0x27 
	0, // 0x28 
	0, // 0x29 
	0, // 0x2A 
	0, // 0x2B 
	0, // 0x2C 
	0, // 0x2D 
	0, // 0x2E 
	0, // 0x2F 
	0, // 0x30 
	0, // 0x31 
	0, // 0x32 
	0, // 0x33 
	0, // 0x34 
	0, // 0x35 
	0, // 0x36 
	0, // 0x37 
	0, // 0x38 
	0, // 0x39 
	0, // 0x3A 
	0, // 0x3B 
	0, // 0x3C 
	0, // 0x3D 
	0, // 0x3E 
	0, // 0x3F 
	0, // 0x40 
	0, // 0x41 
	0, // 0x42 
	0, // 0x43 
	0, // 0x44 
	0, // 0x45 
	0, // 0x46 
	0, // 0x47 
	0, // 0x48 
	0, // 0x49 
	0, // 0x4A 
	0, // 0x4B 
	0, // 0x4C 
	0, // 0x4D 
	0, // 0x4E 
	0, // 0x4F 
	0, // 0x50 
	0, // 0x51 
	0, // 0x52 
	0, // 0x53 
	0, // 0x54 
	0, // 0x55 
	0, // 0x56 
	0, // 0x57 
	0, // 0x58 
	0, // 0x59 
	0, // 0x5A 
	0, // 0x5B 
	0, // 0x5C
	0, // 0x5D 
	0, // 0x5E 
	0, // 0x5F 
	0, // 0x60 
	0, // 0x61 
	0, // 0x62 
	0, // 0x63 
	0, // 0x64 
	0, // 0x65 
	0, // 0x66 
	0, // 0x67 
	0, // 0x68 
	0, // 0x69 
	0, // 0x6A 
	0, // 0x6B 
	0, // 0x6C 
	0, // 0x6D 
	0, // 0x6E 
	0, // 0x6F 
	0, // 0x70 
	0, // 0x71 
	0, // 0x72 
	0, // 0x73 
	0, // 0x74 
	0, // 0x75 
	0, // 0x76 
	0, // 0x77 
	0, // 0x78 
	0, // 0x79 
	0, // 0x7A 
	0, // 0x7B 
	0, // 0x7C 
	0, // 0x7D 
	0, // 0x7E
	0, // 0x7F
	0, // 0x80
	0, // 0x81
	0, // 0x82
	0, // 0x83 
	0, // 0x84 
	0, // 0x85
	0, // 0x86
	0, // 0x87
	0, // 0x88
	0, // 0x89
	0, // 0x8A
	0, // 0x8B
	0, // 0x8C
	0, // 0x8D
	0, // 0x8E
	0, // 0x8F
	0, // 0x90
	0, // 0x91
	0, // 0x92
	0, // 0x93
	0, // 0x94
	0, // 0x95
	0, // 0x96
	0, // 0x97
	0, // 0x98
	0, // 0x99
	0, // 0x9A
	0, // 0x9B
	0, // 0x9C
	0, // 0x9D
	0, // 0x9E
	0, // 0x9F
	0, // 0xA0
	0, // 0xA1
	0, // 0xA2
	0, // 0xA3
	0, // 0xA4
	0, // 0xA5
	0, // 0xA6
	0, // 0xA7
	0, // 0xA8
	0, // 0xA9
	0, // 0xAA
	0, // 0xAB
	0, // 0xAC
	0, // 0xAD
	0, // 0xAE
	0, // 0xAF
	0, // 0xB0
	0, // 0xB1
	0, // 0xB2
	0, // 0xB3
	0, // 0xB4
	0, // 0xB5
	0, // 0xB6
	0, // 0xB7
	0, // 0xB8
	0, // 0xB9
	0, // 0xBA
	0, // 0xBB
	0, // 0xBC
	0, // 0xBD
	0, // 0xBE
	0, // 0xBF
	0, // 0xC0
	0, // 0xC1
	0, // 0xC2
	0, // 0xC3
	0, // 0xC4
	0, // 0xC5
	0, // 0xC6
	0, // 0xC7
	0, // 0xC8
	0, // 0xC9
	0, // 0xCA
	0, // 0xCB
	0, // 0xCC
	0, // 0xCD
	0, // 0xCE
	0, // 0xCF
	0, // 0xD0
	0, // 0xD1
	0, // 0xD2
	0, // 0xD3
	0, // 0xD4
	0, // 0xD5
	0, // 0xD6
	0, // 0xD7
	0, // 0xD8
	0, // 0xD9
	0, // 0xDA
	0, // 0xDB
	0, // 0xDC
	0, // 0xDD
	0, // 0xDE
	0, // 0xDF
	0, // 0xE0
	0, // 0xE1
	0, // 0xE2
	0, // 0xE3
	0, // 0xE4
	0, // 0xE5
	0, // 0xE6
	0, // 0xE7
	0, // 0xE8
	0, // 0xE9
	0, // 0xEA
	0, // 0xEB
	0, // 0xEC
	0, // 0xED
	0, // 0xEE
	0, // 0xEF
	0, // 0xF0
	0, // 0xF1
	0, // 0xF2
	0, // 0xF3
	0, // 0xF4
	0, // 0xF5
	0, // 0xF6
	0, // 0xF7
	0, // 0xF8
	0, // 0xF9
	0, // 0xFA
	0, // 0xFB
	0, // 0xFC
	0, // 0xFD
	0, // 0xFE
	0  // 0xFF
};

const keycode_t exmap_at1[256] = {
};

static inline keycode_t kb_map_scancode_at2(uint8_t scancode[]) {
	if(scancode[0] == 0xE1) {
	}
	else if(scancode[0] == 0xE0) {
		if(scancode[1] == 0xF0) {
			// Released
			return ((keycode_t)exmap_at2[scancode[2]]) | (1 << 8);
		}
		else {
			// Pressed
			return exmap_at2[scancode[1]];
		}
	}
	else if(scancode[0] == 0xF0) {
		keycode_t k = basemap_at2[scancode[1]] | (1 << 8);
		return k;
	}
	else {
		return ((keycode_t)basemap_at2[scancode[0]]); // Pressed
	}

	return 0;
}
static inline int kb_scancode_sequence_complete_at2(uint8_t scancode[], uint8_t scancode_len) {
	if(scancode[0] == 0xE1) {
		return scancode_len == 8 
			? 1 
			: 0;
	}
	else if(scancode[0] == 0xE0) {
		// Multibyte sequence
		if(scancode_len == 1) {
			return 0;
		}
		if(scancode[1] == 0xF0) {
			return scancode_len == 3 
				? 1 
				: 0;
		}
		return scancode_len == 2
			? 1
			: 0;
	}
	else if(scancode[0] == 0xF0) {
		return scancode_len == 2
			? 1
			: 0;
	}
	else if(scancode_len == 1) {
		return 1;
	}

	return 0;
}

static inline keycode_t kb_map_scancode_at1(uint8_t scancode[]) {
	if(scancode[0] == 0xE1) {
		// Pause
		return 119;
	}
	if(scancode[0] == 0xE0) {
		if(scancode[1] == 0x2A) {
			return 99;
		}
		if(scancode[1] == 0xB7) {
			return 99 | (1 << 8);
		}
		return exmap_at1[scancode[1]];
	}
	if(scancode[0] > 128) {
		return (scancode[0] - 128) | (1 << 8);
	}

	return scancode[0];
}

static inline int kb_scancode_sequence_complete_at1(uint8_t scancode[], uint8_t scancode_len) {
	if(scancode[0] == 0xE1) {
		return scancode_len == 6 ? 1 : 0;
	}
	if(scancode[0] == 0xE0) {
		if(scancode_len > 1) {
			if(scancode[1] == 0x2A || scancode[1] == 0xB7) {
				return scancode_len == 4 ? 1 : 0;
			}
			return scancode_len == 2 ? 1 : 0;
		}
	}
	return scancode_len == 1 ? 1 : 0;
}

static inline void kb_add_scancode_seq(ps2_keyboard_t* kb, uint8_t scancode) {
	keycode_t kc;
	kb->scancode_buffer[kb->scancode_index] = scancode;
	kb->scancode_index++;
	if(kb->scancode_set == 0) {
		if(kb_scancode_sequence_complete_at1(kb->scancode_buffer, kb->scancode_index)) {
			kc = kb_map_scancode_at1(kb->scancode_buffer);
			__kb_emit_keycode(kc);
			kb->scancode_index = 0;
		}
	}
	else if(kb->scancode_set == 1) {
		if(kb_scancode_sequence_complete_at2(kb->scancode_buffer, kb->scancode_index)) {
			kc = kb_map_scancode_at2(kb->scancode_buffer);
			__kb_emit_keycode(kc);
			kb->scancode_index = 0;
		}
	}
	
}

extern void* keyboard_handler(void* ctx);
void* keyboard_handler(void __attribute__ ((unused))* ctx) {
	ps2_keyboard_t* keyboard = keyboards;
	while((inb_p(0x64) & 1) == 1) {
		kb_add_scancode_seq(keyboard, inb_p(0x60));
	}
	return NULL;
}

static void init_ps2_keyboard(ps2_keyboard_t* kb) {
	// TODO: Do this properly, this is such a hack
	memset(kb, 0, sizeof(ps2_keyboard_t));
	// uint8_t config = 0;

	// while((inb_p(0x64) & 1) == 1) {
	// 	inb_p(0x60);
	// }
	// outb_p(0x64, 0x20);
	// while((inb_p(0x64) & 1) == 0);
	// config = inb_p(0x60);
	// if(((config >> 6) & 1) == 1)
	// {
	// 	config = config & ~(1<<6);
	// 	outb_p(0x64, 0x60);
	// 	outb_p(0x60, config);
	// }
}

static int init_at_keyboard(void) {
	keyboards = malloc(sizeof(ps2_keyboard_t));
	init_ps2_keyboard(keyboards);
	return INIT_OK;
}

arch_initcall(init_at_keyboard);

