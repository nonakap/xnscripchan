/*	$Id: screen.c,v 1.16 2002/01/13 21:43:46 nonaka Exp $	*/

/*
 * Copyright (c) 2001, 2002 NONAKA Kimihiro <aw9k-nnk@asahi-net.or.jp>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "nscr.h"

static int update;
static int grayscale;
static long nega;

static image_t *bg;
static image_t *curr;

static int is_show_text = 1;
static int is_show_text_effect = 1;
static point_t window_pos;
static image_t *text_window;

void
screen_init(void)
{

	curr = img_create(SCREEN_WIDTH, SCREEN_HEIGHT, BPP);
	bzero(curr->data, curr->width * curr->height * BPP);

	bg = img_create(SCREEN_WIDTH, SCREEN_HEIGHT, BPP);
	bzero(bg->data, bg->width * bg->height * BPP);
}

void
screen_set_bg(char *name, int effect)
{

	if (bg)
		img_destroy(bg);

	if (strcasecmp(name, "black") == 0) {
		bg = img_create(SCREEN_WIDTH, SCREEN_HEIGHT, BPP);
		bzero(bg->data, bg->width * bg->height * BPP);
	} else if (strcasecmp(name, "white") == 0) {
		bg = img_create(SCREEN_WIDTH, SCREEN_HEIGHT, BPP);
		memset(bg->data, 0xff, bg->width * bg->height * BPP);
	} else {
		bg = image_open(name, TRANSMODE_COPY);
	}
	hide_chr('a', 0);
	CMSG->is_show_window = 0;

	screen_update();
	redraw(effect);
}

void
screen_setwindow(unsigned char *filename, int color, rect_t *rect)
{
	image_t *img;
	unsigned char *p;
	int r, g, b;
	int i;

	_ASSERT(rect != NULL);

	window_pos.x = rect->left;
	window_pos.y = rect->top;

	if (text_window)
		img_destroy(text_window);

	if (filename != NULL) {
		img = image_open(filename, TRANSMODE_TAG);
		_ASSERT(img != NULL);
	} else {
		img = img_create(rect->width, rect->height, BPP);
		_ASSERT(img != NULL);

		bzero(img->data, img->width * img->height * BPP);

		img->mask_size = img->width * img->height * BPP;
		img->mask = Emalloc(img->mask_size);
		img->mask_bpp = 24;

		r = color & 0xff;
		g = (color >> 8) & 0xff;
		b = (color >> 16) & 0xff;
		p = img->mask;
		for (i = 0; i < img->width * img->height; i++) {
			*p++ = r; *p++ = g; *p++ = b;
		}
	}
	text_window = img;

	screen_update();
}

void
screen_grayscale(int v)
{

	grayscale = v;

	screen_update();
}

void
screen_nega(long v)
{

	_ASSERT(v >= 0 && v <= 2);

	nega = v;

	screen_update();
}

void
screen_update(void)
{

	update = 1;
}

void
screen_text_onoff(int flag)
{

	is_show_text = flag;
}

void
screen_text_window_effect_onoff(int flag)
{

	is_show_text_effect = flag;
}

void
screen_show_text_window(void)
{

	img_compose(curr, text_window, 0, window_pos.x, window_pos.y);
	sys_copy_area(curr, window_pos.x, window_pos.y,
	    text_window->width, text_window->height,
	    window_pos.x, window_pos.y);
}

void
redraw(int effect)
{

	img_copy_image(curr, bg);
	merge_sprite(curr);
	if (nega == 1)
		img_nega(curr, 0);
	if (grayscale)
		img_grayscale(curr, grayscale, 0);
	if (nega == 2)
		img_nega(curr, 0);
	if (CMSG->is_show_window)
		img_compose(curr, text_window, 0, window_pos.x, window_pos.y);

	switch (effect) {
	case 0:
		break;

	default:
		sys_copy_area(curr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
		break;
	}

	if (is_show_text)
		text_redraw();

	update = 0;
}
