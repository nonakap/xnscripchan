/*	$Id: lzss.c,v 1.3 2002/01/11 17:02:47 nonaka Exp $	*/

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
#include "bitop.h"

#define	RINGBUF_SIZE	256	/* リングバッファサイズ */
#define	LEN		17	/* 最大一致長 */

/*
 * NSA アーカイブの変形 LZSS 展開ルーチン (is_compress == 2)
 */
int
lzss_decode(FILE *fp, unsigned char *p, size_t encode_size, size_t orignal_size)
{
	unsigned char ringbuf[RINGBUF_SIZE];
	unsigned char *top;
	bitop_t b, *bp;
	int flag;
	int offset, count;
	int bufpos;
	int ch;
	int i, d;

	_ASSERT(fp != NULL);
	_ASSERT(p != NULL);
	_ASSERT(encode_size > 0);
	_ASSERT(orignal_size > 0);

	bitop_init(bp = &b, fp, encode_size);
	top = p;

	bzero(ringbuf, RINGBUF_SIZE - LEN);
	bufpos = RINGBUF_SIZE - LEN;

	while (p < top + orignal_size) {
		flag = bitop_get(bp);
		if (flag) {
			ch = bitop_getn(bp, 8);
			ringbuf[bufpos++] = ch;
			if (bufpos >= RINGBUF_SIZE)
				bufpos = 0;
			*p++ = ch;
			if (p >= top + orignal_size)
				break;
		} else {
			offset = bitop_getn(bp, 8);
			count = bitop_getn(bp, 4) + 2;

			for (i = 0; i < count; i++) {
				d = offset + i;
				if (d >= RINGBUF_SIZE)
					d -= RINGBUF_SIZE;
				ch = ringbuf[d];

				ringbuf[bufpos++] = ch;
				if (bufpos >= RINGBUF_SIZE)
					bufpos = 0;

				*p++ = ch;
				if (p >= top + orignal_size)
					break;
			}
		}
	}

	return 0;
}
