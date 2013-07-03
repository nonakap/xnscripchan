/*	$Id: prescan.c,v 1.9 2002/01/23 16:38:58 nonaka Exp $	*/

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

#define	NNAMEBUF	32
#define	NLINEBUF	100

long *linepos;
long scan_lineno = 1;

void
prescan(FILE *fp)
{
	unsigned char *p;
	int is_top;		/* line top? */
	int is_scan;		/* scanning? */
	int maxlen;		/* malloced length */
	int len;		/* used length */
	int c;
	int maxlinenum;

	_ASSERT(fp != NULL);

	fseek(fp, 0L, SEEK_SET);

	p = NULL;
	is_top = 1;
	is_scan = 0;
	maxlen = 0;
	len = 0;
	maxlinenum = NLINEBUF;
	linepos = (long *)Emalloc(sizeof(long) * maxlinenum);
	linepos[0] = linepos[1] = ftell(fp);

	while ((c = fgetc(fp)) != EOF) {
		if (is_encoded)
			c ^= 0x84;
		if (c == '\n') {
			is_top = 1;
			scan_lineno++;

			if (is_scan) {
				_ASSERT(p != NULL);
				_ASSERT(len != 0);

				/*
				 * ラベル自体が存在する行では無くて、
				 * その次の行にジャンプする
				 */
				p[len] = '\0';
				label_add(p, scan_lineno, ftell(fp));
				Efree(p);

				p = NULL;
				len = 0;
				maxlen = 0;
				is_scan = 0;
			}

			if (maxlinenum - scan_lineno <= 1) {
				maxlinenum += NLINEBUF;
				linepos = (long *)Erealloc(linepos,
				    sizeof(long) * maxlinenum);
			}
			linepos[scan_lineno] = ftell(fp);
		} else if (is_top) {
			if (c == ' ' || c == '\t') {
				/* うがぁ */
				continue;
			} else if (c == '*') {
				len = 0;
				is_scan = 1;

				/* XXX: 銀色 - 完全版 - lineno: 97949 */
				/* 空白文字を捨てる */
				while ((c = fgetc(fp)) != EOF) {
					if (is_encoded)
						c ^= 0x84;
					if (c == ' ' || c == '\t')
						continue;
					break;
				}
				if (c == EOF)
					break;

				/* 文字をストリームバッファに返却 */
				if (is_encoded)
					c ^= 0x84;
				ungetc(c, fp);
			} else if (c == '~') {
				/* 行末までの文字を捨てる */
				while ((c = fgetc(fp)) != EOF) {
					if (is_encoded)
						c ^= 0x84;
					if (c == '\n')
						break;
				}
				if (c == EOF)
					break;

				/*
				 * ラベル自体が存在する行では無くて、
				 * その次の行にジャンプする
				 */
				tilde_add(scan_lineno + 1, ftell(fp));

				/* '\n' をストリームバッファに返却 */
				if (is_encoded)
					c ^= 0x84;
				ungetc(c, fp);
			}
			is_top = 0;
		} else if (is_scan) {
			/*
			 * 文末に無駄なスペース、タブがあった場合は
			 * 行末までの文字を捨てる
			 */
			if (c == ' ' || c == '\t') {
				while ((c = fgetc(fp)) != EOF) {
					if (is_encoded)
						c ^= 0x84;
					if (c == '\n')
						break;
				}
				if (c == EOF)
					break;

				/* '\n' をストリームバッファに返却 */
				if (is_encoded)
					c ^= 0x84;
				ungetc(c, fp);
			} else {
				if (maxlen - len <= 1) {
					maxlen += NNAMEBUF;
					p = (unsigned char *)Erealloc(p,maxlen);
				}
				p[len++] = c;
			}
		}
	}

	/*
	 * "*kma[EOF]" な場合？
	 */
	if (p)
		Efree(p);
}

int
screenmode_set(FILE *fp)
{
	const char modename[] = ";mode800";
	unsigned char *linebuf;
	int n, len;
	int ch;

	_ASSERT(fp != NULL);

	fseek(fp, 0L, SEEK_SET);
	for (n = 0; (ch = fgetc(fp)) != EOF; n++) {
		if (is_encoded)
			ch ^= 0x84;
		if (ch == '\n')
			break;
	}

	len = n;
	linebuf = (unsigned char *)Emalloc(len + 1);
	_ASSERT(linebuf != NULL);

	fseek(fp, 0L, SEEK_SET);
	for (n = 0; (ch = fgetc(fp)) != EOF && n < len; n++) {
		if (is_encoded)
			ch ^= 0x84;
		if (ch == '\n')
			break;
		linebuf[n] = ch;
	}
	linebuf[n] = '\0';

	for (n = 0; n < len; n++) {
		if (linebuf[n] == ';') {
			if (strcasecmp(&linebuf[n], modename) == 0) {
				break;
			}
		}
	}
	if (n < len) {
		printf("screen mode = 800x600\n");
		SCREEN_MODE = SCREEN_MODE_800x600;
		SCREEN_WIDTH = 800;
		SCREEN_HEIGHT = 600;
	} else if (SCREEN_MODE == SCREEN_MODE_320x240) {
		printf("screen mode = 320x240\n");
		SCREEN_MODE = SCREEN_MODE_320x240;
		SCREEN_WIDTH = 320;
		SCREEN_HEIGHT = 240;
	} else {
		SCREEN_MODE = SCREEN_MODE_DEFAULT;
		SCREEN_WIDTH = 640;
		SCREEN_HEIGHT = 480;
	}

	return 0;
}
