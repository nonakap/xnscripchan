/*	$Id: sar.c,v 1.10 2002/01/10 17:25:32 nonaka Exp $	*/

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

int
sar_open(object_t *obj, int fd_idx)
{
	archive_t *arc;
	arcfile_t **afpp, **p;
	FILE *fp;
	int base, count;
	int bufsize;
	int c, n;
	int i, j;

	_ASSERT(obj != NULL);
	_ASSERT(obj->type == OBJECT_ARCHIVE);
	_ASSERT(fd_idx >= 0);

	arc = (archive_t *)obj;
	fp = arc->fd[fd_idx]->fp;
	count = get_be16(fp);
	base = get_be32(fp);

	_ASSERT(count > 0);
	_ASSERT(base >= 0);

	/* make offset/size table */
	afpp = (arcfile_t **)Ecalloc(count, sizeof(arcfile_t *));
	for (i = 0; i < count; i++) {
		afpp[i] = (arcfile_t *)Ecalloc(1, sizeof(arcfile_t));

		bufsize = 16;
		afpp[i]->name = (unsigned char *)Emalloc(bufsize);
		for (j = 0; (c = get_byte(fp)) != '\0'; j++) {
			if (j + 1 >= bufsize) {
				unsigned char *tmp;

				bufsize += 16;
				tmp = (unsigned char *)Erealloc(afpp[i]->name,
				    bufsize);
				_ASSERT(tmp != NULL);
				afpp[i]->name = tmp;
			}
			if (islower(c))
				c = toupper(c);
			afpp[i]->name[j] = c;
		}
		afpp[i]->name[j] = '\0';

		afpp[i]->fp = &arc->fd[fd_idx]->fp;
		afpp[i]->offset = get_be32(fp) + base;
		afpp[i]->size = get_be32(fp);
		afpp[i]->decode_size = afpp[i]->size;
		afpp[i]->access = 0;

		n = (*arc->calc_hash)(afpp[i]->name, arc->hash_num);
		arc->table_count[n]++;

		p = &arc->table[n];
		if (*p != NULL)
			(*p)->prev = afpp[i];
		afpp[i]->next = *p;
		*p = afpp[i];
	}

	return 0;
}
