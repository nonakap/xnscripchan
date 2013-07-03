/*	$Id: png.c,v 1.4 2002/01/23 16:38:58 nonaka Exp $	*/

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

#include <png.h>

#include "nscr.h"
#include "image.h"

int png_decode(image_t *, unsigned char *, size_t encode_size);

image_t *
get_png(FILE *fp, long offset, size_t encode_size)
{
	size_t size;
	image_t *img;
	unsigned char *data;
	unsigned char sig[8];
	int rv;

	fseek(fp, offset, SEEK_SET);
	size = fread(sig, 1, 8, fp);
	_ASSERT(size == 8);
	_ASSERT(memcmp(sig, "\x89PNG\x0d\x0a\x1a\0a", 8) == 0);

	data = (unsigned char *)Emalloc(encode_size);
	fseek(fp, offset, SEEK_SET);
	size = fread(data, 1, encode_size, fp);
	_ASSERT(size == encode_size);

	img = image_new();
	rv = png_decode(img, data, encode_size);
	_ASSERT(rv == 0);

	Efree(data);

	return img;
}

typedef struct png_io_ptr_tag {
	unsigned char *top;
	unsigned char *curpos;
	size_t size;
} png_io_ptr_t;

static png_io_ptr_t png_io_ptr;

static void
png_read_from_memory(png_structp pngp, png_bytep data, png_size_t length)
{
	png_io_ptr_t *p;
	png_size_t check;

	p = (png_io_ptr_t *)pngp->io_ptr;

	if (p->curpos + length > p->top + p->size)
		check = p->top + p->size - p->curpos;
	else
		check = length;

	memcpy(data, p->curpos, check);
	p->curpos += check;

	if (check != length)
		png_error(pngp, "Read Error");
}

int
png_decode(image_t *img, unsigned char *data, size_t encode_size)
{
	png_structp pngp;
	png_infop infop;
	int width, height, depth, color_type;
	int i;

	pngp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (pngp == NULL)
		goto read_png_bad;

	infop = png_create_info_struct(pngp);
	if (infop == NULL)
		goto read_png_bad1;

	if (setjmp(png_jmpbuf(pngp)))
		goto read_png_bad2;

	png_io_ptr.top = png_io_ptr.curpos = data;
	png_io_ptr.size = encode_size;
	png_set_read_fn(pngp, &png_io_ptr, png_read_from_memory);

	png_read_info(pngp, infop);

	img->width = width = infop->width;
	img->height = height = infop->height;
	depth = infop->bit_depth;
	color_type = infop->color_type;

	switch (color_type) {
	case PNG_COLOR_TYPE_RGB:
		img->bpp = 8 * BPP;
		break;

	default:
		fprintf(stderr, "Not support color_type %d\n", color_type);
		goto read_png_bad1;
	}

	img->data = (unsigned char *)Ecalloc(width * height * BPP, 1);
	png_start_read_image(pngp);
	for (i = 0; i < height; i++)
		png_read_row(pngp, img->data + i * width * BPP, 0);
	png_read_end(pngp, infop);

	png_destroy_read_struct(&pngp, &infop, 0);

	return 0;

read_png_bad2:
	if (img->data)
		Efree(img->data);
read_png_bad1:
	png_destroy_read_struct(&pngp, infop ? &infop : 0, 0);
read_png_bad:
	return 1;
}

int
write_png(object_t *obj, const unsigned char *filename)
{
	png_structp pngp;
	png_infop infop;
	FILE *fp;
	image_t *img;
	volatile int rv = 1;
	int i;

	_ASSERT(obj != NULL);
	_ASSERT(filename != NULL);

	img = (image_t *)obj;
//	_ASSERT(img->obj.type == OBJECT_IMAGE);

	fp = fopen(filename, "wb");
	if (fp == NULL)
		return rv;

	pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (pngp == NULL)
		goto write_png_bad;

	infop = png_create_info_struct(pngp);
	if (infop == NULL)
		goto write_png_bad1;

	if (setjmp(png_jmpbuf(pngp)))
		goto write_png_bad1;

	png_init_io(pngp, fp);

	infop->width = img->width;
	infop->height = img->height;
	infop->compression_type = PNG_COMPRESSION_TYPE_BASE;
	infop->filter_type = PNG_FILTER_TYPE_BASE;
	infop->interlace_type = PNG_INTERLACE_NONE;

	switch (img->bpp) {
	case 1:
		infop->color_type = PNG_COLOR_TYPE_GRAY;
		infop->bit_depth = 1;
		break;

	case 24:
		infop->color_type = PNG_COLOR_TYPE_RGB;
		infop->bit_depth = 8;
		break;

	default:
		fprintf(stderr, "invalid bpp (bpp = %d)\n", img->bpp);
		goto write_png_bad1;
	}

	png_write_info(pngp, infop);

	for (i = 0; i < img->height; i++)
		png_write_row(pngp, img->data + img->width * i * img->bpp / 8);

	png_write_end(pngp, infop);
	rv = 0;

write_png_bad1:
	png_destroy_write_struct(&pngp, infop != NULL ? &infop : NULL);
write_png_bad:
	fclose(fp);

	return rv;
}
