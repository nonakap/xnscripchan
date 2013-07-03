/*	$Id: nbz.c,v 1.1 2002/01/15 17:43:07 nonaka Exp $	*/

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

#include <stdio.h>
#include <bzlib.h>

#include "nscr.h"

/*
 * NBZ
 *
 * +00h : DWORD(BE32) orignal size
 * +04h-: bzip -1 orignal_file
 */
unsigned char *
nbz_decode(FILE *fp, long offset, size_t encode_size, size_t *decode_size)
{
	unsigned char unused[BZ_MAX_UNUSED];
	BZFILE *bzf;
	unsigned char *top, *p;
	unsigned char *unused_tmp;
	int bzerr;
	int nread;
	int nunused;

	_ASSERT(fp != NULL);
	_ASSERT(offset >= 0);
	_ASSERT(encode_size > 0);

	fseek(fp, offset, SEEK_SET);
	*decode_size = get_be32(fp);
	_ASSERT(*decode_size > 0);

	p = top = (unsigned char *)Emalloc(*decode_size);

	nunused = 0;
	bzf = BZ2_bzReadOpen(&bzerr, fp, 0, 0, unused, nunused);
	_ASSERT(bzf != NULL && bzerr == BZ_OK);

	while (bzerr == BZ_OK) {
		_ASSERT(p < top + *decode_size);
		nread = BZ2_bzRead(&bzerr, bzf, p, 5000);
		_ASSERT((bzerr == BZ_OK||bzerr == BZ_STREAM_END) && nread > 0);
		p += nread;
	}
	_ASSERT(bzerr == BZ_STREAM_END);

	BZ2_bzReadGetUnused(&bzerr, bzf, (void **)&unused_tmp, &nunused);
	_ASSERT(bzerr == BZ_OK);

	BZ2_bzReadClose(&bzerr, bzf);
	_ASSERT(bzerr == BZ_OK);

	_ASSERT(nunused == 0);

	return top;
}
