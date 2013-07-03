/*	$Id: bmp.c,v 1.9 2002/01/15 17:43:07 nonaka Exp $	*/

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

#include "nscr.h"
#include "image.h"

#define	BI_RGB	0
#define	BI_RLE8	1
#define	BI_RLE4	2

int bmp_decode(image_t *, unsigned char *, size_t);

image_t *
get_bmp(FILE *fp, long offset, size_t encode_size)
{
	size_t sz;
	image_t *img;
	unsigned char *data;
	unsigned char sig[2];

	fseek(fp, offset, SEEK_SET);
	sz = fread(sig, 1, 2, fp);
	_ASSERT(sz == 2);
	_ASSERT((sig[0] == 'B') && (sig[1] == 'M'));

	data = (unsigned char *)Emalloc(encode_size);
	fseek(fp, offset, SEEK_SET);
	sz = fread(data, 1, encode_size, fp);
	_ASSERT(sz == encode_size);

	img = image_new();
	bmp_decode(img, data, encode_size);

	Efree(data);

	return img;
}

image_t *
get_bmp_lzss(FILE *fp, long offset, size_t encode_size, size_t decode_size)
{
	image_t *img;
	unsigned char *data;

	data = (unsigned char *)Emalloc(decode_size);
	fseek(fp, offset, SEEK_SET);
	lzss_decode(fp, data, encode_size, decode_size);

	_ASSERT((data[0] == 'B') && (data[1] == 'M'));

	img = image_new();
	bmp_decode(img, data, decode_size);

	Efree(data);

	return img;
}

image_t *
get_bmp_nbz(FILE *fp, long offset, size_t encode_size)
{
	image_t *img;
	unsigned char *data;
	size_t decode_size;

	data = nbz_decode(fp, offset, encode_size, &decode_size);
	_ASSERT((data[0] == 'B') && (data[1] == 'M'));

	img = image_new();
	bmp_decode(img, data, decode_size);

	Efree(data);

	return img;
}

/*
 * Header		14 bytes		BITMAPFILEHEADER 
 *   Signature		2 bytes			'BM' 
 *   FileSize		4 bytes			File size in bytes 
 *   reserved		4 bytes			unused (=0) 
 *   DataOffset		4 bytes			File offset to Raster Data 
 * InfoHeader		40 bytes		BITMAPINFOHEADER 
 *   Size		4 bytes			Size of InfoHeader =40  
 *   Width		4 bytes			Bitmap Width 
 *   Height		4 bytes			Bitmap Height 
 *   Planes		2 bytes			Number of Planes (=1) 
 *   BitCount		2 bytes			Bits per Pixel
 *						  1 = monochrome palette. =1
 *						  4 = 4bit palletized. =16
 *						  8 = 8bit palletized. =256
 *						  16 = 16bit RGB. =65536
 *						  24 = 24bit RGB. =16M
 *   Compression	4 bytes			Type of Compression
 *						  0 = BI_RGB   no compression
 *						  1 = BI_RLE8 8bit RLE encoding 
 *						  2 = BI_RLE4 4bit RLE encoding
 *   ImageSize		4 bytes			(compressed) Size of Image  
 *						  It is valid to set this =0
 * 						    if Compression = 0 
 *   XpixelsPerM	4 bytes			horizontal resolution:
 * 						  Pixels/meter 
 *   YpixelsPerM	4 bytes			vertical resolution:
 * 						  Pixels/meter 
 *   ColorsUsed		4 bytes			Number of actually used colors 
 *   ColorsImportant	4 bytes			Number of important colors  
 *						  0 = all 
 *     ColorTable	4 * NumColors bytes	present only if
 * 						 Info.BitsPerPixel <= 8 colors
 * 						 should be ordered by importance
 *     Red		1 byte			Red intensity 
 *     Green		1 byte			Green intensity 
 *     Blue		1 byte			Blue intensity 
 *     reserved		1 byte			unused (=0) 
 *   repeated NumColors times 
 * Raster Data		Info.ImageSize bytes	The pixel data 
 *
 * http://www.daubnet.com/formats/BMP.html より
 */
