/*	$Id: display.c,v 1.17 2002/01/18 19:36:51 nonaka Exp $	*/

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

static void draw_putchar(long *, long *, long *, long *, unsigned short, long);

void
text_setwindow(long *v, long n, unsigned char *filename)
{
	rect_t rect;

	_ASSERT(n >= 14);

	CTEXT->cur_tx = CTEXT->cur_ty = 0;
	CTEXT->curx = CTEXT->cur_left = v[0];
	CTEXT->cury = CTEXT->cur_top = v[1];
	CTEXT->max_tx = v[2];
	CTEXT->max_ty = v[3];
	CFONT->width = v[4];
	CFONT->height = v[5];
	CFONT->pitch_x = v[6];
	CFONT->pitch_y = v[7];
	CFONT->speed = v[8];
	CFONT->is_bold = v[9] ? 1 : 0;
	CFONT->is_shadow = v[10] ? 1 : 0;

	_ASSERT(CTEXT->max_tx > 0);
	_ASSERT(CTEXT->max_ty > 0);

	CTEXT->cache_size = (CTEXT->max_tx + 1) * CTEXT->max_ty;
	if (CTEXT->cache_data)
		Efree(CTEXT->cache_data);
	CTEXT->cache_data = (unsigned short *)
	    Ecalloc(sizeof(unsigned short), CTEXT->cache_size);
	CTEXT->cache_attr =
	    (long *)Ecalloc(sizeof(long), CTEXT->cache_size);

	CMSG->size = CTEXT->max_tx * CTEXT->max_ty * 16;
	if (CMSG->data)
		Efree(CMSG->data);
	CMSG->data = (unsigned short *)
	    Ecalloc(sizeof(unsigned short), CMSG->size);
	CMSG->pos = CMSG->remain = 0;

	bzero(&rect, sizeof(rect));
	rect.left = v[12];
	rect.top = v[13];
	if (filename == NULL) {
		_ASSERT(n >= 16);
		rect.width = v[14] + 1;
		rect.height = v[15] + 1;
	}

	font_init(CFONT->width, CFONT->height, CFONT->is_bold);
	screen_setwindow(filename, v[11], &rect);
}

void
text_set_clickstr(unsigned char *str, size_t num, long column)
{
	size_t s;

	if (num <= 0)
		return;

	CCLICKSTR->column = column;
	CCLICKSTR->num = num;
	for (s = 0; s < num; s++)
		CCLICKSTR->data[s] = str[s * 2] | str[s * 2 + 1];
}

void
text_set_locate(long x, long y)
{

	_ASSERT(x >= 0 && x < CTEXT->max_tx);
	_ASSERT(y >= 0 && y < CTEXT->max_ty);

	CTEXT->cur_tx = x;
	CTEXT->cur_ty = y;
	CTEXT->curx = CTEXT->cur_left + (CFONT->width + CFONT->pitch_x) * x;
	CTEXT->curx = CTEXT->cur_top + (CFONT->height + CFONT->pitch_y) * y;
}

void
text_setrect(rect_t *rect, int *col, unsigned char *msg, size_t msglen)
{
	int ncolumn;

	_ASSERT(rect != NULL);
	_ASSERT(*col >= 0);
	_ASSERT(msg != NULL);
	_ASSERT(msglen > 0);

	ncolumn = (((msglen + 1) / 2) / (CTEXT->max_tx + 1)) + 1;
	rect->left = CTEXT->cur_left;
	rect->top = CTEXT->cur_top + (CFONT->height + CFONT->pitch_y) * *col;
	rect->width = (CFONT->width + CFONT->pitch_x) *
	    ((ncolumn == 1) ? (msglen / 2) : (CTEXT->max_tx + 1));
	rect->height = (CFONT->height + CFONT->pitch_y) * ncolumn;

	*col += ncolumn;
}

void
newpage(void)
{

	CTEXT->cur_tx = CTEXT->cur_ty = 0;
	CTEXT->curx = CTEXT->cur_left;
	CTEXT->cury = CTEXT->cur_top;
	CTEXT->cache_len = 0;

	bzero(CTEXT->cache_data, CTEXT->cache_size * sizeof(unsigned short));
	bzero(CTEXT->cache_attr, CTEXT->cache_size * sizeof(long));

	redraw(&CSCREEN->window_pos);
}

