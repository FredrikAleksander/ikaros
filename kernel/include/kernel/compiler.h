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
#ifndef __KERNEL__COMPILER_H
#define __KERNEL__COMPILER_H 1

#include <stdint.h>

// Set up preprocessor magic for using the amazing sparse utility.
// Sparse is a tool made for doing static code analysis of the Linux kernel,
// but it works GREAT for any kernel.
#ifdef __CHECKER__
# define __user			__attribute__((noderef, address_space(1)))
# define __kernel   	__attribute__((address_space(0)))
# define __iomem    	__attribute__((noderef, address_space(2)))
# define __private  	__attribute__((noderef))
# define __must_hold(x)	__attribute__((context(x,1,1)))
# define __acquires(x)	__attribute__((context(x,0,1)))
# define __releases(x)  __attribute__((context(x,1,0)))
# define __acquire(x)	__context__(x,1)
# define __release(x)   __context__(x,-1)
# define __bitwise      __attribute__((bitwise))
#else
# define __user
# define __kernel
# define __iomem
# define __private
# define __must_hold(x)
# define __acquires(x)
# define __releases(x)
# define __acquire(x)
# define __release(x)
# define __bitwise
#endif

typedef int16_t  __bitwise int16_le_t;
typedef int32_t  __bitwise int32_le_t;
typedef int64_t  __bitwise int64_le_t;
typedef uint16_t __bitwise uint16_le_t;
typedef uint32_t __bitwise uint32_le_t;
typedef uint64_t __bitwise uint64_le_t;

typedef int16_t  __bitwise int16_be_t;
typedef int32_t  __bitwise int32_be_t;
typedef int64_t  __bitwise int64_be_t;
typedef uint16_t __bitwise uint16_be_t;
typedef uint32_t __bitwise uint32_be_t;
typedef uint64_t __bitwise uint64_be_t;

#endif
