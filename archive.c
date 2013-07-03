/*	$Id: archive.c,v 1.5 2002/01/10 17:25:32 nonaka Exp $	*/

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

#define	HASHNUM	1023

int is_fchklog = 0;
unsigned char nscrflog_filename[PATH_MAX] = "NScrflog.dat";

object_t *
archive_new(int hash_num, int (*calc_func)(char *, int))
{
	archive_t *arc;

	arc = (archive_t *)Ecalloc(1, sizeof(archive_t));
	arc->obj.type = OBJECT_ARCHIVE;
	arc->hash_num = hash_num ? hash_num : HASHNUM;
	arc->calc_hash = calc_func ? calc_func : calc_hash;
	arc->table_count = (int *)Ecalloc(sizeof(int) * arc->hash_num, 1);
	arc->table = (arcfile_t**)Ecalloc(sizeof(arcfile_t) * arc->hash_num, 1);

	return (object_t *)arc;
}

void
archive_delete(object_t *obj)
{
	archive_t *arc;
	int i;

	_ASSERT(obj != NULL);
	_ASSERT(obj->type == OBJECT_ARCHIVE);

	if (--obj->ref_count > 0)
		return;

	arc = (archive_t *)obj;

	if (arc->fd != NULL) {
		for (i = 0; i < arc->fd_num; i++) {
			if (arc->fd[i] != NULL) {
				fclose(arc->fd[i]->fp);
				arc->fd[i] = NULL;
			}
		}
		Efree(arc->fd);
		arc->fd = NULL;
	}

	if (arc->table_count != NULL) {
		Efree(arc->table_count);
		arc->table_count = NULL;
	}
	if (arc->table != NULL) {
		for (i = 0; i < arc->hash_num; i++) {
			if (arc->table[i] != NULL) {
				Efree(arc->table[i]);
				arc->table[i] = NULL;
			}
		}
		Efree(arc->table);
		arc->table = NULL;
	}
}

int
archive_open(object_t *obj, unsigned char *arc_name)
{
	archive_t *arc;
	FILE *fp;
	unsigned char *p;
	int fd_idx;
	int rv;
	int i;

	_ASSERT(obj != NULL);
	_ASSERT(obj->type == OBJECT_ARCHIVE);
	_ASSERT(arc_name != NULL);

	fp = fopen(arc_name, "rb");
	if (fp == NULL) {
		return 1;
	}

	arc = (archive_t *)obj;

	for (i = 0; i < arc->fd_num; i++) {
		_ASSERT(arc->fd[i] != NULL);
		if (strcasecmp(arc->fd[i]->name, arc_name) == 0) {
			/* ファイルがオープン済みの場合、何もしなくても良い */
			return 0;
		}
	}

	/* glow file desc. table */
	if (arc->fd_num >= arc->fd_max) {
		file_desc_t **fdpp;

		arc->fd_max = arc->fd_max ? arc->fd_max * 2 : 1;
		fdpp = (file_desc_t **)Erealloc(arc->fd,
		    arc->fd_max * sizeof(file_desc_t *));
		arc->fd = fdpp;
		for (i = arc->fd_num; i < arc->fd_max; i++)
			fdpp[i] = NULL;
	}

	fd_idx = arc->fd_num++;
	arc->fd[fd_idx] = (file_desc_t *)Ecalloc(1, sizeof(file_desc_t));
	arc->fd[fd_idx]->fp = fp;
	strlcpy(arc->fd[fd_idx]->name, arc_name, PATH_MAX);

	/* ひでー ;-X */
	p = strrchr(arc_name, '.');
	_ASSERT(p != NULL);
	p++;
	if (strcasecmp(p, "sar") == 0) {
		rv = sar_open(obj, fd_idx);
	} else if (strcasecmp(p, "nsa") == 0) {
		rv = nsa_open(obj, fd_idx);
	} else {
		_ASSERT(0);
		rv = 1;
	}
	_ASSERT(rv == 0);

	arc->obj.ref_count++;

	return 0;
}

int
archive_query(object_t *obj, unsigned char *name, arcquery_t *aqp)
{
	archive_t *arc;
	arcfile_t *p;
	int n, len;

	_ASSERT(obj != NULL);
	_ASSERT(name != NULL);
	_ASSERT(aqp != NULL);

	len = strlen(name);
	_ASSERT(len != 0);

	arc = (archive_t *)obj;

	n = (*arc->calc_hash)(name, arc->hash_num);
	if (arc->table_count[n] == 0)
		return 0;

	for (p = arc->table[n]; p != NULL; p = p->next) {
		if (strncasecmp(p->name, name, len) == 0) {
			bzero(aqp, sizeof(*aqp));
			aqp->obj.type = p->obj.type;
			aqp->fp = *p->fp;
			aqp->offset = p->offset;
			aqp->size = p->size;
			aqp->is_compress = p->is_compress;
			aqp->decode_size = p->decode_size;
			p->access = 1;
			return 1;
		}
	}
	return 0;
}

