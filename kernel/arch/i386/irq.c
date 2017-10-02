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
#include <kernel/irq.h>
#include <kernel/pic.h>
#include <string.h>
#include <stdio.h>


#define DEFIRQWRAPPER(irqnum)\
void *irq##irqnum##handler(void)\
{\
	volatile void *addr;\
	asm goto("jmp %l[endofISR]" ::: "memory" : endofISR);\
	asm volatile(".align 16" ::: "memory");\
	startofISR:\
	asm volatile("pushal\n\tpushl %%ebp\n\tmovl %%esp, %%ebp\n\tcld" ::: "memory");\
	asm volatile(\
		"pushl %%ds       \n\t"\
		"pushl %%es       \n\t"\
		"movw $16, %%cx   \n\t"\
		"movw %%cx, %%ds  \n\t"\
		"movw %%cx, %%es  \n\t"\
		"pushl %%ebp      \n\t"\
		"addl $4, (%%esp) \n\t"\
		"pushl %%ebx      \n\t"\
		"call *%%eax      \n\t"\
		"addl $8, %%esp       "\
		:: "a"(irqfunc), "b"((uint32_t) irqnum) : "memory");\
	asm volatile("popl %%es\n\tpopl %%ds\n\tleave\n\tpopal\n\tiret" ::: "memory");\
	endofISR:\
	asm goto(\
		".intel_syntax noprefix         \n\t"\
		"mov eax, offset %l[startofISR] \n\t"\
		"mov [ebx], eax                 \n\t"\
		".att_syntax                        "\
		:: "b"(&addr) : "eax", "memory" : startofISR);\
	return((void *) addr);\
}

#define DEFINTWRAPPER(intnum)\
void *int##intnum##handler(void)\
{\
	volatile void *addr;\
	asm goto("jmp %l[endofISR]" ::: "memory" : endofISR);\
	asm volatile(".align 16" ::: "memory");\
	startofISR:\
	asm volatile("pushal\n\tpushl %%ebp\n\tmovl %%esp, %%ebp\n\tcld" ::: "memory");\
	asm volatile(\
		"pushl %%ds       \n\t"\
		"pushl %%es       \n\t"\
		"movw $16, %%cx   \n\t"\
		"movw %%cx, %%ds  \n\t"\
		"movw %%cx, %%es  \n\t"\
		"pushl %%ebp      \n\t"\
		"addl $4, (%%esp) \n\t"\
		"pushl %%ebx      \n\t"\
		"call *%%eax      \n\t"\
		"addl $8, %%esp       "\
		:: "a"(intfunc), "b"((uint32_t) intnum) : "memory");\
	asm volatile("popl %%es\n\tpopl %%ds\n\tleave\n\tpopal\n\tiret" ::: "memory");\
	endofISR:\
	asm goto(\
		".intel_syntax noprefix         \n\t"\
		"mov eax, offset %l[startofISR] \n\t"\
		"mov [ebx], eax                 \n\t"\
		".att_syntax                        "\
		:: "b"(&addr) : "eax", "memory" : startofISR);\
	return((void *) addr);\
}

#define DEFINTWRAPPER_ERR(intnum)\
void *int##intnum##handler(void)\
{\
	volatile void *addr;\
	asm goto("jmp %l[endofISR]" ::: "memory" : endofISR);\
	asm volatile(".align 16" ::: "memory");\
	startofISR:\
	asm volatile(\
		"pushal                \n\t"\
		"pushl %%ebp           \n\t"\
		"movl %%esp, %%ebp     \n\t"\
		"pushl %%ds            \n\t"\
		"pushl %%es            \n\t"\
		"movw $16, %%cx        \n\t"\
		"movw %%cx, %%ds       \n\t"\
		"movw %%cx, %%es       \n\t"\
		"movl 36(%%ebp), %%edx \n\t"\
		"movl %%ebp, %%esi     \n\t"\
		"addl $32, %%esi       \n\t"\
		"movl %%esi, %%edi     \n\t"\
		"addl $4, %%edi        \n\t"\
		"movl $11, %%ecx       \n\t"\
		"std                   \n\t"\
		"rep movsl             \n\t"\
		"add $4, %%esp         \n\t"\
		"cld                       "\
		::: "memory");\
	asm volatile(\
		"pushl %%edx       \n\t"\
		"pushl %%ebp       \n\t"\
		"addl $8, (%%esp)  \n\t"\
		"pushl %%ebx       \n\t"\
		"call *%%eax       \n\t"\
		"addl $12, %%esp       "\
		:: "a"(intfunc_err), "b"((uint32_t) intnum) : "memory");\
	asm volatile("popl %%es\n\tpopl %%ds\n\tleave\n\tpopal\n\tiret" ::: "memory");\
	endofISR:\
	asm goto(\
		".intel_syntax noprefix         \n\t"\
		"mov eax, offset %l[startofISR] \n\t"\
		"mov [ebx], eax                 \n\t"\
		".att_syntax                        "\
		:: "b"(&addr) : "eax", "memory" : startofISR);\
	return((void *) addr);\
}

