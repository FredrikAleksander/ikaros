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
#include <kernel/console/fbconsole.h>
#include <kernel/console/consolefont.h>
#include <kernel/initcall.h>
#include <kernel/debug/serial_debug.h>
#include <stdlib.h>
#include <string.h>



static struct console_fb_palette vga_palette_colors[16];

static uint8_t* glyphs;

static console_framebuffer_t* framebuffer;

static void fb_console_draw_glyph(console_framebuffer_t* fb, uint8_t ch, uint16_t x,
	uint16_t y, uint8_t* lut_pair, uint8_t lut_size)
{
	// uint8_t black[3] = { 0, 0, 0 };
	// uint8_t white[3] = { 255, 255, 255 };


	uint8_t i,j,k;
	uint8_t* fg = &lut_pair[0];
	uint8_t* bg = &lut_pair[lut_size];

	uint16_t fb_x = x << 3;
	uint16_t fb_y = y * 16;

	uint8_t* ptr;
	uint8_t* color;

	for(i = 0; i < 16; i++) {
		uint8_t row = glyphs[_binary_fonts_console_psf_start.charsize * ch + i];
		for(j = 0; j < 8; j++) {
			ptr = &fb->data[fb->pitch * (fb_y + i) + ((fb_x + j) * lut_size)];
			if(row & (1 << (8-j))) {
				color = fg;//white;
			}
			else {
				color = bg;//black;
			}

			// if(i % 3 == 0) {
			// 	color = white;
			// }
			// else {
			// 	color = black;
			// }

			for(k = 0; k < lut_size; k++) {
				ptr[k] = color[k];
				(void)ptr;
				(void)color;
				(void)lut_pair;
			}
		}
	}
}

extern void fb_console_redraw(console_t* cons) {
	uint16_t i,j;
	uint8_t ch;
	uint8_t attrs;
	console_framebuffer_t* fb = cons->private;
	uint16_t pixel_size = (fb->bpp + 7) >> 3;
	uint16_t pair_size  = pixel_size << 1;
	uint8_t* pairs = fb->private;
	uint8_t* pair;

	for(i = 0; i < cons->rows; i++) {
		for(j = 0; j < cons->columns; j++) {
			ch    = cons->buffer[i*cons->columns+j].ch;
			attrs = cons->buffer[i*cons->columns+j].attrs;
			pair  = &pairs[attrs * pair_size];

			fb_console_draw_glyph(fb, ch, j, i, pair, pixel_size);
		}
	}
}

const struct console_operations fb_console_ops = {
	fb_console_redraw
};

static inline int fb_palette_match(struct console_fb_palette* pal, uint8_t color) {
	return pal->r == vga_palette_colors[color].r &&
		pal->g == vga_palette_colors[color].g &&
		pal->b == vga_palette_colors[color].b ? 1 : 0;
}

static inline void fb_console_lut_indexed(console_framebuffer_t* fb, 
	uint8_t* lut, uint8_t color)
{
	uint8_t i;
	
	// Framebuffer uses a palette.
	// Because the palette may not have the exact colors
	// used by VGA, this can get tricky.
	for(i = 0; i < fb->palette_num; i++) {
		if(fb_palette_match(&fb->palette[i], color)) {
			lut[0] = i;
			return;
		}
	}

	lut[0] = color;
}

static inline uint8_t bitmask(int x) {
	uint8_t m = 0;
	for(int i = 0; i < x; i++) {
		m |= 1 << i;
	}
	return m;
}

static inline void fb_console_lut_rgb(console_framebuffer_t* fb, 
	uint8_t* lut, uint8_t color)
{
	uint8_t* tmp;
	uint8_t  bpp = (fb->bpp + 7) >> 3;
	struct console_fb_color_fmt* fmt = &fb->format;
	uint32_t clr;
	struct console_fb_palette pal = vga_palette_colors[color];

	clr = 0;

	uint8_t red_mask   = bitmask(fmt->red_mask_size);
	uint8_t green_mask = bitmask(fmt->green_mask_size);
	uint8_t blue_mask  = bitmask(fmt->blue_mask_size);

	clr |= ((pal.r & red_mask) << fmt->red_position);
	clr |= ((pal.g & green_mask) << fmt->green_position);
	clr |= ((pal.b & blue_mask) << fmt->blue_position);

	// (void)color;

	tmp = (uint8_t*)&clr;
	for(int i = 0; i < bpp; i++) {
		lut[i] = tmp[i];
	}
}

