/*	$Id: xdimage.h,v 1.2 2001/12/13 16:56:10 nonaka Exp $	*/
/* $libvimage-Id: libxvimage.h,v 1.1 1999/01/16 12:52:55 ryo Exp $ */

/*
 * Copyright (c) 2001 NONAKA Kimihiro <aw9k-nnk@asahi-net.or.jp>
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

#ifndef	__XDIMAGE_H__
#define	__XDIMAGE_H__

typedef struct xdimage_t {
	Display *display;
	Visual *visual;
	int screen;
	int depth;
	image_t *image;
	XImage *ximage;
	Pixmap pixmap;
} xdimage_t;

xdimage_t *img_create_xdimage(Display *, image_t *);
void img_destroy_xdimage(xdimage_t *);

xdimage_t *img_create_xdimage_from_image(Display *, image_t *);
xdimage_t *img_create_ximage_from_xdimage(xdimage_t *);
xdimage_t *img_create_pixmap_from_xdimage(xdimage_t *, Drawable);

#endif	/* __XDIMAGE_H__ */
