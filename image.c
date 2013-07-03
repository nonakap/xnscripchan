/*	$Id: image.c,v 1.21 2002/01/23 16:38:58 nonaka Exp $	*/
/* $libvimage-Id: vimage.c,v 1.14 1999/01/22 21:21:39 ryo Exp $ */

#ifndef	IMAGE_CACHE
#define	IMAGE_CACHE	8
#endif /* IMAGE_CACHE */

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
#include "archive.h"
#include "image.h"

static unsigned char alpha_map[256][256][256];

#if (IMAGE_CACHE > 0)
static int imagecache_init(void);
static image_t *imagecache_query(unsigned char *filename);
static void imagecache_insert(image_t *img, unsigned char *filename);
static void imagecache_delete(image_t *img);
#endif /* IMAGE_CACHE > 0 */

void
image_init(void)
{
	int s, d, a;

	/* create alpha table */
	for (d = 0; d < 256; d++) {
		for (s = 0; s < 256; s++) {
			for (a = 0; a < 256; a++) {
				alpha_map[d][s][a] = (d*a+s*(255-a))/255;
			}
		}
	}

#if (IMAGE_CACHE > 0)
	imagecache_init();
#endif /* IMAGE_CACHE > 0 */
}

int
in_rect(rect_t *rect, point_t *point)
{

	if ((point->x >= rect->left) && (point->x < rect->left + rect->width)
	    && (point->y >= rect->top) && (point->y < rect->top + rect->height))
		return 1;
	return 0;
}

image_t *
image_new(void)
{
	image_t *img;

	img = (image_t *)Ecalloc(1, sizeof(image_t));
	img->obj.type = OBJECT_IMAGE;
	img->obj.ref_count++;

	return img;
}

