/*	$Id: xdimage.c,v 1.6 2001/12/19 16:22:36 nonaka Exp $	*/
/* $libvimage-Id: ximage.c,v 1.13 1999/01/22 21:21:39 ryo Exp $ */

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

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "nscr.h"
#include "image.h"
#include "xdimage.h"

static void create_ximage(xdimage_t *);
static unsigned char *alloc_ximage(xdimage_t *);
inline static u_int getshift(u_long, int);
inline static u_int getmasklen(u_long, int);
static u_short pack_rgb_to_pixel16(int, int, int, int, int, int,
    int, int, int, int);
static u_long pack_rgb_to_pixel24(int, int, int, int, int, int,
    int, int, int, int);

xdimage_t *
img_create_xdimage(display, image)
	Display *display;
	image_t *image;
{
	xdimage_t *xdimage;
	int screen;

	screen = XDefaultScreen(display);

	xdimage = (xdimage_t *)Emalloc(sizeof(xdimage_t));
	xdimage->display = display;
	xdimage->screen = screen;
	xdimage->visual = DefaultVisual(display, screen);
	xdimage->depth = DefaultDepth(display, screen);
	xdimage->image = image;
	xdimage->ximage = NULL;
	xdimage->pixmap = 0;

	return xdimage;
}

void
img_destroy_xdimage(xdimage)
	xdimage_t *xdimage;
{
	if (xdimage == NULL)
		return;

	if (xdimage->ximage != NULL) {
		XDestroyImage(xdimage->ximage);
		xdimage->ximage = NULL;
	}
	if (xdimage->pixmap != NULL) {
		XFreePixmap(xdimage->display, xdimage->pixmap);
		xdimage->pixmap = NULL;
	}
	Efree(xdimage);
}

xdimage_t *
img_create_xdimage_from_image(display, image)
	Display *display;
	image_t *image;
{

	return img_create_xdimage(display, image);
}

xdimage_t *
img_create_ximage_from_xdimage(xdimage)
	xdimage_t *xdimage;
{
	if (xdimage->ximage == NULL)
		 alloc_ximage(xdimage);

	switch (xdimage->visual->class) {
	case TrueColor:
		create_ximage(xdimage);
		break;
	case DirectColor:
		if (xdimage->depth > 8) {
			create_ximage(xdimage);
			break;
		}
		/* FALLTHROUGH */
	default:
		fprintf(stderr, "img_create_ximage_from_xdimage: "
		    "not support this visual class.\n");
	}

	return xdimage;
}

xdimage_t *
img_create_pixmap_from_xdimage(xdimage, drawable)
	xdimage_t *xdimage;
	Drawable drawable;
{
	Pixmap pixmap;
	image_t *image = xdimage->image;

	img_create_ximage_from_xdimage(xdimage);

	pixmap = xdimage->pixmap;
	if (pixmap == NULL)
		pixmap = XCreatePixmap(
		    xdimage->display,
		    drawable,
		    image->width,
		    image->height,
		    xdimage->depth);

	XPutImage(
	    xdimage->display,
	    pixmap,
	    DefaultGC(xdimage->display, xdimage->screen),
	    xdimage->ximage,
	    0,
	    0,
	    0,
	    0,
	    image->width,
	    image->height);

	xdimage->pixmap = pixmap;

	return xdimage;
}

static unsigned char *
alloc_ximage(xdimage)
	xdimage_t *xdimage;
{
	Display *display = xdimage->display;
	Visual *visual = xdimage->visual;
	image_t *image = xdimage->image;
	unsigned char *p;
	int width = image->width;
	int height = image->height;
	int depth = xdimage->depth;
	int bp;

	if (depth <= 8)
		bp = 8;
	else if (depth <= 16)
		bp = 16;
	else
		bp = 32;

	p = (unsigned char *)Emalloc(width * height * (bp / 8));
	xdimage->ximage = XCreateImage(display, visual, depth, ZPixmap, 0, p,
	    width, height, bp, 0);

	return p;
}

