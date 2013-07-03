/*	$Id: select.c,v 1.11 2002/01/18 18:23:39 nonaka Exp $	*/

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

typedef struct selection_tag {
	int no;
	char *msg;
	size_t msglen;
	char *label;
	rect_t rect;
} selection_t;

static int nselect = 0;
static selection_t *select_top = NULL;

static int select_kind = -1;
static long sel_var = -1;
static int prev = -1;
static int col;

void
select_init(int n)
{

	_ASSERT(n > 0);

	select_top = (selection_t *)Emalloc(sizeof(selection_t) * n);
	nselect = n;
	select_kind = -1;
	sel_var = -1;
	prev = -1;
	col = CTEXT->cur_ty;
}

void
select_destroy(void)
{

	if (select_top) {
		Efree(select_top);
		select_top = NULL;
	}
	nselect = 0;
	select_kind = -1;
	sel_var = -1;
	prev = -1;
	col = 0;
}

void
select_add(int no, char *msg, char *label)
{
	selection_t *t;

	_ASSERT(no >= 0 && no < nselect);

	t = &select_top[no];
	t->no = no;
	t->msg = Estrdup(msg);
	t->msglen = strlen(msg);
	t->label = label ? Estrdup(label) : NULL;

	text_setrect(&t->rect, &col, t->msg, t->msglen);
	_ASSERT(col <= CTEXT->max_ty);
}

void
select_start(int kind, long v)
{

	select_kind = kind;
	sel_var = v;
	select_redraw();
}

void
select_do(int btnno, point_t *point)
{
	selection_t *p;
	reg_t t;
	int i;

	switch (btnno) {
	case 0:
		for (p = select_top, i = 0; i < nselect; i++) {
			if (in_rect(&p[i].rect, point))
				break;
		}
		break;

	default:
		fprintf(stderr, "select_do: btnno = %d\n", btnno);
		return;
	}
	if (i == nselect) {
		return;
	}

	switch (select_kind) {
	case SELECT_SELECT:
		DSNAP(("select_do: goto label = %s\n", p[i].label));
		label_jump(p[i].label + 1);
		break;

	case SELECT_SELGOSUB:
		t.type = TOKEN_OFFSET;
		t.u.val = flex_get_pos();
		push(&t);

		t.type = TOKEN_LINENO;
		t.u.val = lineno;
		push(&t);

		DSNAP(("select_do: gosub label = %s\n", p[i].label));
		label_jump(p[i].label + 1);
		break;

	case SELECT_SELNUM:
		varnum_set(sel_var, p[i].no);
		break;

	default:
		_ASSERT(0);
		break;
	}

	select_destroy();

	newpage();
	state = STATE_COMMAND;
}

void
select_move(point_t *point)
{
	selection_t *p;
	int i;

	for (p = select_top, i = 0; i < nselect; i++) {
		if (in_rect(&p[i].rect, point)) {
			break;
		}
	}
	if (i == nselect) {
		i = -1;
	}

	if (i != prev) {
		prev = i;
		select_redraw();
	}
}

void
select_redraw(void)
{
	selection_t *p;
	int i;

	for (p = select_top, i = 0; i < nselect; i++) {
		if (i == prev) {
			display_selection(p[i].rect.left, p[i].rect.top,
			    p[i].msg, 1);
		} else {
			display_selection(p[i].rect.left, p[i].rect.top,
			    p[i].msg, 0);
		}
	}
}

void
select_dump(void)
{
	selection_t *p;
	int i;

	for (i = 0; i < nselect; i++) {
		p = &select_top[i];
		printf("\t%d: %s\n", p->no + 1, p->msg);
	}
}