image_t *
image_open(unsigned char *filename, int transmode)
{
	unsigned char tmp[32];
	arcquery_t aq;
	imageinfo_t info;
	size_t sz, data_size;
	image_t *img;
	unsigned char *data, *p, *q;
	FILE *fp;
	long off;
	int opened;
	int w;
	int r, g, b;
	int i, j;

	_ASSERT(filename != NULL);

	bzero(&info, sizeof(info));
	parse_tag(&info, filename);

#if (IMAGE_CACHE > 0)
	img = imagecache_query(filename);
	if (img)
		return img;
#endif /* IMAGE_CACHE > 0 */

	bzero(&aq, sizeof(aq));
	if (!archive_query(core->arc, info.filename, &aq)) {
		opened = 1;
		/* パス名変換しないと駄目 */
		fp = fopen(info.filename, "rb");
		if (fp == NULL) {
			archive_print(core->arc);
			_ASSERT(fp != NULL);
		}
		off = 0;
		fseek(fp, 0L, SEEK_END);
		aq.size = ftell(fp);
	} else {
		opened = 0;
		fp = aq.fp;
		off = aq.offset;
	}

	fseek(fp, off, SEEK_SET);
	sz = fread(tmp, 1, sizeof(tmp), fp);
	_ASSERT(sz == sizeof(tmp));

	/* ひでー ;-X */
	if (memcmp(&tmp[6], "JFIF", 4) == 0) {
		img = get_jpeg(fp, off, aq.size);
	} else if (tmp[0] == 'B' && tmp[1] == 'M') {
		img = get_bmp(fp, off, aq.size);
	} else if (memcmp(&tmp[0], "\x89PNG\x0d\x0a\x1a\0a", 8) == 0) {
		img = get_png(fp, off, aq.size);
	} else if (aq.is_compress == 1) {
		img = get_spb(fp, off, aq.size, aq.decode_size);
	} else if (aq.is_compress == 2) {
		img = get_bmp_lzss(fp, off, aq.size, aq.decode_size);
	} else if (memcmp(&tmp[4], "BZ", 2) == 0) {
		/* NBZ は BMP, WAV, MID のみサポート(な筈;-X) */
		img = get_bmp_nbz(fp, off, aq.size);
	} else {
		/* 未対応フォーマット */
		printf("Unknown format: %s\n", info.filename);
		mem_dump(tmp, 32, "signature");
		mem_dump((unsigned char *)&aq, sizeof(aq), "archive info");
		_ASSERT(0);
		return NULL;
	}
	_ASSERT(img != NULL);

	img->name = Estrdup(filename);

	if (transmode != TRANSMODE_TAG)
		info.transmode = transmode;

	img->mask_bpp = 8;
	switch (info.transmode) {
	case TRANSMODE_COPY:
		DPRINTF(("TRANSMODE_COPY\n"));
		break;

	case TRANSMODE_LEFTTOP:
	case TRANSMODE_COLOR:
	case TRANSMODE_PALETTE:
		img->mask_size = img->width * img->height;
		img->mask = Emalloc(img->mask_size);
		if (info.transmode == TRANSMODE_LEFTTOP) {
			DPRINTF(("TRANSMODE_LEFTTOP\n"));
			r = img->data[0];
			g = img->data[1];
			b = img->data[2];
		} else if (info.transmode == TRANSMODE_RIGHTTOP) {
			DPRINTF(("TRANSMODE_RIGHTTOP\n"));
			r = img->data[(img->width - 1) * 3 + 0];
			g = img->data[(img->width - 1) * 3 + 1];
			b = img->data[(img->width - 1) * 3 + 2];
		} else if (info.transmode == TRANSMODE_COLOR) {
			DPRINTF(("TRANSMODE_COLOR\n"));
			r = (info.transcolor >> 16) & 0xff;
			g = (info.transcolor >> 8) & 0xff;
			b = info.transcolor & 0xff;
		} else {
			DPRINTF(("TRANSMODE_PALETTE\n"));
			_ASSERT(img->palette != NULL);
			_ASSERT(img->npalette > info.transcolor);
			r = img->palette[info.transcolor].r;
			g = img->palette[info.transcolor].g;
			b = img->palette[info.transcolor].b;
		}
		for (i = j = 0; i < img->width * img->height; i++, j += BPP) {
			if ((img->data[j + 0] == r)
			 && (img->data[j + 1] == g)
			 && (img->data[j + 2] == b)) {
				img->mask[i] = 255;
			} else {
				img->mask[i] = 0;
			}
		}
		break;

	case TRANSMODE_ALPHA:
		DPRINTF(("TRANSMODE_ALPHA\n"));
		w = img->width / 2;
		_ASSERT(w > 0);

		data_size = w * img->height * BPP;
		data = Emalloc(data_size);
		img->mask_size = w * img->height;
		img->mask = Emalloc(img->mask_size);
		for (i = 0; i < img->height; i++) {
			/* 画像部のコピー */
			p = img->data + i * img->width * BPP;
			q = data + i * w * BPP;
			_ASSERT(p < img->data + img->data_size);
			_ASSERT(q < data + data_size);
			memcpy(q, p, w * BPP);

			/* マスク部のコピー */
			p = img->data + (i * img->width + w) * BPP;
			q = img->mask + i * w;
			_ASSERT(p < img->data + img->data_size);
			_ASSERT(q < img->mask + img->mask_size);
			for (j = 0; j < w; j++, p += 3) {
				/* これでいい？ */
				*q++ = p[0];
			}
		}

		Efree(img->data);
		img->width = w;
		img->data = data;
		img->data_size = data_size;
		break;

	default:
		_ASSERT(0);
		break;
	}

	if (opened) {
		fclose(fp);
	}

#if (IMAGE_CACHE > 0)
	imagecache_insert(img, filename);
#endif /* IMAGE_CACHE > 0 */

	return img;
}

image_t *
img_create(int width, int height, int bpp)
{
	image_t *image;

	_ASSERT(width > 0);
	_ASSERT(height > 0);
	_ASSERT(bpp > 0 && bpp <= 4);

	image = (image_t *)Ecalloc(sizeof(image_t), 1);
	image->width = width;
	image->height = height;
	image->bpp = 8 * bpp;
	image->data_size = width * height * bpp;
	image->data = (unsigned char *)Emalloc(image->data_size);

	return image;
}

void
img_destroy(image_t *image)
{

	if (image != NULL) {
#if (IMAGE_CACHE > 0)
		imagecache_delete(image);
#else /* IMAGE_CACHE == 0 */
		if (image->name != NULL)
			Efree(image->name);
		if (image->data != NULL)
			Efree(image->data);
		if (image->mask != NULL)
			Efree(image->mask);
		if (image->palette != NULL)
			Efree(image->palette);
		Efree(image);
#endif /* IMAGE_CACHE > 0 */
	}
}

image_t *
img_duplicate_header(image_t *image)
{
	image_t *new_image;

	new_image = img_create(image->width, image->height, image->bpp / 8);
	if (new_image == NULL)
		return NULL;
	return new_image;
}

