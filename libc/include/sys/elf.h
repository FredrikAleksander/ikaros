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

#ifndef __SYS__ELF_H
#define __SYS__ELF_H 1

enum elf_ident {
	EI_MAG0		= 0, // 0x7F
	EI_MAG1		= 1, // 'E'
	EI_MAG2		= 2, // 'L'
	EI_MAG3		= 3, // 'F'
	EI_CLASS	= 4, // Architecture (32/64)
	EI_DATA		= 5, // Byte Order
	EI_VERSION	= 6, // ELF Version
	EI_OSABI	= 7, // OS Specific
	EI_ABIVERSION	= 8, // OS Specific
	EI_PAD		= 9  // Padding
};
 
# define ELFMAG0	0x7F
# define ELFMAG1	'E'
# define ELFMAG2	'L'
# define ELFMAG3	'F'
 
# define ELFDATA2LSB 1
# define ELFCLASS32 1

enum elf_type {
	ET_NONE = 0,
	ET_REL  = 1,
	ET_EXEC = 2
};

# define EM_386		3  // x86 Machine Type
# define EV_CURRENT	1  // ELF Current Version

# define SHN_UNDEF	(0x00) // Undefined/Not present

enum sht_types {
   SHT_NULL	= 0,   // Null section
   SHT_PROGBITS	= 1,   // Program information
   SHT_SYMTAB	= 2,   // Symbol table
   SHT_STRTAB	= 3,   // String table
   SHT_RELA	= 4,   // Relocation (w/ addend)
   SHT_NOBITS	= 8,   // Not present in file
   SHT_REL		= 9,   // Relocation (no addend)
};

enum sht_attributes {
   SHF_WRITE	= 0x01, // Writable section
   SHF_ALLOC	= 0x02,  // Exists in memory
   SHF_EXECINSTR = 0x4,
   SHF_MASKPROC = 0xf0000000
};

enum stt_bindings {
	STB_LOCAL		= 0, // Local scope
	STB_GLOBAL		= 1, // Global scope
	STB_WEAK		= 2  // Weak, (ie. __attribute__((weak)))
};
 
enum stt_types {
	STT_NOTYPE		= 0, // No type
	STT_OBJECT		= 1, // Variables, arrays, etc.
	STT_FUNC		= 2  // Methods or functions
};

#endif