void
display_selection(long x, long y, unsigned char *msg, int selected)
{
	size_t i, len;
	long tx, ty;
	unsigned short c;

	_ASSERT(msg != NULL);
	_ASSERT(x >= CTEXT->cur_left);
	_ASSERT(y >= CTEXT->cur_top);

	tx = (x - CTEXT->cur_left) / (CFONT->width + CFONT->pitch_x);
	ty = (y - CTEXT->cur_top) / (CFONT->height + CFONT->pitch_y);

	len = strlen(msg);
	for (i = 0; i < len; ) {
		if (msg[i] & 0x80) {
			c = sjis2euc(msg[i] << 8 | msg[i+1]);
			i += 2;
			draw_putchar(&tx, &ty, &x, &y, c,
			    core->selection.color[selected]);
		} else {
			i++;
		}
	}
}

/*
 * 通常表示
 */
void
display_message(unsigned char *msg, size_t len)
{
	size_t i, pos;
	unsigned short ch;

	for (i = 0; i < len; ) {
		if (msg[i] & 0x80) {
			_ASSERT(i + 1 < len);
			ch = sjis2euc(msg[i] << 8 | msg[i+1]);
			i += 2;
		} else {
			ch = msg[i++];
		}

		_ASSERT(CMSG->remain < CMSG->size);	/* XXX: realloc */
		pos = CMSG->pos + CMSG->remain++;
		if (pos >= CMSG->size)
			pos -= CMSG->size;
		CMSG->data[pos] = ch;
	}
}

#define	CMSGDEC() \
{ \
	if (CMSG->remain > 0) { \
		if (++CMSG->pos >= CMSG->size) \
			CMSG->pos -= CMSG->size; \
		CMSG->remain--; \
	} else { \
		break; \
	} \
}

int
display_string(void)
{
	int next_state = STATE_COMMAND;
	int is_clickwait = 1;
	size_t i;
	long v;
	unsigned short ch;

	if (!CMSG->is_show_window) {
		CMSG->is_show_window = 1;
		screen_show_text_window();
	}

	while (CMSG->remain > 0) {
		ch = CMSG->data[CMSG->pos];
		CMSGDEC();

		if (ch & 0x8000) {
			draw_putchar(&CTEXT->cur_tx, &CTEXT->cur_ty,
			     &CTEXT->curx, &CTEXT->cury, ch, CTEXT->color);
			if (CMSG->is_clickwait) {
				for (i = 0; i < CCLICKSTR->num; i++) {
					if (ch == CCLICKSTR->data[i]) {
						next_state = (CCLICKSTR->column + CTEXT->cur_ty < CTEXT->max_ty) ? STATE_CLICKWAIT : STATE_CLICKPAGEWAIT;
						break;
					}
				}
			}
		} else {
			switch (ch) {
			case ' ':
			case '\t':
				break;

			case '@':
				next_state = STATE_CLICKWAIT;
				break;

			case '\\':
				if (CMSG->remain > 0
				    && CMSG->data[CMSG->pos] == '\n') {
					/* skip '\n' */
					CMSGDEC();
				}
				next_state = STATE_CLICKPAGEWAIT;
				break;

			case '/':
				if (CMSG->remain > 0
				    && CMSG->data[CMSG->pos] == '\n') {
					/* skip '\n' */
					CMSGDEC();
				} else {
					draw_putchar(&CTEXT->cur_tx,
					    &CTEXT->cur_ty,
					    &CTEXT->curx, &CTEXT->cury,
					    ch, CTEXT->color);
				}
				break;

			case '!':
				if (CMSG->remain <= 0) {
					draw_putchar(&CTEXT->cur_tx,
					    &CTEXT->cur_ty,
					    &CTEXT->curx, &CTEXT->cury,
					    0xa1aa /*！*/, CTEXT->color);
					break;
				}

				if (CMSG->data[CMSG->pos] == '\n') {
					CMSGDEC();
					draw_putchar(&CTEXT->cur_tx,
					    &CTEXT->cur_ty,
					    &CTEXT->curx, &CTEXT->cury,
					    0xa1aa /*！*/, CTEXT->color);
					break;
				}

				switch (CMSG->data[CMSG->pos]) {
				case 'd':
				case 'w':
					CMSGDEC();

					if (CMSG->data[CMSG->pos] == 'd')
						cancel_timer = 1;
					else
						cancel_timer = 0;

					v = 0;
					while (CMSG->remain > 0) {
						if (!isdigit(CMSG->data[CMSG->pos]))
							break;
						v *= 10;
						v = CMSG->data[CMSG->pos] - '0';
						CMSGDEC();
					}
					DSNAP(("waittimer = %d\n", v));

				        waittimer = v;
				        timer_set();
					next_state = STATE_WAITTIMER;
					break;

				case 's':
					CMSGDEC();
					if (isdigit(CMSG->data[CMSG->pos])) {
						v = CMSG->data[CMSG->pos] - '0';
						CMSGDEC();

						while (CMSG->remain > 0) {
							if (!isdigit(CMSG->data[CMSG->pos]))
								break;
							v *= 10;
							v = CMSG->data[CMSG->pos] - '0';
							CMSGDEC();
						}
						/* XXX */
					} else if (CMSG->data[CMSG->pos]=='d') {
						/* XXX: !sd */
						CMSGDEC();
					}
					break;
				}
				break;

			case '#': {
				unsigned char tmp[7];
				long v;

				if (CMSG->remain < 6) {
					CMSG->remain = 0;	/* XXX */
					break;
				}

				for (i = 0; i < 6; i++) {
					tmp[i] = CMSG->data[CMSG->pos];
					CMSGDEC();
				}
				tmp[6] = '\0';
				v = strtol(tmp, NULL, 16);
				DSNAP(("color = 0x%06x\n", v));
				sys_alloc_color(v, &CTEXT->color);
				}
				break;

			case '_':
				is_clickwait = 0;
				break;

			case '\n':
			default:
				draw_putchar(&CTEXT->cur_tx, &CTEXT->cur_ty,
				    &CTEXT->curx, &CTEXT->cury, ch,
				    CTEXT->color);
				break;
			}
			if (next_state != STATE_COMMAND)
				break;
		}
		if (!is_clickwait) {
			CMSG->is_clickwait = 0;
			is_clickwait = 1;
		} else if (!CMSG->is_clickwait) {
			CMSG->is_clickwait = 1;
		}
	}

	return next_state;
}

