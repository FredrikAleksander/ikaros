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
#include <kernel/input/keyboard.h>
#include <kernel/scheduler/wait.h>
#include <kernel/scheduler/scheduler.h>
#include <stdio.h>
#include <kernel/initcall.h>
#include <sys/spinlock.h>

#define KEY_RING_SIZE 128
#define KEY_ESCAPE 13

static spinlock_t        key_state_lock;
static uint16_t          key_ring_index;
static keycode_t         key_ring[KEY_RING_SIZE];
static uint8_t           key_modifiers;
static wait_queue_head_t key_wait_queue;

const char plain_map[256] = {
	0, 27, '1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', '-', '=', 8, 9,
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
	'o','p','[',']', 13, 0, 'a', 's', 'd', 'f',
	'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
	'.', '/', 0, '*', 0, ' '
};

const char shift_map[256] = {
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*',
	'(', ')', '_', '+', 0, 0, 'Q', 'W', 'E', 'R', 'T',
	'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0, 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,
	'|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
	0, '*'
};

const char altgr_map[256] = {
	0, 0, 0, '@', 0, '$', 0, 0,
	'{', '[', ']', '}'
};

static char _kb_map(keycode_t code, uint8_t mods) {
	char x = 0;;

	if(mods & KEY_MODIFIER_SHIFT)
		x = shift_map[code & 0xFF];
	else if(mods & (KEY_MODIFIER_ALT_GR | KEY_MODIFIER_ALT))
		x =  altgr_map[code & 0xFF];
	else 
		x = plain_map[code & 0xFF];

	if(mods & KEY_MODIFIER_CAPS)
	{
		if(x >= 'a' && x <= 'z') {
			return x - 32;
		}
		else if(x >= 'A' && x <= 'Z') {
			return x + 32;
		}
	}

	return x;
}

static uint8_t _key_ring_idx(void) {
	unsigned long flags;
	uint8_t idx;
	spinlock_acquire_irqsave(&key_state_lock, &flags);
	idx = key_ring_index;
	spinlock_release_irqload(&key_state_lock, &flags);
	return idx;
}

uint8_t kb_getmods(void) {
	unsigned long flags;
	uint8_t idx;
	spinlock_acquire_irqsave(&key_state_lock, &flags);
	idx = key_modifiers;
	spinlock_release_irqload(&key_state_lock, &flags);
	return idx;
}

keycode_t kb_getkey(void) {
	uint8_t tmp = _key_ring_idx();
	uint8_t cmp = 0;
	WAIT_EVENT_INTERRUPTIBLE(key_wait_queue, (cmp = _key_ring_idx()) != tmp);
	return key_ring[cmp];
}

char kb_getch(void) {
	char ch;
	keycode_t key;
	uint16_t  state;

	while(1) {
		key   = kb_getkey();
		state = (key & 0xFF00) >> 8;
		key = key & 0xFF;
		if(state == 0) {
			ch = _kb_map(key, kb_getmods());
			if(ch != 0)
				return ch;
		}
	}
	return '\0';
}

void __kb_emit_keycode(keycode_t keycode) {
	uint16_t released = keycode >> 8;
	uint16_t key = keycode & 0xFF;

	unsigned long flags;
	spinlock_acquire_irqsave(&key_state_lock, &flags);

	if(key == 58) {
		if(released) {
			if(key_modifiers & KEY_MODIFIER_CAPS) {
				key_modifiers &= ~KEY_MODIFIER_CAPS;
			}
			else {
				key_modifiers |= KEY_MODIFIER_CAPS;
			}
		}
	}

	if(key == 29 || key == 97) {
		// Ctrl
		if(released) {
			key_modifiers &= ~KEY_MODIFIER_CTRL;
		}
		else {
			key_modifiers |= KEY_MODIFIER_CTRL;
		}
	}
	else if(key == 42 || key == 54) {
		// Shift
		if(released) {
			key_modifiers &= ~KEY_MODIFIER_SHIFT;
		}
		else {
			key_modifiers |= KEY_MODIFIER_SHIFT;
		}
	}
	else if(key == 56) {
		// Alt
		if(released) {
			key_modifiers &= ~KEY_MODIFIER_ALT;
		}
		else {
			key_modifiers |= KEY_MODIFIER_ALT;
		}
	}
	else if(key == 100) {
		// Alt Gr
		if(released) {
			key_modifiers &= ~KEY_MODIFIER_ALT_GR;
		}
		else {
			key_modifiers |= KEY_MODIFIER_ALT_GR;
		}
	}
	key_ring_index = (key_ring_index + 1) % 128;
	key_ring[key_ring_index] = keycode;
	spinlock_release_irqload(&key_state_lock, &flags);
	wait_wake_up_interruptible(&key_wait_queue);
}

static int kb_early_init(void) {
	key_state_lock = SPINLOCK_UNLOCKED;
	key_ring_index = 0;
	key_modifiers  = 0;
	wait_queue_init(&key_wait_queue);
	return INIT_OK;
}

early_initcall(kb_early_init);
