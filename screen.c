/*	$Id: screen.c,v 1.18 2002/01/18 19:36:51 nonaka Exp $	*/

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
#include "image.h"

static int is_show_text = 1;
static int is_show_text_effect = 1;

void
screen_init(void)
{
	image_t *img;

	img = img_create(SCREEN_WIDTH, SCREEN_HEIGHT, BPP);
	bzero(img->data, img->width * img->height * BPP);
	CSCREEN->offscreen = (object_t *)img;

	img = img_create(SCREEN_WIDTH, SCREEN_HEIGHT, BPP);
	bzero(img->data, img->width * img->height * BPP);
	CSCREEN->bg = (object_t *)img;
}

void
screen_set_bg(unsigned char *name)
{
	image_t *img;

	if (strcasecmp(name, "black") == 0) {
		img = img_create(SCREEN_WIDTH, SCREEN_HEIGHT, BPP);
		bzero(img->data, img->width * img->height * BPP);
		img->name = Estrdup("black");
	} else if (strcasecmp(name, "white") == 0) {
		img = img_create(SCREEN_WIDTH, SCREEN_HEIGHT, BPP);
		memset(img->data, 0xff, img->width * img->height * BPP);
		img->name = Estrdup("white");
	} else {
		img = image_open(name, TRANSMODE_COPY);
	}
	if (CSCREEN->bg)
		img_destroy((image_t *)CSCREEN->bg);
	CSCREEN->bg = (object_t *)img;

	hide_chr('a');
	CMSG->is_show_window = 0;

	screen_update();
}

void
screen_setwindow(unsigned char *filename, int color, rect_t *rect)
{
	unsigned char colormap[16];
	image_t *img;
	unsigned char *p;
	int r, g, b;
	int i;

	_ASSERT(rect != NULL);
	CSCREEN->window_pos = *rect;

	if (filename) {
		img = image_open(filename, TRANSMODE_TAG);
		_ASSERT(img != NULL);

		rect->width = img->width;
		rect->height = img->height;
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

		snprintf(colormap, sizeof(colormap), "#%06x", color);
		img->name = Estrdup(colormap);
	}
	if (CSCREEN->text_window)
		img_destroy((image_t *)CSCREEN->text_window);
	CSCREEN->text_window = (object_t *)img;
	CSCREEN->window_pos = *rect;

	screen_update();
}

void
screen_grayscale(int v)
{

	CSCREEN->grayscale = v;

	screen_update();
}

void
screen_nega(long v)
{

	_ASSERT(v >= 0 && v <= 2);

	CSCREEN->nega = v;

	screen_update();
}

void
screen_update(void)
{

	img_copy_image((image_t *)CSCREEN->offscreen, (image_t *)CSCREEN->bg);
	merge_sprite((image_t *)CSCREEN->offscreen);
	if (CSCREEN->nega == 1)
		img_nega((image_t *)CSCREEN->offscreen, 0);
	if (CSCREEN->grayscale)
		img_grayscale((image_t *)CSCREEN->offscreen,
		    CSCREEN->grayscale, 0);
	if (CSCREEN->nega == 2)
		img_nega((image_t *)CSCREEN->offscreen, 0);
	if (CMSG->is_show_window)
		img_compose((image_t *)CSCREEN->offscreen,
		    (image_t *)CSCREEN->text_window, 0,
		    CSCREEN->window_pos.left, CSCREEN->window_pos.top);
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

	img_compose((image_t *)CSCREEN->offscreen,
	    (image_t *)CSCREEN->text_window, 0,
	    CSCREEN->window_pos.left, CSCREEN->window_pos.top);
	sys_copy_area((image_t *)CSCREEN->offscreen,
	    CSCREEN->window_pos.left, CSCREEN->window_pos.top,
	    CSCREEN->window_pos.width, CSCREEN->window_pos.height,
	    CSCREEN->window_pos.left, CSCREEN->window_pos.top);
}

void
redraw(rect_t *rect)
{

	sys_copy_area((image_t *)CSCREEN->offscreen,
	    rect->left, rect->top, rect->width, rect->height,
	    rect->left, rect->top);

	if (is_show_text)
		text_redraw();
}
