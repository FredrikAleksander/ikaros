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
#ifndef __TEST_MEMORY_REGION_H
#define __TEST_MEMORY_REGION_H
#include <unittest.h>
#include <kernel/memory_region.h>

static char* test_find_and_set() {
	memory_region_t region_a;
	memory_region_t region_b;

	uint32_t* bitmap_a;
	uint32_t* bitmap_b;
	page_t    page;

	void* r;

	region_a.next = &region_b;
	region_a.base = 0;
	region_a.length = 64;
	bitmap_a = (uint32_t*)region_a.bitmap;
	bitmap_a[0] = 0xFFFFFFFF;
	bitmap_a[1] = 0xFFFFFFFF;

	region_b.next = 0;
	region_b.base = 64;
	region_b.length = 64;
	bitmap_b = (uint32_t*)region_b.bitmap;
	bitmap_b[0] = 0xFFFFFFFF;
	bitmap_b[1] = 0x3FFFFFFF;
	
	TEST_ASSERT("Page 126 should be clear", memory_region_getbit(&region_a, 126) == 0);
	memory_region_setbit(&region_a, 126, 1);
	TEST_ASSERT("Page 126 should be set", memory_region_getbit(&region_a, 126) == 1);
	TEST_ASSERT("Page 127 should be clear", memory_region_getbit(&region_a, 127) == 0);
	memory_region_setbit(&region_a, 127, 1);
	TEST_ASSERT("Page 127 should be set", memory_region_getbit(&region_a, 127) == 1);
	memory_region_setbit(&region_a, 127, 0);
	TEST_ASSERT("Page 127 should be clear", memory_region_getbit(&region_a, 127) == 0);
	r = memory_region_find_and_set(&region_a, 0, &page);
	TEST_ASSERT("Error during memory_region_find_and_set", r != 0);
	TEST_ASSERT("Page 128 should be set", memory_region_getbit(&region_a, 127) == 1);
	r = memory_region_find_and_set(&region_a, 0, &page);
	TEST_ASSERT("Page was returned, but memory should be full", r == 0);

	return 0;
}

static char* test_memory_region() {
	TEST_RUN(test_find_and_set);
	/*TEST_RUN(test_setbit);
	TEST_RUN(test_getbit);*/
	return 0;
}
#endif