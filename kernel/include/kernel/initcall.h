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
#ifndef __KERNEL__INITCALL_H
#define __KERNEL__INITCALL_H 1

#define INIT_OK   0
#define INIT_ERR -1

typedef int (*initcall_t)();

#define __define_initcall(fn, id) \
	static initcall_t __initcall_##fn##id __attribute__ ((used)) __attribute__ ((section(".initcall" #id ".init"))) = fn;

#define early_initcall(fn)   __define_initcall(fn, 0)
#define arch_initcall(fn)    __define_initcall(fn, 1)
#define bus_initcall(fn)     __define_initcall(fn, 2)
#define fs_initcall(fn)      __define_initcall(fn, 3)
#define devices_initcall(fn) __define_initcall(fn, 4)

#ifdef __cplusplus
extern "C" {
#endif

void __invoke_initcall0(void);
void __invoke_initcall1(void);
void __invoke_initcall2(void);
void __invoke_initcall3(void);
void __invoke_initcall4(void);

#ifdef __cplusplus
}
#endif



static inline void invoke_initcall_early(void) {
	__invoke_initcall0();
}
static inline void invoke_initcall_arch(void) {
	__invoke_initcall1();
}
static inline void invoke_initcall_bus(void) {
	__invoke_initcall2();
}
static inline void invoke_initcall_fs(void) {
	__invoke_initcall3();
}
static inline void invoke_initcall_devices(void) {
	__invoke_initcall4();
}

#endif