static void
create_ximage(xdimage)
	xdimage_t *xdimage;
{
	Visual *visual = xdimage->visual;
	XImage *ximage = xdimage->ximage;
	image_t *image = xdimage->image;
	unsigned char *datap = ximage->data;
	int bytes_per_line = ximage->bytes_per_line;
	int byte_order = ximage->byte_order;
	int depth = xdimage->depth;
	int width = image->width;
	int height = image->height;

	int rshift, gshift, bshift;
	u_int rmaskl, gmaskl, bmaskl;
	int r, g, b;
	int x, y;

	rshift = getshift(visual->red_mask, depth);
	gshift = getshift(visual->green_mask, depth);
	bshift = getshift(visual->blue_mask, depth);

	rmaskl = getmasklen(visual->red_mask, depth);
	gmaskl = getmasklen(visual->green_mask, depth);
	bmaskl = getmasklen(visual->blue_mask, depth);

	switch (depth) {
	case 15:
	case 16:
	    {
		u_short *p;
		unsigned char *pic = image->data;
		u_short pixel;

		for (y = 0; y < height; y++) {
			p = (unsigned short *)(datap + y * bytes_per_line);
			for (x = 0; x < width; x++) {
				r = *pic++;
				g = *pic++;
				b = *pic++;

				pixel = pack_rgb_to_pixel16(r, g, b,
				    rshift, gshift, bshift,
				    rmaskl, gmaskl, bmaskl, byte_order);
				*p++ = pixel;
			}
		}
	    }
		break;

	case 24:
	    {
		unsigned char *p;
		unsigned char *pic = image->data;
		u_long pixel;

		for (y = 0; y < height; y++){
			p = datap + y * bytes_per_line;
			for (x = 0; x < width; x++){
				r = *pic++;
				g = *pic++;
				b = *pic++;

				pixel = pack_rgb_to_pixel24(r, g, b,
				    rshift, gshift, bshift,
				    rmaskl, gmaskl, bmaskl, byte_order);
				*p++ = pixel & 0xff;
				*p++ = (pixel >> 8) & 0xff;
				*p++ = (pixel >> 16) & 0xff;
			}
		}
	    }
		break;

	case 32:
	    {
		u_long *p;
		unsigned char *pic = image->data;
		u_long pixel;

		for (y = 0; y < height; y++){
			p = (unsigned long *)(datap + y * bytes_per_line);
			for (x = 0; x < width; x++){
				r = *pic++;
				g = *pic++;
				b = *pic++;

				pixel = pack_rgb_to_pixel24(r, g, b,
				    rshift, gshift, bshift,
				    rmaskl, gmaskl, bmaskl, byte_order);
				*p++ = pixel;
			}
		}
	    }
		break;

	default:
		fprintf(stderr, "not support current depth = %d\n", depth);
		break;
	}
}

inline static u_int
getshift(mask, depth)
	u_long mask;
	int depth;
{
	/*                         0 1 2 3 4 5 6 7 8 9 A B C D E F */
	static char f2shift[] = { -1,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0 };
	int shift = 0, shiftoff;

	for (shiftoff = 0; shiftoff < depth; shiftoff += 4) {
		shift = f2shift[mask & 15];
		if (shift >= 0)
			break;
		mask >>= 4;
	}
	return shift + shiftoff;
}

inline static u_int
getmasklen(mask, depth)
	u_long mask;
	int depth;
{
	/*                               0 1 2 3 4 5 6 7 8 9 A B C D E F */
	static unsigned char b2len[] = { 0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4 };
	int len = 0;
	int i;
	int tmp;

	mask >>= getshift(mask, depth);

	for (i = 0; i < depth; i += 4) {
		tmp = b2len[mask & 15];
		len += tmp;
		if (tmp != 4)
			break;
		mask >>= 4;
	}
	return len;
}

static u_short
pack_rgb_to_pixel16(r, g, b,
    rshift, gshift, bshift,
    rmaskl, gmaskl, bmaskl,
    byte_order)
	int r, g, b;
	int rshift, gshift, bshift;
	int rmaskl, gmaskl, bmaskl;
	int byte_order;
{
	u_short pixel;

	r = (r >> (8 - rmaskl)) << rshift;
	g = (g >> (8 - gmaskl)) << gshift;
	b = (b >> (8 - bmaskl)) << bshift;

	if (byte_order == MSBFirst) {
#ifdef WORDS_BIGENDIAN
		/*
		 * big endian -> big endian
		 */
		pixel = r + g + b;
#else
		/*
		 * little endian -> big endian
		 */
		u_short p = r + g + b;
		pixel = ((p << 8)  & 0xff00) | ((p >> 8) & 0x00ff);
#endif
	} else { /* LSBFirst */
#ifdef WORDS_BIGENDIAN
		/*
		 * big endian -> little endian
		 */
		u_short p = r + g + b;
		pixel = ((p << 8) & 0xff00) | ((p >> 8) & 0x00ff);
#else
		/*
		 * little endian -> little endian
		 */
		pixel = r + g + b;
#endif
	}

	return pixel;
}

static u_long
pack_rgb_to_pixel24(r, g, b,
    rshift, gshift, bshift,
    rmaskl, gmaskl, bmaskl,
    byte_order)
	int r, g, b;
	int rshift, gshift, bshift;
	int rmaskl, gmaskl, bmaskl;
	int byte_order;
{
	u_long pixel;

	r = (r >> (8 - rmaskl)) << rshift;
	g = (g >> (8 - gmaskl)) << gshift;
	b = (b >> (8 - bmaskl)) << bshift;

	if (byte_order == MSBFirst) {
#ifdef WORDS_BIGENDIAN
		/*
		 * big endian -> big endian
		 */
		pixel = r + g + b;
#else
		/*
		 * little endian -> big endian
		 */
		u_long p = r + g + b;
		pixel = ((p << 24) & 0xff000000) |
		        ((p <<  8) & 0x00ff0000) |
		        ((p >>  8) & 0x0000ff00) |
		        ((p >> 24) & 0x000000ff);
#endif
	} else {	/* LSBFirst */
#ifdef WORDS_BIGENDIAN
		/*
		 * big endian -> little endian
		 */
		u_long p = r + g + b;
		pixel = ((p << 24) & 0xff000000) |
		        ((p <<  8) & 0x00ff0000) |
		        ((p >>  8) & 0x0000ff00) |
		        ((p >> 24) & 0x000000ff);
#else
		/*
		 * little endian -> little endian
		 */
		pixel = r + g + b;
#endif
	}

	return pixel;
}
