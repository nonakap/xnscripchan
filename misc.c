/*	$Id: misc.c,v 1.9 2002/01/11 17:02:47 nonaka Exp $	*/

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
#include "misc.h"

void *
#if defined(NDEBUG)
Emalloc(size_t size)
#else
Emalloc_func(size_t size, const char *file, int line)
#endif
{
	char *p;

	_ASSERT(size != 0);

	p = (char *)malloc(size);
	if (p == NULL) {
		fprintf(stderr, "Emalloc_func: Can't alloc memory.\n");
#if !defined(NDEBUG)
		fprintf(stderr, "file: %s, line: %d\n", file, line);
#endif
		fprintf(stderr, "size = %d\n", size);
#if 1
		_ASSERT(p != NULL);
		exit(255);
#else
		return NULL;
#endif
	}
	return p;
}

void *
#if defined(NDEBUG)
Ecalloc(size_t number, size_t size)
#else
Ecalloc_func(size_t number, size_t size, const char *file, int line)
#endif
{
	char *p;

	_ASSERT(number != 0);
	_ASSERT(size != 0);

	p = (char *)calloc(number, size);
	if (p == NULL) {
		fprintf(stderr, "Ecalloc_func: Can't alloc memory.\n");
#if !defined(NDEBUG)
		fprintf(stderr, "file: %s, line: %d\n", file, line);
#endif
		fprintf(stderr, "number = %d\n", number);
		fprintf(stderr, "size = %d\n", size);
#if 1
		_ASSERT(p != NULL);
		exit(255);
#else
		return NULL;
#endif
	}
	return p;
}

void *
#if defined(NDEBUG)
Erealloc(void *ptr, size_t size)
#else
Erealloc_func(void *ptr, size_t size, const char *file, int line)
#endif
{
	char *p;

	_ASSERT(size != 0);

	p = (char *)realloc(ptr, size);
	if (p == NULL) {
		fprintf(stderr, "Erealloc_func: Can't alloc memory.\n");
#if !defined(NDEBUG)
		fprintf(stderr, "file: %s, line: %d\n", file, line);
#endif
		fprintf(stderr, "ptr = %p\n", ptr);
		fprintf(stderr, "size = %d\n", size);
#if 1
		_ASSERT(p != NULL);
		exit(255);
#else
		return NULL;
#endif
	}
	return p;
}

void
#if defined(NDEBUG)
Efree(void *ptr)
#else
Efree_func(void *ptr, const char *file, int line)
#endif
{

	if (ptr == NULL) {
		fprintf(stderr, "Efree_func: ptr == NULL\n");
#if !defined(NDEBUG)
		fprintf(stderr, "file: %s, line: %d\n", file, line);
#endif
		return;
	}
	free(ptr);
}

char *
#if defined(NDEBUG)
Estrdup(const char *str)
#else
Estrdup_func(const char *str, const char *file, int line)
#endif
{
	char *p;

	_ASSERT(str != NULL);

	p = strdup(str);
	if (p == NULL) {
		fprintf(stderr, "Estrdup_func: Can't alloc memory.\n");
#if !defined(NDEBUG)
		fprintf(stderr, "file: %s, line: %d\n", file, line);
#endif
		fprintf(stderr, "str = %s\n", str);
#if 1
		_ASSERT(p != NULL);
		exit(255);
#else
		return NULL;
#endif
	}
	return p;
}

static unsigned short
sjis2jis(unsigned short sjis)
{
	unsigned short h, l;
    
	h = (sjis >> 8) & 0xff;
	l = sjis & 0xff;

	h -= (h <= 0x9f) ? 0x71 : 0xb1;
	h = (h << 1) + 1;
	if (l > 0x7f)
		l--;
	if (l >= 0x9e) {
		l -= 0x7d;
		h++;
	} else
		l -= 0x1f;

	return (h << 8) | l;
}

static unsigned short
jis2euc(unsigned short jis)
{

	return ((jis + 0x80) & 0xff) + ((jis + 0x8000) & 0xff00);
}

unsigned short
sjis2euc(unsigned short sjis)
{

	return jis2euc(sjis2jis(sjis));
}

unsigned char *
sjis2euc_str(unsigned char *str)
{
	unsigned char *p, *q;
	int iskanji2nd;
	unsigned short sjis;
	unsigned short euc;
	unsigned char w;

	_ASSERT(str != NULL);

	iskanji2nd = 0;
	sjis = 0;
	for (p = q = str; *p != '\0'; p++) {
		if (!iskanji2nd) {
			if ((*p & 0x80) == 0)
				continue;

			w = *p ^ 0x40;
			if ((w >= 0xa0) && (w <= 0xdf)) {
				sjis = *p << 8;
				iskanji2nd = 1;
			} else
				q++;
		} else {
			w = *p & 0x40;
			if ((*p >= 0x40 && *p <= 0x7e)
			    || (*p >= 0x80 && *p <= 0xfc)) {
				sjis |= *p;
				iskanji2nd = 0;

				euc = sjis2euc(sjis);
				*q++ = (euc >> 8) & 0xff;
				*q++ = euc & 0xff;
			} else {
				q++;
			}
		}
	}
	return str;
}

unsigned char
get_byte(FILE *fp)
{
	int c;

	_ASSERT(fp != NULL);

	c = fgetc(fp);

	_ASSERT(c != EOF);

	return (unsigned char)c;
}

void
get_bytes(FILE *fp, unsigned char *buf, size_t size)
{
	size_t i;

	_ASSERT(fp != NULL);
	_ASSERT(buf != NULL);
	_ASSERT(size != 0);

	for (i = 0; i < size; i++)
		buf[i] = get_byte(fp);
}