typedef void* (*irqfunc_t)(void* ctx);
typedef void* (*intfunc_t)(void* ctx);
typedef void* (*intfunc_err_t)(void* ctx, uint32_t err);

irqfunc_t     irqfuncs[16];
intfunc_t     intfuncs[256];
intfunc_err_t intfuncs_err[32];

void taskswitch(void __attribute__ ((unused))* stack)
{
}

void irqfunc(uint32_t irqnum, void *ctx)
{
	void *stack = NULL;
	if(irqnum != 7)
	{
		if(irqfuncs[irqnum] != NULL)
			stack = irqfuncs[irqnum](ctx);
		pic_eoi(irqnum);
		if(stack)
			taskswitch(stack);
	}
	else
	{
		pic_eoi_spurious(irqnum);
	}
}

void intfunc(uint32_t intnum, void *ctx)
{
	void *stack = NULL;
	if(intfuncs[intnum] != NULL)
		stack = intfuncs[intnum](ctx);
	if(stack)
		taskswitch(stack);
}

void intfunc_err(uint32_t intnum, void *ctx, uint32_t errcode)
{
	void *stack = NULL;
	if(intfuncs_err[intnum] != NULL)
		stack = intfuncs_err[intnum](ctx, errcode);
	if(stack)
		taskswitch(stack);
}


DEFIRQWRAPPER(0)
DEFIRQWRAPPER(1)
DEFIRQWRAPPER(2)
DEFIRQWRAPPER(3)
DEFIRQWRAPPER(4)
DEFIRQWRAPPER(5)
DEFIRQWRAPPER(6)
DEFIRQWRAPPER(7)
DEFIRQWRAPPER(8)
DEFIRQWRAPPER(9)
DEFIRQWRAPPER(10)
DEFIRQWRAPPER(11)
DEFIRQWRAPPER(12)
DEFIRQWRAPPER(13)
DEFIRQWRAPPER(14)
DEFIRQWRAPPER(15)

DEFINTWRAPPER(0)//division by 0
DEFINTWRAPPER(1)//debug
DEFINTWRAPPER(2)//NMI
DEFINTWRAPPER(3)//breakpoint
DEFINTWRAPPER(4)//INTO
DEFINTWRAPPER(5)//BOUND
DEFINTWRAPPER(6)//invalid opcode
DEFINTWRAPPER(7)//coprocessor not available
DEFINTWRAPPER(9)//coprocessor segment overrun
DEFINTWRAPPER(16)//coprocessor error

DEFINTWRAPPER_ERR(8)//double fault
DEFINTWRAPPER_ERR(10)//TSS error
DEFINTWRAPPER_ERR(11)//segment not present
DEFINTWRAPPER_ERR(12)//stack fault
DEFINTWRAPPER_ERR(13)//general protection fault
DEFINTWRAPPER_ERR(14)//page fault
 
DEFINTWRAPPER(0x80)//system call

struct idt_entry
{
    uint16_t base_lo;
    uint16_t selector;
    uint8_t zero;
    uint8_t type;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_desc
{
    unsigned short limit;
    uintptr_t base;
} __attribute__((packed)) idt_desc;

static inline void idt_fill(uint64_t __attribute__((unused)) * _entry, uint16_t __attribute__((unused)) selector, void __attribute__((unused)) * offset, uint8_t __attribute__((unused)) type, uint8_t __attribute__((unused)) prv_level)
{
	struct idt_entry* entry = (struct idt_entry*)_entry;
	entry->selector = selector;
	entry->zero = 0;
	entry->base_lo = ((uintptr_t)offset) & 0x0000FFFF;
	entry->base_hi = (((uintptr_t)offset) & 0xFFFF0000) >> 16;
	entry->type = 0x8e;
}

static inline void idt_reload(struct idt_desc* idt_desc) {
	asm volatile
	(
	   "lidt (%0)"
	   :
	   : "r" (idt_desc)
	);
}

void post_ps2();

void* timer_handler(void __attribute__ ((unused)) * ctx) {
	return NULL;
}
void* keyboard_handler(void __attribute__ ((unused))* ctx) {
	unsigned char __attribute__((unused)) scan_code[6];
	int i = 0;
	while((inb_p(0x64) & 1) == 1) {
		scan_code[i] = inb_p(0x60);
		i++;
	}
	post_ps2();
	// printf("KB");
	return NULL;
}
void* div0_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* debug_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* nmi_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* int3_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* int4_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* bounds_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* invalid_opcode_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* nofpu_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* fpuseg_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* fpuerr_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}
void* double_fault_handler(void __attribute__ ((unused))* ctx, uint32_t __attribute__ ((unused)) err) {
	return NULL;
}
void* invalid_tss_handler(void __attribute__ ((unused))* ctx, uint32_t __attribute__ ((unused)) err) {
	return NULL;
}
void* segnp_handler(void __attribute__ ((unused))* ctx, uint32_t __attribute__ ((unused)) err) {
	return NULL;
}
void* stackflt_handler(void __attribute__ ((unused))* ctx, uint32_t __attribute__ ((unused)) err) {
	return NULL;
}
void* gpf_handler(void __attribute__ ((unused))* ctx, uint32_t __attribute__ ((unused)) err) {
	return NULL;
}
void* pauge_fault_handler(void __attribute__ ((unused))* ctx, uint32_t __attribute__ ((unused)) err) {
	return NULL;
}
void* syscall_handler(void __attribute__ ((unused))* ctx) {
	return NULL;
}

