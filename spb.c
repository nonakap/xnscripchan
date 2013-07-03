/*	$Id: spb.c,v 1.3 2002/01/15 17:43:07 nonaka Exp $	*/

/*
 * Copyright (c) 2002 NONAKA Kimihiro <aw9k-nnk@asahi-net.or.jp>
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

int spb_decode(image_t *img, unsigned char *data, size_t encode_size);

image_t *
get_spb(FILE *fp, long offset, size_t encode_size, size_t decode_size)
{
	image_t *img;
	unsigned char *data;
	size_t size;

	_ASSERT(fp != NULL);
	_ASSERT(encode_size > 0);
	_ASSERT(decode_size > 0);

	UNUSED(decode_size);

	fseek(fp, offset, SEEK_SET);
	data = (unsigned char *)Emalloc(encode_size);
	size = fread(data, encode_size, 1, fp);
	_ASSERT(size == 1);

	img = (image_t *)Ecalloc(sizeof(image_t), 1);

	spb_decode(img, data, encode_size);

	Efree(data);

	return img;
}

#define	BIT_GETN(d,n)							\
	if (remain_bit < n) {						\
		if (src >= data + encode_size) {			\
			break;						\
		}							\
		bits = (bits << 8) | *src++;				\
		remain_bit += 8;					\
	}								\
	remain_bit -= n;						\
	d = (bits >> remain_bit) & ((1 << n) - 1);

#define	BIT_GET8(d)							\
	if (remain_bit < 8) {						\
		if (src >= data + encode_size) {			\
			break;						\
		}							\
		bits = (bits << 8) | *src++;				\
	}								\
	d = (bits >> remain_bit);

#define	BIT_SET()							\
	if (dest >= tmp_data + tmp_data_size) {				\
		break;							\
	}								\
	*dest++ = ch;

int
spb_decode(image_t *img, unsigned char *data, size_t encode_size)
{
	unsigned char *src, *dest, *tmp_data;
	unsigned char *p, *q;
	size_t tmp_data_size;
	unsigned int bits;
	int nbit, mask, remain_bit;
	int t;
	unsigned char ch;
	int rgb = 2;
	int i, j;

	_ASSERT(img != NULL);
	_ASSERT(data != NULL);
	_ASSERT(encode_size > 5);

	img->width = get_mem_be16(&data[0]);
	img->height = get_mem_be16(&data[2]);
	img->bpp = 8 * BPP;

	_ASSERT(img->width > 0);
	_ASSERT(img->height > 0);

	img->data_size = img->width * img->height * BPP;
	img->data = (unsigned char *)Emalloc(img->data_size);

	tmp_data_size = img->width * img->height;
	tmp_data = (unsigned char *)Emalloc(tmp_data_size);

	src = &data[4];
	dest = tmp_data;
	bits = 0;
	remain_bit = 0;
	for (rgb = 2; rgb >= 0; rgb--, dest = tmp_data) {
		BIT_GET8(ch);
		BIT_SET();

		while (dest < tmp_data + tmp_data_size) {
			BIT_GETN(nbit, 3);
			if (nbit == 0) {
				BIT_SET();
				BIT_SET();
				BIT_SET();
				BIT_SET();
				continue;
			}

			if (nbit == 7) {
				BIT_GETN(mask, 1);
				mask++;
			} else {
				mask = nbit + 2;
			}

			for (i = 0; i < 4; i++) {
				if (mask == 8) {
					BIT_GET8(ch);
				} else {
					BIT_GETN(t, mask);
					if (t & 1) {
						ch += (t >> 1) + 1;
					} else {
						ch -= (t >> 1);
					}
				}
				BIT_SET();
			}
		}

		p = tmp_data;
		q = img->data + rgb;
		for (j = 0; j < img->height / 2; j++) {
			for (i = 0; i < img->width; i++) {
				*q = *p++;
				q += 3;
			}
			q += img->width * 3;
			for (i = 0; i < img->width; i++) {
				q -= 3;
				*q = *p++;
			}
			q += img->width * 3;
		}
		if (img->height & 1) {
			for (i = 0; i < img->width; i++) {
				*q = *p++;
				q += 3;
			}
		}
	}

	Efree(tmp_data);

	return 0;
}
