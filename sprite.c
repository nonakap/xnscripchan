/*	$Id: sprite.c,v 1.8 2002/01/14 17:43:42 nonaka Exp $	*/

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

#define	MAX_CHR	3

typedef struct sprite_manage {
	int	flag;
	image_t	*image;
	point_t	point;
} sprmgr_t;

static sprmgr_t spr[MAX_SPRITE];
static sprmgr_t chr[MAX_CHR];

static long chr_priority = 25;
static long chr_underline = 479;

void
load_chr(unsigned char kind, unsigned char *filename, int effect)
{
	int n;

	UNUSED(effect);

	_ASSERT(filename != NULL);

	switch (kind) {
	case 'c':
		n = 0;
		break;

	case 'l':
		n = 1;
		break;

	case 'r':
		n = 2;
		break;

	default:
		_ASSERT(0);
		return;
	}

	_ASSERT(0 <= n && n < MAX_CHR);
	DPRINTF(("load character '%c'\n", kind));
	if (chr[n].image)
		img_destroy(chr[n].image);
	chr[n].image = image_open(filename, TRANSMODE_TAG);
	chr[n].flag = 1;

	screen_update();
	redraw(effect);
}

void
hide_chr(unsigned char kind, int effect)
{
	int i;

	UNUSED(effect);

	switch (kind) {
	case 'c':
		chr[0].flag = 0;
		break;

	case 'l':
		chr[1].flag = 0;
		break;

	case 'r':
		chr[2].flag = 0;
		break;

	case 'a':
		for (i = 0; i < MAX_CHR; i++)
			chr[i].flag = 0;
		break;

	default:
		_ASSERT(0);
		return;
	}

	DPRINTF(("hide character '%c'\n", kind));
	screen_update();
	redraw(effect);
}

void
merge_chr(unsigned char kind, image_t *dest)
{
	int n;
	int xoff, yoff;

	_ASSERT(dest != NULL);

	switch (kind) {
	case 'c':
		if (chr[0].image == NULL)
			return;

		n = 0;
		xoff = (SCREEN_WIDTH - chr[0].image->width) / 2;
		break;

	case 'l':
		if (chr[1].image == NULL)
			return;

		n = 1;
		xoff = (SCREEN_WIDTH / 4) - chr[1].image->width / 2;
		break;

	case 'r':
		if (chr[2].image == NULL)
			return;

		n = 2;
		xoff = ((SCREEN_WIDTH / 4) * 3) - chr[2].image->width / 2;
		break;

	default:
		_ASSERT(0);
		return;
	}

	_ASSERT(0 <= n && n < MAX_CHR);
	if (chr[n].flag) {
		DPRINTF(("merge chr %d\n", n));
		if (chr[n].image->height < chr_underline + 1) {
			yoff = chr_underline + 1 - chr[n].image->height;
		} else {
			yoff = 0;
		}
		img_compose(dest, chr[n].image, 0, xoff, yoff);
	}
}

void
chr_set_priority(long v)
{

	_ASSERT(v >= 0 && v < 50);

	chr_priority = v;
}

void
chr_set_underline(long v)
{

	_ASSERT(v >= 0 && v <= SCREEN_HEIGHT);

	chr_underline = v;
}

void
load_sprite(int n, unsigned char *filename, point_t *p, int show)
{

	_ASSERT(n >= 0 && n < MAX_SPRITE);
	_ASSERT(filename != NULL);
	_ASSERT(p != NULL);

	if (spr[n].image)
		img_destroy(spr[n].image);
	spr[n].image = image_open(filename, TRANSMODE_TAG);
	spr[n].flag = show;
	spr[n].point = *p;
}

void
set_sprite(int n, point_t *point, int abs, long maskv)
{

	_ASSERT(n >= 0 && n < MAX_SPRITE);
	_ASSERT(point != NULL);

	spr[n].point.x += point->x;
	spr[n].point.y += point->y;

	if (abs) {
		_ASSERT(maskv >= 0 && maskv <= 255);
		if (maskv == 255) {
			spr[n].image->has_clarity = 0;
			spr[n].image->clarity = 255;
		} else {
			spr[n].image->has_clarity = 1;
			spr[n].image->clarity = maskv;
		}
	} else {
		_ASSERT(maskv >= -255 && maskv <= 255);
		if (maskv > 0) {
			if (spr[n].image->clarity + maskv >= 255) {
				spr[n].image->has_clarity = 0;
				spr[n].image->clarity = 255;
			} else {
				spr[n].image->has_clarity = 1;
				spr[n].image->clarity += maskv;
			}
		} else if (maskv < 0) {
			maskv = -maskv;
			spr[n].image->has_clarity = 1;
			if (spr[n].image->clarity > maskv) {
				spr[n].image->clarity -= maskv;
			} else {
				spr[n].image->clarity = 0;
			}
		}
	}
}

void
show_sprite(int n, int show)
{

	_ASSERT(n >= 0 && n < MAX_SPRITE);

	spr[n].flag = show;
}

void
clear_sprite(int n)
{
	int i;

	_ASSERT(n == -1 || (n >= 0 && n < MAX_SPRITE));

	if (n == -1) {
		for (i = 0; i < MAX_SPRITE; i++) {
			if (spr[i].image) {
				img_destroy(spr[i].image);
				spr[i].image = NULL;
			}
		}
	} else {
		if (spr[n].image) {
			img_destroy(spr[n].image);
			spr[n].image = NULL;
		}
	}
}

void
merge_sprite(image_t *dest)
{
	int i;

	_ASSERT(dest != NULL);

	for (i = MAX_SPRITE - 1; i >= 0; i--) {
		if (spr[i].image) {
			if (spr[i].flag) {
				DPRINTF(("merge sprite %d\n", i));
				img_compose(dest, spr[i].image, 0,
				    spr[i].point.x, spr[i].point.y);
			}
		}

		if (i == chr_priority) {
			merge_chr('c', dest);
			merge_chr('l', dest);
			merge_chr('r', dest);
		}
	}
}