size_t
get_string(FILE *fp, unsigned char *buf, size_t size)
{
	size_t i;

	_ASSERT(fp != NULL);
	_ASSERT(buf != NULL);
	_ASSERT(size != 0);

	for (i = 0; i < size; i++) {
		buf[i] = get_byte(fp);
		if (buf[i] == '\0')
			break;
	}
	return i;
}

unsigned short
get_le16(FILE *fp)
{
	unsigned short v;

	_ASSERT(fp != NULL);

	v = get_byte(fp);
	v |= get_byte(fp) << 8;

	return v;
}

unsigned int
get_le32(FILE *fp)
{
	unsigned int v;

	_ASSERT(fp != NULL);

	v = get_byte(fp);
	v |= get_byte(fp) << 8;
	v |= get_byte(fp) << 16;
	v |= get_byte(fp) << 24;

	return v;
}

unsigned short
get_be16(FILE *fp)
{
	unsigned short v;

	_ASSERT(fp != NULL);

	v = get_byte(fp) << 8;
	v |= get_byte(fp);

	return v;
}

unsigned int
get_be32(FILE *fp)
{
	unsigned int v;

	_ASSERT(fp != NULL);

	v = get_byte(fp) << 24;
	v |= get_byte(fp) << 16;
	v |= get_byte(fp) << 8;
	v |= get_byte(fp);

	return v;
}

void
put_byte(FILE *fp, int v)
{
	int rv;

	_ASSERT(fp != NULL);

	rv = fputc(v, fp);

	_ASSERT(rv != EOF);
	UNUSED(rv);
}

void
put_le16(FILE *fp, unsigned short v)
{

	_ASSERT(fp != NULL);

	put_byte(fp, v & 0xff);
	put_byte(fp, (v >> 8) & 0xff);
}

void
put_le32(FILE *fp, unsigned int v)
{

	_ASSERT(fp != NULL);

	put_byte(fp, v & 0xff);
	put_byte(fp, (v >> 8) & 0xff);
	put_byte(fp, (v >> 16) & 0xff);
	put_byte(fp, (v >> 24) & 0xff);
}

unsigned short
get_mem_le16(unsigned char *p)
{
	unsigned short v;

	_ASSERT(p != NULL);

	v = p[0];
	v |= p[1] << 8;

	return v;
}

unsigned int
get_mem_le24(unsigned char *p)
{
	unsigned int v;

	_ASSERT(p != NULL);

	v = p[0];
	v |= p[1] << 8;
	v |= p[2] << 16;

	return v;
}

unsigned int
get_mem_le32(unsigned char *p)
{
	unsigned int v;

	_ASSERT(p != NULL);

	v = p[0];
	v |= p[1] << 8;
	v |= p[2] << 16;
	v |= p[3] << 24;

	return v;
}

unsigned short
get_mem_be16(unsigned char *p)
{
	unsigned short v;

	_ASSERT(p != NULL);

	v = p[0] << 8;
	v |= p[1];

	return v;
}

unsigned int
get_mem_be32(unsigned char *p)
{
	unsigned int v;

	_ASSERT(p != NULL);

	v = p[0] << 24;
	v |= p[1] << 16;
	v |= p[2] << 8;
	v |= p[3];

	return v;
}

void
put_mem_le16(unsigned char *p, unsigned short v)
{

	_ASSERT(p != NULL);

	p[0] = v & 0xff;
	p[1] = (v >> 8) & 0xff;
}

void
put_mem_le24(unsigned char *p, unsigned int v)
{

	_ASSERT(p != NULL);

	p[0] = v & 0xff;
	p[1] = (v >> 8) & 0xff;
	p[2] = (v >> 16) & 0xff;
}

void
put_mem_le32(unsigned char *p, unsigned int v)
{

	_ASSERT(p != NULL);

	p[0] = v & 0xff;
	p[1] = (v >> 8) & 0xff;
	p[2] = (v >> 16) & 0xff;
	p[3] = (v >> 24) & 0xff;
}

void
mem_dump(const unsigned char *p, size_t size, const unsigned char *prefix)
{
	unsigned char buf[16];
	size_t s, i;

	for (s = 0; s < size; s++) {
		if ((s % 16) == 0) {
			if (prefix == NULL) {
				printf("%08x: ", s);
			} else {
				printf("%s: ", prefix);
			}
			memset(buf, '.', sizeof(buf));
		}

		printf("%02x ", p[s]);
		if (p[s] >= 0x20 && p[s] <= 0x7e)
			buf[s % 16] = p[s];

		if ((s % 16) == 15) {
			printf("| ");
			for (i = 0; i < sizeof(buf); i++)
				printf("%c", buf[i]);
			printf("\n");
		}
	}
	if ((s % 16) != 0) {
		printf("%*c", 3 * (16 - (s % 16)), ' ');
		printf("| ");
		for (i = 0; i < (s % 16); i++)
			printf("%c", buf[i]);
		printf("\n");
	}
}

int
calc_hash(char *str, int num)
{
	char *p, *q;
	size_t s;
	unsigned int h, g;

	_ASSERT(str != NULL);

	q = Estrdup(str);
	for (s = 0; s < strlen(q); s++) {
		if (islower(q[s]))
			q[s] = toupper(q[s]);
	}

	for (p = q, h = 0; *p != '\0'; p++) {
		h = (h << 4) + *p;
		g = h & 0xf0000000;
		if (g != 0) {
			h ^= g >> 24;
			h ^= g;
		}
	}

	Efree(q);

	return h % num;
}
