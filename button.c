/*	$Id: button.c,v 1.10 2002/01/08 18:13:27 nonaka Exp $	*/

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

typedef struct button_tag {
	long no;
	rect_t sel_rect;
	rect_t draw_rect;
	struct button_tag *next;
} button_t;

static long btn_var = -1;
static int prev = -1;

static char *button_name = NULL;
static long nbutton = 0;
static button_t *button_top = NULL;

static image_t *btn_image;

void
button_init(unsigned char *name)
{

	button_destroy();

	button_name = Estrdup(name);
	if (strlen(button_name) != 0)
		btn_image = image_open(name, TRANSMODE_COPY);
}

void
button_destroy(void)
{

	if (button_name) {
		Efree(button_name);
		button_name = NULL;
	}

	if (btn_image) {
		img_destroy(btn_image);
		btn_image = NULL;
	}

	button_clear();
}

void
button_clear(void)
{
	button_t *p, *next;

	for (p = button_top; p != NULL; p = next) {
		next = p->next;
		Efree(p);
	}
	button_top = NULL;
	nbutton = 0;
	btn_var = -1;
	prev = -1;
}

void
button_add(long no, rect_t *sel_rect, rect_t *draw_rect)
{
	button_t **p, *t;

	t = (button_t *)Emalloc(sizeof(button_t));
	t->sel_rect = *sel_rect;
	t->draw_rect = *draw_rect;
	t->no = no;
	t->next = NULL;

	for (p = &button_top; *p != NULL; p = &((*p)->next)) {
		_ASSERT((*p)->no != t->no);
	}
	*p = t;
}

void
button_start(long v)
{

	btn_var = v;
}

void
button_do(long btnno, point_t *point)
{
	button_t *p;

	_ASSERT(button_name != NULL);
	_ASSERT(button_top != NULL);

	switch (btnno) {
	case 0:
		for (p = button_top; p != NULL; p = p->next) {
			if (in_rect(&p->sel_rect, point)) {
				varnum_set(btn_var, p->no);
				break;
			}
		}
		if (p == NULL) {
			varnum_set(btn_var, 0);
			break;
		}
		break;

	case 1:
		varnum_set(btn_var, -1);
		break;

	default:
		_ASSERT(0);
		break;
	}
	state = STATE_COMMAND;
}

void
button_move(point_t *point)
{
	button_t *p;
	int curr = prev;

	for (p = button_top; p != NULL; p = p->next) {
		if (in_rect(&p->sel_rect, point)) {
			curr = p->no;
			break;
		}
	}
	if (p == NULL)
		curr = -1;

	if (curr != prev) {
		redraw(1);
		if (curr != -1) {
			sys_copy_area(btn_image,
			    p->draw_rect.left, p->draw_rect.top,
			    p->sel_rect.width, p->sel_rect.height,
			    p->sel_rect.left, p->sel_rect.top);
		}
		prev = curr;
	}
}

void
button_dump(void)
{
	button_t *p;

	if (button_name == NULL)
		return;

	printf("button_name = %s\n", button_name);
	for (p = button_top; p != NULL; p = p->next) {
		printf("\t%ld: (%d, %d)-(%d, %d)/(%d, %d)-(%d, %d)\n",
		    p->no,
		    p->sel_rect.left, p->sel_rect.top,
		    p->sel_rect.left + p->sel_rect.width,
		    p->sel_rect.top + p->sel_rect.height,
		    p->draw_rect.left, p->draw_rect.top,
		    p->draw_rect.left + p->draw_rect.width,
		    p->draw_rect.top + p->draw_rect.height);
	}
}