int
archive_is_access(object_t *obj, unsigned char *name)
{
	archive_t *arc;
	arcfile_t *p;
	size_t n, len;

	_ASSERT(obj != NULL);
	_ASSERT(name != NULL);

	len = strlen(name);
	_ASSERT(len != 0);

	arc = (archive_t *)obj;

	n = (*arc->calc_hash)(name, arc->hash_num);
	if (arc->table_count[n] == 0)
		return 0;

	for (p = arc->table[n]; p != NULL; p = p->next) {
		if (strncasecmp(p->name, name, len) == 0) {
			if (p->access)
				return 1;
			return 0;
		}
	}
	return 0;
}

void
archive_print(object_t *obj)
{
	archive_t *arc;
	arcfile_t *p;
	int i;

	_ASSERT(obj != NULL);

	arc = (archive_t *)obj;

	for (i = 0; i < arc->fd_num; i++)
		printf("archive filename = %s\n", arc->fd[i]->name);

	for (i = 0; i < arc->hash_num; i++) {
		if (arc->table_count[i] != 0) {
#if 0
			printf(" index = %d, num = %d\n",
			    i, arc->table_count[i]);
#endif
			for (p = arc->table[i]; p != NULL; p = p->next) {
				printf(" name = \"%-12s\", offset = 0x%08lx, "
				    "size = 0x%08x, type = %d\n",
				    p->name, p->offset, p->size, p->obj.type);
			}
		}
	}
	printf("\n");
}

int
archive_extract(object_t *obj, unsigned char *name)
{
	arcquery_t aq;
	FILE *ofp;
	size_t s;

	_ASSERT(obj != NULL);
	_ASSERT(obj->type == OBJECT_ARCHIVE);
	_ASSERT(name != NULL);

	if (!archive_query(obj, name, &aq)) {
		fprintf(stderr, "archive_extract: Can't find '%s'\n", name);
		return 1;
	}

	ofp = fopen(name, "wb");
	if (ofp == NULL) {
		fprintf(stderr, "archive_extract: Can't create '%s'\n", name);
		return 1;
	}

	fseek(aq.fp, aq.offset, SEEK_SET);
	for (s = 0; s < aq.size; s++) {
		fputc(fgetc(aq.fp), ofp);
	}

	fclose(ofp);

	return 0;
}

int
save_fchklog(object_t *obj)
{
	unsigned char buf[32];
	archive_t *arc;
	arcfile_t *p;
	FILE *fp;
	size_t s;
	int count;
	int i;

	_ASSERT(obj != NULL);
	_ASSERT(obj->type == OBJECT_ARCHIVE);

	if (!is_fchklog)
		return 0;

	arc = (archive_t *)obj;

	fp = fopen(nscrflog_filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "save: can't open %s\n", nscrflog_filename);
		return 1;
	}

	/* 個数書き込み */
	/* 分離するのも何だけど、後から個数書くのは面倒だし:-P */
	for (count = 0, i = 0; i < arc->hash_num; i++) {
		for (p = arc->table[i]; p != NULL; p = p->next) {
			if (p->access)
				count++;
		}
	}
	snprintf(buf, sizeof(buf), "%d", count);
	for (s = 0; s < strlen(buf); s++)
		put_byte(fp, buf[s]);
	put_byte(fp, 0x0a);

	/* ファイル名書き込み */
	for (i = 0; i < arc->hash_num; i++) {
		for (p = arc->table[i]; p != NULL; p = p->next) {
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
load_fchklog(object_t *obj)
{
	arcquery_t aq;
	archive_t *arc;
	FILE *fp;
	unsigned char *buf;
	size_t i, bufsize;
	int ch;
	int c, count;

	_ASSERT(obj != NULL);
	_ASSERT(obj->type == OBJECT_ARCHIVE);

	if (!is_fchklog)
		return 0;

	arc = (archive_t *)obj;

	fp = fopen(nscrflog_filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "load: can't open %s\n", nscrflog_filename);
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

		bzero(buf, bufsize);
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

		if (!archive_query(obj, buf, &aq)) {
			fprintf(stderr, "can't find image '%s'.\n", buf);
		}
	}

	fclose(fp);

	return 0;
}
