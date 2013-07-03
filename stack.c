/*	$Id: stack.c,v 1.9 2002/01/08 18:13:27 nonaka Exp $	*/

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

#define	NSTACK	1000

STATIC reg_t stack[NSTACK];
STATIC int max_stack;
STATIC int nstack;

static void dump_stack(int);

void
init_stack(void)
{

	nstack = 0;
}

void
push(reg_t *p)
{

	_ASSERT(nstack < NSTACK);

	stack[nstack++] = *p;
	if (nstack > max_stack)
		max_stack = nstack;
}

reg_t
pop(void)
{

	_ASSERT(nstack >= 0);

	return stack[--nstack];
}

void
stack_show(void)
{

	printf("lineno = %d\n", lineno);
	printf("nstack = %d\n", nstack);
	printf("max_stack = %d\n", max_stack);

	dump_stack(nstack);
}

static void
dump_stack(int n)
{
	int i;

	for (i = 0; i < n; i++) {
		printf("No: %d\n", i);
		printf("type = %d\n", stack[i].type);
		switch (stack[i].type) {
		case TOKEN_STRING:
			printf("str = %s\n", stack[i].u.str);
			break;

		default:
			printf("val = 0x%0lx\n", stack[i].u.val);
			break;
		}
	}
}
