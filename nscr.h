/*	$Id: nscr.h,v 1.24 2002/01/18 19:36:51 nonaka Exp $	*/

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

#ifndef	__NSCR_H__
#define	__NSCR_H__

#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "y.tab.h"
#include "misc.h"
#include "image.h"

#define	UNUSED(v)	((void)(v))
#define	_ASSERT(s)	assert(s)

#ifdef DEBUG
#define	DPRINTF(arg)	do { printf arg; fflush(stdout); } while (0)
#define	STATIC
#else
#define	DPRINTF(arg)	((void)0)
#define	STATIC		static
#endif

#ifdef SNAP
#define	DSNAP(arg)	do { printf arg; fflush(stdout); } while (0)
#else
#define	DSNAP(arg)	((void)0)
#endif

#define	ERRORMSG(arg)	do { printf arg; fflush(stdout); } while (0)
#define	EXIT(arg) \
do { \
	printf("lineno = %d\n", lineno); \
	exit(arg); \
} while (/*CONSTCOND*/0)

#define	NELEMS(array)	((int)(sizeof(array)/sizeof(array[0])))


#define	NSCR_VERSION	192	/* 1.92 */
#define	MAX_CLICKSTR	20

typedef struct reg_tag {
	int type;
	int subtype;
	int (*func)(struct reg_tag *, long);
	YYSTYPE u;
} reg_t;

typedef struct _nscr_core_tag {
	object_t		obj;

	object_t		*arc;

	off_t			epc;
	int			state;
	int			block;

	struct {
		int		width;
		int		height;

		int		nega;
		long		grayscale;

		rect_t		window_pos;

		object_t	*offscreen;
		object_t	*bg;
		object_t	*text_window;
	} screen;

	struct {
		long		width;
		long		height;
		long		pitch_x;
		long		pitch_y;

		long		speed;

		int		is_bold;
		int		is_shadow;
	} font;

	struct {
		long		cur_left;	/* X 基点 */
		long		cur_top;	/* Y 基点 */
		long		curx, cury;	/* pixel 単位 */

		long		max_tx;		/* 最大桁数(文字単位) */
		long		max_ty;		/* 最大行数(文字単位) */
		long		cur_tx, cur_ty;	/* 文字単位 */

		int		color;

		unsigned short	*cache_data;
		long		*cache_attr;
		size_t		cache_size;	/* 最大文字数 */
		int		cache_len;	/* 何文字表示されているか */
	} text;

	struct {
		unsigned short	data[MAX_CLICKSTR];
		size_t		num;
		long		column;
	} clickstr;

	struct {
		unsigned short	*data;
		size_t		size;

		size_t		pos;
		size_t		remain;

		int		is_show_window;
		int		is_clickwait;
	} msg;

	struct {
		int		color[2];
	} selection;

	struct {
		int		no;
	} effect;

	struct {
		object_t	*symbol;
	} command;

	struct {
		object_t	*symbol;
	} alias;
} nscr_core_t;

#define	CALIAS		(&core->alias)
#define	CCLICKSTR	(&core->clickstr)
#define	CCOMMAND	(&core->command)
#define	CEFFECT		(&core->effect)
#define	CFONT		(&core->font)
#define	CMSG		(&core->msg)
#define	CSCREEN		(&core->screen)
#define	CSELECTION	(&core->selection)
#define	CTEXT		(&core->text)

#define	BLOCK_DEF		(1 << 0)
#define	BLOCK_EX		(1 << 1)
#define	SELECT_COMMAND		(1 << 2)

#define	STATE_ERROR		0
#define	STATE_END		1
#define	STATE_COMMAND		2
#define	STATE_ARG		3
#define	STATE_ARG_NEXT		4
#define	STATE_CHECK_IMAGE	5
#define	STATE_CHECK_PAREN	6
#define	STATE_CHECK_TOKEN0	7
#define	STATE_CHECK_TOKEN1	8
#define	STATE_EXEC		9
#define	STATE_NEXTCOMMAND	10
#define	STATE_EFFECT		11
#define	STATE_BUTTONWAIT	12
#define	STATE_SELECTWAIT	13
#define	STATE_CLICKWAIT		14
#define	STATE_CLICKPAGEWAIT	15
#define	STATE_EFFECTWAIT	16
#define	STATE_WAITTIMER		17
#define	STATE_NUM		18

extern nscr_core_t *core;
extern int token;
extern int state;
extern int next_state;
extern long skipline;
extern int is_globalon;
extern int is_fchklog;
extern int is_labellog;
extern unsigned char globalsav_filename[PATH_MAX];
extern unsigned char nscrflog_filename[PATH_MAX];