image_t *
img_copy_image(image_t *dest, image_t *src)
{
	int width, height;
	int h;

	_ASSERT(src->bpp == dest->bpp);

	if ((src->width == dest->width) && (src->height == dest->height)) {
		memcpy(dest->data, src->data, src->width * src->height * BPP);
	} else {
		DPRINTF(("img_copy_image: part copy\n"));
		width = src->width < dest->width ? src->width:dest->width;
		height = src->height < dest->height ? src->height:dest->height;

		for (h = 0; h < height; h++) {
			memcpy(dest->data + dest->width * h * BPP,
			    src->data + src->width * h * BPP,
			    width * BPP);
		}
	}

	return dest;
}

image_t *
img_duplicate_image(image_t *image)
{
	image_t *new_image;

	new_image = img_duplicate_header(image);
	if (new_image == NULL)
		return NULL;

	img_copy_image(new_image, image);
	return new_image;
}

image_t *
img_grayscale(image_t *bg, int gray, int copy)
{
	image_t *p;

	_ASSERT(bg != NULL);

	if (copy)
		p = img_duplicate_image(bg);
	else
		p = bg;

	_ASSERT(p != NULL);
	_ASSERT(p->data != NULL);

	if (gray != 0) {
		int idx;
		int r, g, b;
		int v;
		int i, j;

		for (j = 0; j < p->height; j++) {
			for (i = 0; i < p->width; i++) {
				idx = (i + p->width * j) * BPP;

				r = p->data[idx + 0];
				g = p->data[idx + 1];
				b = p->data[idx + 2];

				v = r * 77 + g * 150 + b * 29;

				r = v * (gray & 0xff) >> 16;
				g = v * ((gray >> 8) & 0xff) >> 16;
				b = v * ((gray >> 16) & 0xff) >> 16;

				p->data[idx + 0] = r;
				p->data[idx + 1] = g;
				p->data[idx + 2] = b;
			}
		}
	}

	return p;
}

image_t *
img_nega(image_t *bg, int copy)
{
	image_t *p;
	unsigned char *d;

	_ASSERT(bg != NULL);

	if (copy)
		p = img_duplicate_image(bg);
	else
		p = bg;

	_ASSERT(p != NULL);
	_ASSERT(p->data != NULL);

	for (d = p->data; d < p->data + p->data_size; p++)
		*d = 0xff - *d;

	return p;
}

