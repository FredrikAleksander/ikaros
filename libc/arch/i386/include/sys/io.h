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
#ifndef __ARCH_I386_SYS__IO_H
#define __ARCH_I386_SYS__IO_H 1

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
	asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outb_p(uint16_t port, uint8_t val) {
	asm volatile("outb %0, %1\noutb %%al, $0x80" : : "a"(val), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t val) {
	asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw_p(uint16_t port, uint16_t val) {
	asm volatile("outw %0, %1\noutb %%al, $0x80" : : "a"(val), "Nd"(port));
}

static inline void outl(uint16_t port, uint32_t val) {
	asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outl_p(uint16_t port, uint32_t val) {
	asm volatile("outl %0, %1\noutb %%al, $0x80" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline uint8_t inb_p(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %1, %0\noutb %%al, $0x80" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline uint16_t inw_p(uint16_t port) {
	uint16_t ret;
	asm volatile("inw %1, %0\noutb %%al, $0x80" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline uint32_t inl(uint16_t port) {
	uint32_t ret;
	asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline uint32_t inl_p(uint16_t port) {
	uint32_t ret;
	asm volatile("inl %1, %0\noutb %%al, $0x80" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void insb(uint16_t port, void* target, uintptr_t len) {
	asm volatile("cld; rep; insb" : "=D"(target), "=c"(len) : "d"(port), "0"(target), "1"(len));
}

static inline void insw(uint16_t port, void* target, uintptr_t len) {
	asm volatile("cld; rep; insw" : "=D"(target), "=c"(len) : "d"(port), "0"(target), "1"(len));
}

static inline void insl(uint16_t port, void* target, uintptr_t len) {
	asm volatile("cld; rep; insl" : "=D"(target), "=c"(len) : "d"(port), "0"(target), "1"(len));
}

static inline void outsb(uint16_t port, const void* target, uintptr_t len) {
	asm volatile("cld; rep; outsb" : "=S"(target), "=c"(len) : "d"(port), "0"(target), "1"(len));
}

static inline void outsw(uint16_t port, const void* target, uintptr_t len) {
	asm volatile("cld; rep; outsw" : "=S"(target), "=c"(len) : "d"(port), "0"(target), "1"(len));
}

static inline void outsl(uint16_t port, const void* target, uintptr_t len) {
	asm volatile("cld; rep; outsl" : "=S"(target), "=c"(len) : "d"(port), "0"(target), "1"(len));
}
 
static inline void io_wait() {
	asm volatile ("outb %%al, $0x80" : : "a"(0));
}

#endif
