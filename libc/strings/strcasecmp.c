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
#include <strings.h>

static int _tolower(int c) {
	if(c >= 65 && c <= 90) {
		return c + 32;
	}
	return c;
}

int strcasecmp(const char * s0, const char * s1) {
	size_t i = 0;
	int flag = 0;
	int c0, c1;

	while(flag == 0) {
		c0 = _tolower(s0[i]);
		c1 = _tolower(s1[i]);

		if(c0 > c1) {
			flag = 1;
		}
		else if(c0 < c1) {
			flag = -1;
		}

		if(c0 == '\0') {
			break;
		}

		i++;
	}
	return flag;
}

int strncasecmp(const char * s0, const char * s1, size_t max_len) {
	size_t i = 0;
	int flag = 0;
	int c0, c1;

	while(i < max_len && flag == 0) {
		c0 = _tolower(s0[i]);
		c1 = _tolower(s1[i]);

		if(c0 > c1) {
			flag = 1;
		}
		else if(c0 < c1) {
			flag = -1;
		}

		if(c0 == '\0') {
			break;
		}

		i++;
	}
	return flag;
}
