/*	$Id: bitop.c,v 1.1 2002/01/09 18:14:34 nonaka Exp $	*/

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

void
bitop_init(bitop_t *b, FILE *fp, size_t size)
{

	_ASSERT(b != NULL);
	_ASSERT(fp != NULL);
	_ASSERT(size != 0);

	b->fp = fp;
	b->size = --size;
	b->mask = 0x100;
	b->v = get_byte(fp);
}

int
bitop_get(bitop_t *b)
{

	_ASSERT(b != NULL);

	b->mask >>= 1;
	if (b->mask == 0x00) {
		_ASSERT(b->size != 0);
		b->size--;
		b->v = get_byte(b->fp);
		b->mask = 0x80;
	}
	return (b->v & b->mask) ? 1 : 0;
}

unsigned int
bitop_getn(bitop_t *b, int n)
{
	unsigned int v;

	_ASSERT(b != NULL);
	_ASSERT(n > 0 && n <= 32);

	for (v = 0; n > 0; n--) {
		v <<= 1;
		if (bitop_get(b))
			v |= 1;
	}
	return v;
}