int
bmp_decode(image_t *img, unsigned char *data, size_t encode_size)
{
	unsigned char *src, *dest;
	unsigned char *data_end, *image_end;
	size_t decode_size;
	unsigned long offset;
	int compress;
	int bpp;
	int w, h, rw;
	int dx, dy;
	int done;
	int i, j;
	unsigned char c, n;

	_ASSERT(img != NULL);
	_ASSERT(data != NULL);
	_ASSERT(encode_size >= 132);

	/* check signature */
	if (data[0] != 'B' || data[1] != 'M') {
		_ASSERT(0);
		return 1;
	}

	/* check header */
	offset = get_mem_le32(&data[0x0a]);		/* data offset */

	_ASSERT(get_mem_le32(&data[0x0e]) == 40);	/* Windows BMP */
	w = img->width = get_mem_le32(&data[0x12]);
	h = img->height = get_mem_le32(&data[0x16]);
	_ASSERT(img->width > 0);
	_ASSERT(img->height > 0);

	_ASSERT(get_mem_le16(&data[0x1a]) == 1);	/* =1 */

	bpp = get_mem_le16(&data[0x1c]);
	_ASSERT(bpp == 8 || bpp == 16 || bpp == 24);	
	img->bpp = 8 * BPP;

	compress = get_mem_le32(&data[0x1e]);
	decode_size = get_mem_le32(&data[0x22]);

	if (bpp <= 8) {
		img->npalette = get_mem_le32(&data[0x2e]);
		if (img->npalette == 0)
			img->npalette = 1 << bpp;
		_ASSERT(img->npalette <= (1 << bpp));
	}

	/* decode data */
	img->data_size = img->width * img->height * BPP;
	img->data = Ecalloc(img->data_size, 1);

	data_end = data + encode_size;
	image_end = img->data + img->data_size;

	switch (bpp) {
	case 8:
		img->palette = Ecalloc(img->npalette * sizeof(palette_t), 1);
		src = &data[0x36];
		for (i = 0; i < img->npalette; i++, src += 4) {
			img->palette[i].b = src[0];
			img->palette[i].g = src[1];
			img->palette[i].r = src[2];
			/* src[3] unused */
		}

		if (compress == BI_RGB) {
			rw = ROUNDUP(w, 4);
			for (j = 0; j < h; j++) {
				src = data + offset + (h - j - 1) * rw;
				dest = img->data + j * w * BPP;
				for (i = 0; i < w; i++, src++, dest += BPP) {
					_ASSERT(src < data_end);
					_ASSERT(dest < image_end);
					_ASSERT(*src < img->npalette);
					dest[0] = img->palette[*src].r;
					dest[1] = img->palette[*src].g;
					dest[2] = img->palette[*src].b;
				}
			}
		} else if (compress == BI_RLE8) {
			dx = 0;
			dy = h - 1;
			done = 0;
			src = data;
			dest = img->data + dy * w * BPP;
			while (src < data + encode_size
			    && dest < img->data + img->data_size
			    && dest >= img->data
			    && !done) {
				_ASSERT(src < data_end);
				n = *src++;
				_ASSERT(src < data_end);
				c = *src++;
				if (n == 0) {
					switch (c) {
					case 0:
						/* end of line */
						dx = 0;
						if (--dy < 0)
							break;
						dest = img->data + dy * w * BPP;
						break;

					case 1:
						/* end of data */
						done = 1;
						break;

					case 2:
						/* move image offset */
						_ASSERT(src < data_end);
						dx += *src++;
						_ASSERT(src < data_end);
						dy -= *src++;

						if (++dx >= w) {
							dx -= w;
							if (--dy < 0)
								break;
							dest = img->data + (dy * w + dx) * BPP;
						}
						break;

					default:
						/* raw data */
						for (i = 0; i < c; i++, src++, dest += BPP) {
							_ASSERT(src < data_end);
							_ASSERT(dest < image_end);
							dest[0] = img->palette[*src].r;
							dest[1] = img->palette[*src].g;
							dest[2] = img->palette[*src].b;

							if (++dx >= w) {
								dx -= w;
								if (--dy < 0)
									break;
								dest = img->data + (dy * w + dx) * BPP;
							}
						}

						/* skip pad if odd */
						if (c & 1)
							src++;
						break;
					}
				} else {
					_ASSERT(c < img->npalette);
					for (i = 0; i < n; i++, dest += BPP) {
						_ASSERT(src < data_end);
						_ASSERT(dest < image_end);
						dest[0] = img->palette[c].r;
						dest[1] = img->palette[c].g;
						dest[2] = img->palette[c].b;

						if (++dx >= w) {
							dx -= w;
							if (--dy < 0)
								break;
							dest = img->data + (dy * w + dx) * BPP;
						}
					}
				}
			}
		}
		break;

	case 16:
		/* BI_RGB 時は RGB555 で良い */
		rw = ROUNDUP(w * 2, 4);
		for (j = 0; j < h; j++) {
			src = data + offset + (h - j - 1) * rw;
			dest = img->data + j * w * BPP;
			for (i = 0; i < w; i++, src += 2, dest += BPP) {
				_ASSERT(src < data_end);
				_ASSERT(dest < image_end);
				dest[0] = (src[1] & 0x7c) << 1;		/* r */
				dest[1] = ((src[1] & 0x03) << 6)
				            | ((src[0] & 0xe0) >> 2);	/* g */
				dest[2] = (src[0] & 0x1f) << 3;		/* b */
			}
		}
		break;

	case 24:
		rw = ROUNDUP(w * 3, 4);
		for (j = 0; j < h; j++) {
			src = data + offset + (h - j - 1) * rw;
			dest = img->data + j * w * BPP;
			for (i = 0; i < w; i++, src += 3, dest += BPP) {
				_ASSERT(src < data_end);
				_ASSERT(dest < image_end);
				dest[0] = src[2];	/* r */
				dest[1] = src[1];	/* g */
				dest[2] = src[0];	/* b */
			}
		}
		break;

	default:
		_ASSERT(0);
		return 1;
	}

	return 0;
}