static inline void fb_console_lut(console_framebuffer_t* fb, 
	uint8_t* lut, uint8_t color)
{
	if(fb->flags & FB_CONSOLE_FLAG_RGB) {
		fb_console_lut_rgb(fb, lut, color);
	}
	else {
		fb_console_lut_indexed(fb, lut, color);
	}
}

static inline void fb_console_create_lut(console_framebuffer_t* fb) {
	int i;
	uint8_t  fg, bg;
	uint32_t pixel_size = (fb->bpp + 7) >> 3;
	uint32_t pair_size  = pixel_size << 1;
	uint8_t* lut;
	uint8_t buffer[64];

	for(i = 0; i < 16; i++) {
		fb_console_lut(fb, &buffer[i*pixel_size], i);
	}

	fb->private = malloc(pair_size * 256);
	lut = fb->private;
	for(i = 0; i < 256; i++) {
		fg = i & 0x0F;
		bg = (i & 0xF0) >> 4;

		memcpy(&lut[i*pair_size], &buffer[fg * pixel_size], pixel_size);
		memcpy(&lut[i*pair_size+pixel_size], &buffer[bg * pixel_size], pixel_size);
	}
}

static void _set_color(struct console_fb_palette* color, uint8_t r, uint8_t g, uint8_t b) {
	color->r = r;
	color->g = g;
	color->b = b;
}

static int __init_font(void) {
	_set_color(vga_palette_colors + 0, 0, 0, 0);       /* Black          */
	_set_color(vga_palette_colors + 1, 0, 0, 170);     /* Blue           */
	_set_color(vga_palette_colors + 2, 0, 170, 0),     /* Green          */
	_set_color(vga_palette_colors + 3, 0, 170, 170);   /* Cyan           */
	_set_color(vga_palette_colors + 4, 170, 0, 0);     /* Red            */
	_set_color(vga_palette_colors + 5, 170, 0, 170);   /* Magenta        */
	_set_color(vga_palette_colors + 6, 170, 85, 0);    /* Brown          */
	_set_color(vga_palette_colors + 7, 170, 170, 170); /* Gray           */
	_set_color(vga_palette_colors + 8, 85, 85, 85);    /* Dark Gray      */
	_set_color(vga_palette_colors + 9, 85, 85, 255);   /* Bright Blue    */
	_set_color(vga_palette_colors + 10, 85, 255, 85);   /* Bright Green   */
	_set_color(vga_palette_colors + 11, 85, 255, 255);  /* Bright Cyan    */
	_set_color(vga_palette_colors + 12, 255, 85, 85);   /* Bright Red     */
	_set_color(vga_palette_colors + 13, 255, 85, 255);  /* Bright Magenta */
	_set_color(vga_palette_colors + 14, 255, 255, 255); /* Yellow         */
	_set_color(vga_palette_colors + 15, 255, 255, 255); /* White          */

	glyphs = NULL;
	if(_binary_fonts_console_psf_start.magic[0] != PSF1_MAGIC0 ||
		_binary_fonts_console_psf_start.magic[1] != PSF1_MAGIC1)
	{
		asm volatile("cli; hlt");
		return 1;
	}
	glyphs = ((uint8_t*)&_binary_fonts_console_psf_start) + 4;

	return INIT_OK;
}

early_initcall(__init_font);

void __init_fb_console(console_framebuffer_t* fb) {
	framebuffer = fb;
	console_t* cons = malloc(sizeof(console_t));
	memset(cons, 0, sizeof(console_t));
	cons->columns = fb->width / 8;
	cons->rows = fb->height / 16;
	cons->buffer = (console_char_t*)malloc(sizeof(console_char_t)*cons->rows*cons->columns);
	cons->flags = CONSOLE_FLAG_AUTODRAW | CONSOLE_FLAG_SHOW_CURSOR;
	cons->ops = &fb_console_ops;
	cons->private = fb;
	fb_console_create_lut(fb);
	__console_initialize(cons);
}