int save_fchklog(object_t *);
int load_fchklog(object_t *);
int save_labellog(void);
int load_labellog(void);

/* button.c */
void button_init(unsigned char *);
void button_destroy(void);
void button_clear(void);
void button_add(long, rect_t *, rect_t *);
void button_start(long);
void button_do(long, point_t *);
void button_move(point_t *);
void button_dump(void);

/* bmp.c */
image_t *get_bmp(FILE *, long, size_t);
image_t *get_bmp_lzss(FILE *, long, size_t, size_t);
image_t *get_bmp_nbz(FILE *, long, size_t);

/* command.c */
void command_init(void);

/* display.c */
void text_setwindow(long *, long, unsigned char *);
void text_set_clickstr(unsigned char *, size_t, long);
void text_set_locate(long, long);
void text_setrect(rect_t *, int *, unsigned char *, size_t);
void display_selection(long, long, unsigned char *, int);
void display_message(unsigned char *, size_t);
int display_string(void);
void newpage(void);
void text_redraw(void);

/* effect.c */
int do_effect(void);

/* jpeg.c */
image_t *get_jpeg(FILE *, long, size_t);

/* lex.l */
extern int lineno;
extern int is_encoded;
extern FILE *yyin;

int yylex(void);
void yyrestart(FILE *);
long flex_get_pos(void);
void skiplines(int);

/* lzss.c */
int lzss_decode(FILE *fp, unsigned char *, size_t, size_t);

/* main.c */
extern int trapon;
extern int is_trap;
extern unsigned char *trap_label;

void reset(void);

/* nbz.c */
unsigned char *nbz_decode(FILE *, long, size_t, size_t *);

/* nsa.c */
int nsa_open(object_t *, int);

/* parse.y */
void yyerror(const char *);

/* _parse.c */
int parse(void);

/* png.c */
image_t *get_png(FILE *, long, size_t);
int write_png(object_t *, const unsigned char *);

/* prescan.c */
extern long *linepos;
extern long scan_lineno;

void prescan(FILE *);

/* sar.c */
int sar_open(object_t *, int);

/* screen.c */
void screen_init(void);
void screen_set_bg(unsigned char *);
void redraw(rect_t *);
void screen_setwindow(unsigned char *, int, rect_t *);
void screen_grayscale(int);
void screen_nega(long);
void screen_update(void);
void screen_text_onoff(int);
void screen_text_window_effect_onoff(int);
void screen_show_text_window(void);

/* select.c */
enum {
	SELECT_SELECT = 0,
	SELECT_SELGOSUB,
	SELECT_SELNUM
};

void select_init(int);
void select_destroy(void);
void select_add(int, char *, char *);
void select_start(int, long);
void select_do(int, point_t *);
void select_move(point_t *);
void select_redraw(void);
void select_dump(void);

/* spb.c */
image_t *get_spb(FILE *, long, size_t, size_t);

/* sprite.c */
#define	MAX_SPRITE	50

void load_chr(unsigned char, unsigned char *);
void merge_chr(unsigned char, image_t *);
void hide_chr(unsigned char);
void chr_set_priority(long);
void chr_set_underline(long);
void load_sprite(int, unsigned char *, point_t *, int);
void set_sprite(int, point_t *, int, long);
void show_sprite(int, int);
void clear_sprite(int);
void merge_sprite(image_t *);

/* stack.c */
void init_stack(void);
void stack_show(void);
void push(reg_t *);
reg_t pop(void);

/* timer.c */
extern long waittimer;
extern int cancel_timer;

void timer_set(void);
long timer_get(void);
void timer_term(void);

/* var.c */
#define	GLOBALVAR_MIN	200
#define	GLOBALVAR_MAX	4096
#define	GLOBALVAR_NUM	((GLOBALVAR_MAX)-(GLOBALVAR_MIN))

void init_var(void);
void varnum_set(long, long);
long varnum_get(long);
void varnum_set_limit(long, long, long);
void varstr_set(long, unsigned char *);
unsigned char *varstr_get(long);
void varstr_join(long, unsigned char *);
int save_globalvar(void);
int load_globalvar(void);

/* x11.c */
#define	SCREEN_WIDTH	CSCREEN->width
#define	SCREEN_HEIGHT	CSCREEN->height
#define	BPP		3

int engine_exec(void);
int gui_init(int, int);
void font_init(long, long, long);
void gui_close(void);
void gui_caption(unsigned char *);
void sys_draw_char(int, int, unsigned short, long);
void sys_copy_area(image_t *, int, int, int, int, int, int);
void sys_alloc_color(long, int *);
void sys_mouse_getpos(point_t *);
void mouse_getpos_lastclick(point_t *);

#endif /* __NSCR_H__ */
