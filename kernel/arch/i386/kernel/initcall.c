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
#include <kernel/boot/multiboot2.h>
#include <string.h>
#include <stdio.h>
#include <sys/elf32.h>

extern uintptr_t __initcall0_ptrs_start;
extern uintptr_t __initcall0_ptrs_end;

void __invoke_initcall0(void) { 
	initcall_t* initcall_start = (initcall_t*)&__initcall0_ptrs_start; 
	initcall_t* initcall_end = (initcall_t*)&__initcall0_ptrs_end; 
	while(initcall_start != initcall_end) { 
		(*initcall_start)(); 
		initcall_start++; 
	} 
}

extern uintptr_t __initcall1_ptrs_start;
extern uintptr_t __initcall1_ptrs_end;

void __invoke_initcall1(void) { 
	initcall_t* initcall_start = (initcall_t*)&__initcall1_ptrs_start; 
	initcall_t* initcall_end = (initcall_t*)&__initcall1_ptrs_end; 
	while(initcall_start != initcall_end) { 
		(*initcall_start)(); 
		initcall_start++; 
	} 
}

extern uintptr_t __initcall2_ptrs_start;
extern uintptr_t __initcall2_ptrs_end;

void __invoke_initcall2(void) { 
	initcall_t* initcall_start = (initcall_t*)&__initcall2_ptrs_start; 
	initcall_t* initcall_end = (initcall_t*)&__initcall2_ptrs_end; 
	while(initcall_start != initcall_end) { 
		(*initcall_start)(); 
		initcall_start++; 
	} 
}

extern uintptr_t __initcall3_ptrs_start;
extern uintptr_t __initcall3_ptrs_end;

void __invoke_initcall3(void) { 
	initcall_t* initcall_start = (initcall_t*)&__initcall3_ptrs_start; 
	initcall_t* initcall_end = (initcall_t*)&__initcall3_ptrs_end; 
	while(initcall_start != initcall_end) { 
		(*initcall_start)(); 
		initcall_start++; 
	} 
}

extern uintptr_t __initcall4_ptrs_start;
extern uintptr_t __initcall4_ptrs_end;

void __invoke_initcall4(void) { 
	initcall_t* initcall_start = (initcall_t*)&__initcall4_ptrs_start; 
	initcall_t* initcall_end = (initcall_t*)&__initcall4_ptrs_end; 
	while(initcall_start != initcall_end) { 
		(*initcall_start)(); 
		initcall_start++; 
	} 
}
