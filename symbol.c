/*	$Id: symbol.c,v 1.8 2002/01/08 18:13:27 nonaka Exp $	*/

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
static int symbol_add(char *sym, int type, void *arg);

static symbol_t *symbol_list = NULL;

symbol_t *
symbol_lookup(char *sym)
{
	symbol_t *p;

	for (p = symbol_list; p != NULL; p = p->next)
		if (strcasecmp(sym, p->sym) == 0)
			break;
	return p;
}

void
symbol_show(void)
{
	symbol_t *p;

	for (p = symbol_list; p != NULL; p = p->next)
		symbol_print(p);
}

static void
symbol_print(symbol_t *p)
{

	printf("type = %d\n", p->type);
	printf("sym = %s\n", p->sym);
	if (p->type == SYMBOL_NUMALIAS)
		printf("val = 0x%lx\n", p->u.val);
	else if (p->type == SYMBOL_STRALIAS)
		printf("str = %s\n", p->u.str);
	else
		printf("unknown = 0x%08lx\n", p->u.val);
}

int
symbol_add_numalias(char *sym, long num)
{

	symbol_add(sym, SYMBOL_NUMALIAS, (void *)num);

	return 0;
}

int
symbol_add_stralias(char *sym, void *str)
{

	symbol_add(sym, SYMBOL_STRALIAS, str);

	return 0;
}

int
symbol_add_command(char *sym, void *str)
{

	symbol_add(sym, SYMBOL_COMMAND, str);

	return 0;
}

static int
symbol_add(char *sym, int type, void *arg)
{
	symbol_t *p, **q;

	p = (symbol_t *)Emalloc(sizeof(symbol_t));
	p->next = NULL;
	p->type = type;
	p->sym = Estrdup(sym);

	switch (type) {
	case SYMBOL_NUMALIAS:
		p->u.val = (long)arg;
		break;

	case SYMBOL_STRALIAS:
		p->u.str = (char *)arg;
		break;

	case SYMBOL_COMMAND:
		p->u.func = (int (*)(reg_t *, long))arg;
		break;

	default:
		_ASSERT(0);
		return 1;
	}

	for (q = &symbol_list; *q != NULL; q = &((*q)->next))
		continue;
	*q = p;

	return 0;
}