static void
draw_putchar(long *tx, long *ty, long *px, long *py,
     unsigned short ch, long color)
{

	if (ch == '\n') {
		*tx = 0;
		(*ty)++;
		*px = CTEXT->cur_left;
		*py += CFONT->height + CFONT->pitch_y;
		return;
	}

	if (*tx > CTEXT->max_tx - 1) {
		if ((*tx == CTEXT->max_tx)
		    && (ch == 0xa1a2 || ch == 0xa1a3 || ch == 0xa1d7)) {
			/* Nothing to do */;
		} else {
			*tx = 0;
			(*ty)++;
			*px = CTEXT->cur_left;
			*py += CFONT->height + CFONT->pitch_y;
		}
	}

	CTEXT->cache_data[*tx + *ty * (CTEXT->max_tx + 1)] = ch;
	CTEXT->cache_attr[*tx + *ty * (CTEXT->max_tx + 1)] = color;
	CTEXT->cache_len++;

	sys_draw_char(*px, *py, ch, color);

	(*tx)++;
	*px += CFONT->width + CFONT->pitch_x;
}

void
text_redraw(void)
{
	long color;
	int cx, cy;
	int tx, ty;
	unsigned short c;

	for (ty = 0; ty < CTEXT->max_ty; ty++) {
		cy = CTEXT->cur_top + ty * (CFONT->height + CFONT->pitch_y);
		for (cx = CTEXT->cur_left, tx = 0;
		    tx < CTEXT->max_tx + 1;
		    cx += CFONT->width + CFONT->pitch_x, tx++) {
			c = CTEXT->cache_data[tx + ty * (CTEXT->max_tx+1)];
			if (c == 0)
				continue;
			color = CTEXT->cache_attr[tx + ty * (CTEXT->max_tx+1)];
			sys_draw_char(cx, cy, c, color);
		}
	}

	select_redraw();
}
