/*	$Id: label.c,v 1.10 2002/01/18 18:52:37 nonaka Exp $	*/

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
#include "label.h"

typedef struct nscrlabel_tag {
	unsigned char *name;
	long offset;
	int line;
	int access;
	struct nscrlabel_tag *next;
} nscrlabel_t;

typedef struct tilde_tag {
	int line;
	long offset;
} tilde_t;

#define	PRIME	211
#define	NHASH	PRIME
#ifndef NHASH
#define	NHASH	1
#endif

#define	NBUF	16

int is_labellog = 0;
unsigned char nscrllog_filename[PATH_MAX] = "NScrllog.dat";

static int ntilde = 0;
static int alloced_ntilde = 0;
static tilde_t *tilde_table = NULL;
static nscrlabel_t *strlabel_table[NHASH];

static nscrlabel_t *
label_lookup(unsigned char *s)
{
	nscrlabel_t *p;

	for (p = strlabel_table[calc_hash(s, NHASH)]; p != NULL; p = p->next) {
		if (strcasecmp(p->name, s) == 0) {
			p->access = 1;
			return p;
		}
	}
	return NULL;
}

void
label_add(unsigned char *s, int line, long offset)
{
	nscrlabel_t **p, *t;
	size_t size;

	_ASSERT(s != NULL);

	size = strlen(s);
	if (size == 0)
		return;

	/* nuke '\r' */
	if (s[size - 1] == '\r')
		s[size - 1] = '\0';

	t = (nscrlabel_t *)Emalloc(sizeof(nscrlabel_t));
	t->name = Estrdup(s);
	t->offset = offset;
	t->line = line;
	t->access = 0;
	t->next = NULL;

	p = &strlabel_table[calc_hash(s, NHASH)];
	for (; *p != NULL; p = &((*p)->next)) {
		_ASSERT(strcasecmp(s, (*p)->name) != 0);
	}
	*p = t;
}

void
label_jump(unsigned char *s)
{
	nscrlabel_t *p;

	p = label_lookup(s);
	_ASSERT(p != NULL);

	DSNAP(("label_jump: offset = %ld, lineno = %d\n", p->offset, p->line));
	fseek(yyin, p->offset, SEEK_SET);
	lineno = p->line;
	yyrestart(yyin);
}

void
label_access(unsigned char *s)
{
	nscrlabel_t *p;

	p = label_lookup(s);
	_ASSERT(p != NULL);

	p->access = 1;
}

int
label_is_access(unsigned char *s)
{
	nscrlabel_t *p;

	_ASSERT(s != NULL);

	for (p = strlabel_table[calc_hash(s, NHASH)]; p != NULL; p = p->next) {
		if (strcasecmp(p->name, s) == 0) {
			if (p->access)
				return 1;
			return 0;
		}
	}
	return 0;
}

void
tilde_add(int line, long offset)
{

	if (ntilde + 1 >= alloced_ntilde) {
		tilde_t *p;
		p = (tilde_t *)Erealloc(tilde_table, sizeof(tilde_t) * 128);
		tilde_table = p;
		alloced_ntilde += 128;
	}

	tilde_table[ntilde].line = line;
	tilde_table[ntilde].offset = offset;
	ntilde++;
}

void
tilde_jump(int dir)
{
	int idx = 0;
	int i;

	switch (dir) {
	case LABEL_TILDE_JUMPB:
		for (i = 0; i < ntilde; i++) {
			if (lineno < tilde_table[i].line)
				break;
			idx = i;
		}
		break;

	case LABEL_TILDE_JUMPF:
		for (i = 0; i < ntilde; i++) {
			if (lineno > tilde_table[i].line)
				continue;

			idx = i;
			break;
		}
		break;

	default:
		_ASSERT(0);
		i = ntilde;
		break;
	}
	_ASSERT(i != ntilde);

	DPRINTF(("jump: %d -> %d[%d]\n", lineno, tilde_table[idx].line, idx));

	fseek(yyin, tilde_table[idx].offset, SEEK_SET);
	lineno = tilde_table[idx].line;
	yyrestart(yyin);
}

void
label_show(void)
{
	nscrlabel_t *p;
	int i;

	for (i = 0; i < NHASH; i++) {
		printf("strtable %d:\n", i);
		for (p = strlabel_table[i]; p != NULL; p = p->next) {
			printf("name: %s\n", p->name);
			printf("offset: %ld\n", p->offset);
			printf("line:   %d\n", p->line);
		}
	}
}

int
save_labellog(void)
{
	unsigned char buf[32];
	nscrlabel_t *p;
	FILE *fp;
	size_t s;
	int count;
	int i;

	if (!is_labellog)
		return 0;

	fp = fopen(nscrllog_filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "save: can't open %s\n", nscrllog_filename);
		return 1;
	}

	/* 個数書き込み */
	/* 分離するのも何だけど、後から個数書くのは面倒だし:-P */
	for (count = 0, i = 0; i < NHASH; i++) {
		for (p = strlabel_table[i]; p != NULL; p = p->next) {
			if (p->access)
				count++;
		}
	}
	snprintf(buf, sizeof(buf), "%d", count);
	for (s = 0; s < strlen(buf); s++)
		put_byte(fp, buf[s]);
	put_byte(fp, 0x0a);

	/* ファイル名書き込み */
	for (i = 0; i < NHASH; i++) {
		for (p = strlabel_table[i]; p != NULL; p = p->next) {
			if (p->access) {
				put_byte(fp, '"');
				for (s = 0; s < strlen(p->name); s++)
					put_byte(fp, p->name[s] ^ 0x84);
				put_byte(fp, '"');
			}
		}
	}

	fclose(fp);

	return 0;
}

int
load_labellog(void)
{
	unsigned char *buf;
	nscrlabel_t *p;
	FILE *fp;
	size_t i, bufsize;
	int ch;
	int c, count;

	if (!is_labellog)
		return 0;

	fp = fopen(nscrllog_filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "load: can't open %s\n", nscrllog_filename);
		return 1;
	}

	count = 0;
	while ((ch = get_byte(fp)) != 0x0a) {
		_ASSERT(isdigit(ch));
		_ASSERT(count < (count * 10) + (ch - '0'));
		count = (count * 10) + (ch - '0');
	}
	if (count <= 0) {
		fclose(fp);
		return 1;
	}

	bufsize = 16;
	buf = (unsigned char *)Emalloc(bufsize);
	for (c = 0; c < count; c++) {
		ch = get_byte(fp);
		if (ch != '"')
			break;

		for (i = 0;; i++) {
			if (i + 1 >= bufsize) {
				unsigned char *tmp;

				bufsize += 16;
				tmp = (unsigned char *)Erealloc(buf, bufsize);
				_ASSERT(tmp != NULL);
				buf = tmp;
			}
			ch = get_byte(fp);
			if (ch == '"') {
				buf[i] = '\0';
				break;
			}
			buf[i] = ch ^ 0x84;

			_ASSERT(!feof(fp));
		}

		p = label_lookup(buf);
		_ASSERT(p != NULL);
	}
	Efree(buf);

	fclose(fp);

	return 0;
}