image_t *
img_compose(image_t *bg, image_t *fg, int copy, int xoff, int yoff)
{
	image_t *image = NULL;
	unsigned char *s, *d, *m;
	int width, height;
	int bg_x, bg_y;
	int fg_x, fg_y;
	int h, w;
	unsigned char c;

	_ASSERT(bg != NULL);
	_ASSERT(fg != NULL);
	_ASSERT(bg->bpp == fg->bpp);

	width = bg->width;
	height = bg->height;
	fg_x = fg_y = bg_x = bg_y = 0;

	if (xoff < 0) {
		width += xoff;
		fg_x -= xoff;
		bg_x = 0;
	}
	if (xoff > 0) {
		width -= xoff;
		bg_x += xoff;
		fg_x = 0;
	}
	if (yoff < 0) {
		height += yoff;
		fg_y -= yoff;
		bg_y = 0;
	}
	if (yoff > 0) {
		height -= yoff;
		bg_y += yoff;
		fg_y = 0;
	}

	if (width > fg->width - fg_x)
		width = fg->width - fg_x;
	if (height > fg->height - fg_y)
		height = fg->height - fg_y;

	if (copy) {
		image = img_duplicate_image(bg);
		_ASSERT(image != NULL);
	} else
		image = bg;

	if (fg->has_clarity) {
		unsigned char t;

		c = fg->clarity;
		if (fg->mask == NULL) {
			for (h = 0; h < height; h++) {
				s = fg->data + ((fg_y + h) * fg->width + fg_x) * BPP;
				d = image->data + ((bg_y + h) * image->width + bg_x) * BPP;
				for (w = 0; w < width; w++, m++) {
					*d = alpha_map[*s][*d][c];
					d++;
					*d = alpha_map[*s][*d][c];
					d++;
					*d = alpha_map[*s][*d][c];
					d++;
				}
			}
		} else {
			switch (fg->mask_bpp) {
			case 8:
				for (h = 0; h < height; h++) {

					s = fg->data + ((fg_y + h) * fg->width + fg_x) * BPP;
					d = image->data + ((bg_y + h) * image->width + bg_x) * BPP;
					m = fg->mask + (fg_y + h) * fg->width + fg_x;
					for (w = 0; w < width; w++, m++) {
						t = alpha_map[*s][*d][c];
						*d = alpha_map[*d][t][*m];
						d++; s++;
						t = alpha_map[*s][*d][c];
						*d = alpha_map[*d][t][*m];
						d++; s++;
						t = alpha_map[*s][*d][c];
						*d = alpha_map[*d][t][*m];
						d++; s++;
					}
				}
				break;

			case 24:
				for (h = 0; h < height; h++) {
					s = fg->data + ((fg_y + h) * fg->width + fg_x) * BPP;
					d = image->data + ((bg_y + h) * image->width + bg_x) * BPP;
					m = fg->mask + ((fg_y + h) * fg->width + fg_x) * BPP;
					for (w = 0; w < width; w++) {
						t = alpha_map[*s][*d][c];
						*d = alpha_map[*d][t][*m];
						d++; s++; m++;
						t = alpha_map[*s][*d][c];
						*d = alpha_map[*d][t][*m];
						d++; s++; m++;
						t = alpha_map[*s][*d][c];
						*d = alpha_map[*d][t][*m];
						d++; s++; m++;
					}
				}
				break;

			default:
				_ASSERT(0);
				return NULL;
			}
		}
	} else {
		if (fg->mask == NULL) {
			for (h = 0; h < height; h++) {
				memcpy(image->data + ((bg_y + h) * image->width + bg_x) * BPP,
				    fg->data + ((fg_y + h) * fg->width + fg_x) * BPP,
				    width * BPP);
			}
		} else {
			switch (fg->mask_bpp) {
			case 8:
				for (h = 0; h < height; h++) {
					s = fg->data + ((fg_y + h) * fg->width + fg_x) * BPP;
					d = image->data + ((bg_y + h) * image->width + bg_x) * BPP;
					m = fg->mask + (fg_y + h) * fg->width + fg_x;
					for (w = 0; w < width; w++, m++) {
						*d = alpha_map[*d][*s][*m];
						d++; s++;
						*d = alpha_map[*d][*s][*m];
						d++; s++;
						*d = alpha_map[*d][*s][*m];
						d++; s++;
					}
				}
				break;

			case 24:
				for (h = 0; h < height; h++) {
					s = fg->data + ((fg_y + h) * fg->width + fg_x) * BPP;
					d = image->data + ((bg_y + h) * image->width + bg_x) * BPP;
					m = fg->mask + ((fg_y + h) * fg->width + fg_x) * BPP;
					for (w = 0; w < width; w++) {
						*d = alpha_map[*d][*s][*m];
						d++; s++; m++;
						*d = alpha_map[*d][*s][*m];
						d++; s++; m++;
						*d = alpha_map[*d][*s][*m];
						d++; s++; m++;
					}
				}
				break;

			default:
				_ASSERT(0);
				return NULL;
			}
		}
	}

	return image;
}

static int default_transmode = TRANSMODE_LEFTTOP;

void
set_transmode_default(int transmode)
{

	_ASSERT(transmode >= TRANSMODE_COPY && transmode <= TRANSMODE_ALPHA);
	default_transmode = transmode;
}