void irq_init() {
	memset(intfuncs, 0, sizeof(intfunc_t) * 256);
	memset(irqfuncs, 0, sizeof(irqfunc_t) * 16);
	memset(intfuncs_err, 0, sizeof(intfunc_err_t) * 32);
	memset(idt, 0, 2048);

	irqfuncs[0]  = timer_handler;
	irqfuncs[1]  = keyboard_handler;

	intfuncs[0]  = div0_handler;
	intfuncs[1]  = debug_handler;
	intfuncs[2]  = nmi_handler;
	intfuncs[3]  = int3_handler;
	intfuncs[4]  = int4_handler;
	intfuncs[5]  = bounds_handler;
	intfuncs[6]  = invalid_opcode_handler;
	intfuncs[7]  = nofpu_handler;
	intfuncs[9]  = fpuseg_handler;
	intfuncs[16] = fpuerr_handler;

	intfuncs_err[8] = double_fault_handler;
	intfuncs_err[10] = invalid_tss_handler;
	intfuncs_err[11] = segnp_handler;
	intfuncs_err[12] = stackflt_handler;
	intfuncs_err[13] = gpf_handler;
	intfuncs_err[14] = pauge_fault_handler;
	
	intfuncs[0x80] = syscall_handler;

	idt_fill(idt+32, 8, irq0handler(), 0xe, 0);//IRQ handlers
	idt_fill(idt+33, 8, irq1handler(), 0xe, 0);
	idt_fill(idt+34, 8, irq2handler(), 0xe, 0);
	idt_fill(idt+35, 8, irq3handler(), 0xe, 0);
	idt_fill(idt+36, 8, irq4handler(), 0xe, 0);
	idt_fill(idt+37, 8, irq5handler(), 0xe, 0);
	idt_fill(idt+38, 8, irq6handler(), 0xe, 0);
	idt_fill(idt+39, 8, irq7handler(), 0xe, 0);
	idt_fill(idt+40, 8, irq8handler(), 0xe, 0);
	idt_fill(idt+41, 8, irq9handler(), 0xe, 0);
	idt_fill(idt+42, 8, irq10handler(), 0xe, 0);
	idt_fill(idt+43, 8, irq11handler(), 0xe, 0);
	idt_fill(idt+44, 8, irq12handler(), 0xe, 0);
	idt_fill(idt+45, 8, irq13handler(), 0xe, 0);
	idt_fill(idt+46, 8, irq14handler(), 0xe, 0);
	idt_fill(idt+47, 8, irq15handler(), 0xe, 0);
	 
	idt_fill(idt+0, 8, int0handler(), 0xe, 0);//exception handlers
	idt_fill(idt+1, 8, int1handler(), 0xe, 0);
	idt_fill(idt+2, 8, int2handler(), 0xe, 0);
	idt_fill(idt+3, 8, int3handler(), 0xe, 0);
	idt_fill(idt+4, 8, int4handler(), 0xe, 0);
	idt_fill(idt+5, 8, int5handler(), 0xe, 0);
	idt_fill(idt+6, 8, int6handler(), 0xe, 0);
	idt_fill(idt+7, 8, int7handler(), 0xe, 0);
	idt_fill(idt+8, 8, int8handler(), 0xe, 0);
	idt_fill(idt+9, 8, int9handler(), 0xe, 0);
	idt_fill(idt+10, 8, int10handler(), 0xe, 0);
	idt_fill(idt+11, 8, int11handler(), 0xe, 0);
	idt_fill(idt+12, 8, int12handler(), 0xe, 0);
	idt_fill(idt+13, 8, int13handler(), 0xe, 0);
	idt_fill(idt+14, 8, int14handler(), 0xe, 0);
	idt_fill(idt+16, 8, int16handler(), 0xe, 0);
	idt_fill(idt+0x80, 8, int0x80handler(), 0xe, 3);//system call handler
	idt_desc.base = (uintptr_t)idt;
	idt_desc.limit = 2048;
	idt_reload(&idt_desc);
}