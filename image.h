/*	$Id: image.h,v 1.13 2002/01/18 18:23:39 nonaka Exp $	*/

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

#ifndef	__IMAGE_H__
#define	__IMAGE_H__

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "misc.h"

#define	ROUNDUP(v,m)	((((v) + (m) - 1) / (m)) * (m))

typedef struct point_tag {
	int x;
	int y;
} point_t;

typedef struct rect_tag {
	int left;
	int top;
	int width;
	int height;
} rect_t;

typedef struct palette_tag {
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
	unsigned char	v;
} palette_t;

typedef struct imageinfo_tag {
	unsigned char	*filename;
	int		transcolor;
	int		transmode;
} imageinfo_t;

typedef struct image_tag {
	object_t	obj;

	unsigned char	*name;

	int		width;
	int		height;
	int		bpp;
	int		has_mask;
	int		has_clarity;

	int		transmode;
	int		transcolor;

	int		clarity;

	unsigned char	*data;
	off_t		data_offset;
	size_t		data_size;

	unsigned char	*mask;
	off_t		mask_offset;
	size_t		mask_size;
	int		mask_bpp;

	palette_t	*palette;
	int		npalette;
} image_t;

/* transmode */
enum {
	TRANSMODE_COPY = 0,
	TRANSMODE_LEFTTOP,
	TRANSMODE_RIGHTTOP,
	TRANSMODE_ALPHA,
	TRANSMODE_COLOR,
	TRANSMODE_PALETTE,
	TRANSMODE_MAX,
	TRANSMODE_TAG = -1
};

void	image_init(void);
int	in_rect(rect_t *, point_t *);
image_t	*image_new(void);
image_t	*image_open(unsigned char *, int);
image_t	*img_create(int, int, int);
void	img_destroy(image_t *);
image_t	*img_duplicate_header(image_t *);
image_t	*img_copy_image(image_t*, image_t *);
image_t	*img_duplicate_image(image_t *);
image_t	*img_grayscale(image_t *, int, int);
image_t	*img_nega(image_t *, int);
image_t	*img_compose(image_t *, image_t *, int, int, int);

void	set_transmode_default(int);
void	parse_tag(imageinfo_t *, unsigned char *);

#endif	/* __IMAGE_H__ */