void
parse_tag(imageinfo_t *info, unsigned char *name)
{
	unsigned char *p, *q;
	unsigned long c;

	_ASSERT(info != NULL);
	_ASSERT(name != NULL);

	info->transmode = default_transmode;
	p = name;
	DPRINTF(("name = %s\n", name));

	_ASSERT(*p != '\0');
	if (*p == ':') {
		p++;
		switch (*p) {
		case 'a':
			DPRINTF(("alpha channel\n"));
			p++;
			info->transmode = TRANSMODE_ALPHA;
			break;

		case 'l':
			DPRINTF(("lefttop pixel\n"));
			p++;
			info->transmode = TRANSMODE_LEFTTOP;
			break;

		case 'r':
			DPRINTF(("righttop pixel\n"));
			p++;
			info->transmode = TRANSMODE_RIGHTTOP;
			break;

		case 'c':
			DPRINTF(("copy\n"));
			p++;
			info->transmode = TRANSMODE_COPY;
			break;

		case '#':
			DPRINTF(("color\n"));
			p++;
			info->transmode = TRANSMODE_COLOR;
			for (c = 0; *p != '\0'; p++) {
				if (isdigit(*p)) {
					c = (c << 4) + (*p - '0');
				} else if (*p >= 'a' && *p <= 'f') {
					c = (c << 4) + (*p - 'a' + 10);
				} else if (*p >= 'A' && *p <= 'F') {
					c = (c << 4) + (*p - 'A' + 10);
				} else
					break;
			}
			_ASSERT(c < 0x01000000);
			info->transcolor = c;
			break;

		case '!':
			DPRINTF(("palette\n"));
			p++;
			info->transmode = TRANSMODE_PALETTE;
			for (c = 0; *p != '\0'; p++) {
				if (!isdigit(*p))
					break;

				c = (c * 10) + (*p - '0');
			}
			_ASSERT(c < 256);
			info->transcolor = c;
			break;

		case '/':
		case ';':
			p++;
			break;

		default:
			_ASSERT(0);
			break;
		}

		/* アニメーション指定は無視 */
		_ASSERT(*p != '\0');
		q = strchr(p, ';');
		if (q) {
			p = q + 1;
		}
	}

	info->filename = p;
}

#if (IMAGE_CACHE > 0)
typedef struct _imagecache_tag {
	image_t			*image;

	unsigned char		*filename;
	int			inuse;

	struct _imagecache_tag	*prev;
	struct _imagecache_tag	*next;
} imagecache_t;

static imagecache_t *cache_top;
static imagecache_t *cache_bottom;
static int cache_num;

static int
imagecache_init(void)
{

	cache_top = cache_bottom = NULL;
	cache_num = 0;

	return 0;
}

static void
imagecache_insert(image_t *img, unsigned char *filename)
{
	imagecache_t *ic;

	ic = (imagecache_t *)Ecalloc(sizeof(imagecache_t), 1);
	ic->image = img;
	ic->filename = Estrdup(filename);
	ic->inuse = 1;

	ic->prev = NULL;
	ic->next = cache_top;
	if (cache_top)
		cache_top->prev = ic;
	cache_top = ic;
	if (cache_bottom == NULL)
		cache_bottom = ic;

	/* delete cache if overflow */
	if (++cache_num > IMAGE_CACHE) {
		/* もう少し解放しないと駄目だよな… */
		for (ic = cache_bottom; ic != NULL; ic = ic->prev)
			if (!ic->inuse)
				break;
		if (ic) {
			cache_num--;

			/* Unlink this image from cache list */
			if (ic == cache_top && ic->next)
				cache_top = ic->next;
			if (ic == cache_bottom && ic->prev)
				cache_bottom = ic->prev;
			if (ic->prev) {
				ic->prev->next = ic->next;
			}
			if (ic->next) {
				ic->next->prev = ic->prev;
			}

			/* Now destroy image */
			if (ic->image) {
				if (ic->image->name)
					Efree(ic->image->name);
				if (ic->image->data)
					Efree(ic->image->data);
				if (ic->image->mask)
					Efree(ic->image->mask);
				if (ic->image->palette)
					Efree(ic->image->palette);
				Efree(ic->image);
			}
			if (ic->filename)
				Efree(ic->filename);
			Efree(ic);
		}
	}
}

static image_t *
imagecache_query(unsigned char *filename)
{
	image_t *img;
	imagecache_t *ic;

	_ASSERT(filename != NULL);

	for (ic = cache_top; ic != NULL; ic = ic->next)
		if (strcasecmp(ic->filename, filename) == 0)
			break;
	if (ic) {
		ic->inuse = 1;
		img = ic->image;

		if (ic != cache_top) {
			if (ic == cache_bottom)
				cache_bottom = ic->prev;
			if (ic->prev)
				ic->prev->next = ic->next;
			if (ic->next)
				ic->next->prev = ic->prev;

			ic->prev = NULL;
			ic->next = cache_top;
			if (cache_top)
				cache_top->prev = ic;
			cache_top = ic;
			if (cache_bottom == NULL)
				cache_bottom = ic;
		}
	} else
		img = NULL;

	return img;
}

static void
imagecache_delete(image_t *img)
{
	imagecache_t *ic;

	_ASSERT(img != NULL);

	for (ic = cache_top; ic != NULL; ic = ic->next)
		if (ic->image == img)
			break;
	if (ic)
		ic->inuse = 0;
}
#endif /* IMAGE_CACHE > 0 */
