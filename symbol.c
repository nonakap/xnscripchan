/*	$Id: symbol.c,v 1.9 2002/01/18 18:23:39 nonaka Exp $	*/

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
#include "symbol.h"

static void symbol_print(symbol_t *sym);

symbol_t *
symbol_lookup(object_t *obj, char *sym)
{
	symbol_t *p;

	for (p = (symbol_t *)obj; p != NULL; p = p->next)
		if (strcasecmp(sym, p->sym) == 0)
			break;
	return p;
}

void
symbol_show(object_t *obj)
{
	symbol_t *p;

	for (p = (symbol_t *)obj; p != NULL; p = p->next)
		symbol_print(p);
}

static void
symbol_print(symbol_t *p)
{

	_ASSERT(p != NULL);

	printf("type = %d\n", p->type);
	printf("sym = %s\n", p->sym);
	if (p->type == SYMBOL_NUMALIAS)
		printf("val = 0x%lx\n", p->u.val);
	else if (p->type == SYMBOL_STRALIAS)
		printf("str = %s\n", p->u.str);
	else
		printf("unknown = 0x%08lx\n", p->u.val);
}

void
symbol_destroy(object_t *obj)
{
	symbol_t *p, *next;

	_ASSERT(obj != NULL);

	for (p = (symbol_t *)obj; p != NULL; p = next) {
		next = p->next;
		Efree(p);
	}
}

int
symbol_add(object_t **obj, char *sym, void *arg, int type)
{
	symbol_t *p, **q;

	_ASSERT(obj != NULL);
	_ASSERT(sym != NULL);

	p = (symbol_t *)Emalloc(sizeof(symbol_t));
	p->next = NULL;
	p->type = type;
	p->sym = Estrdup(sym);
	p->u.ptr = arg;

	for (q = (symbol_t **)obj; *q != NULL; q = &((*q)->next))
		continue;
	*q = p;

	return 0;
}
