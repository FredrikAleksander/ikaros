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

#ifndef __SYS__ELF32_H
#define __SYS__ELF32_H 1

#include <stdint.h>
#include <sys/elf.h>

typedef uint16_t elf32_half_t;
typedef uint32_t elf32_off_t;
typedef uint32_t elf32_addr_t;
typedef uint32_t elf32_word_t;
typedef int32_t  elf32_sword_t;

#define ELF_NIDENT 16
#define ELF32_ST_BIND(INFO)	((INFO) >> 4)
#define ELF32_ST_TYPE(INFO)	((INFO) & 0x0F)
# define ELF32_R_SYM(INFO)	((INFO) >> 8)
# define ELF32_R_TYPE(INFO)	((uint8_t)(INFO))

enum rtt_types {
	R_386_NONE		= 0, // No relocation
	R_386_32		= 1, // Symbol + Offset
	R_386_PC32		= 2  // Symbol + Offset - Section Offset
};

typedef struct _elf32_hdr {
	uint8_t      e_ident[ELF_NIDENT];
	elf32_half_t e_type;
	elf32_half_t e_machine;
	elf32_word_t e_version;
	elf32_addr_t e_entry;
	elf32_off_t  e_phoff;
	elf32_off_t  e_shoff;
	elf32_word_t e_flags;
	elf32_half_t e_ehsize;
	elf32_half_t e_phentsize;
	elf32_half_t e_phnum;
	elf32_half_t e_shentsize;
	elf32_half_t e_shnum;
	elf32_half_t e_shstrndx;
} elf32_ehdr_t;

typedef struct _elf32_shdr {
	elf32_word_t sh_name;
	elf32_word_t sh_type;
	elf32_word_t sh_flags;
	elf32_word_t sh_addr;
	elf32_off_t  sh_offset;
	elf32_word_t sh_size;
	elf32_word_t sh_link;
	elf32_word_t sh_info;
	elf32_word_t sh_addralign;
	elf32_word_t sh_entsize;
} elf32_shdr_t;

typedef struct _elf32_sym {
	elf32_word_t st_name;
	elf32_addr_t st_value;
	elf32_word_t st_size;
	uint8_t      st_info;
	uint8_t      st_other;
	elf32_half_t st_shndx;
} elf32_sym_t;

typedef struct _elf32_rel {
	elf32_addr_t r_offset;
	elf32_word_t r_info;
} elf32_rel_t;

typedef struct _elf32_rela {
	elf32_addr_t  r_offset;
	elf32_word_t  r_info;
	elf32_sword_t r_addend;
} elf32_rela_t;

typedef struct _elf32_phdr {
	elf32_word_t p_type;
	elf32_off_t  p_offset;
	elf32_addr_t p_vaddr;
	elf32_addr_t p_paddr;
	elf32_word_t p_filesz;
	elf32_word_t p_memsz;
	elf32_word_t p_flags;
	elf32_word_t p_align;
} elf32_phdr;



#endif
