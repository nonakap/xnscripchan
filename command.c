/*	$Id: command.c,v 1.24 2002/01/15 17:43:07 nonaka Exp $	*/

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

#include <sys/time.h>
#include <time.h>

#include "nscr.h"
#include "archive.h"
#include "label.h"
#include "symbol.h"

//static int cmd_abssetcursor(reg_t *, long);
static int cmd_add(reg_t *, long);
static int cmd_amsp(reg_t *, long);
static int cmd_arc(reg_t *, long);
static int cmd_autoclick(reg_t *, long);
//static int cmd_avi(reg_t *, long);
static int cmd_bg(reg_t *, long);
//static int cmd_bgalia(reg_t *, long);
static int cmd_blt(reg_t *, long);
static int cmd_br(reg_t *, long);
static int cmd_btn(reg_t *, long);
static int cmd_btndef(reg_t *, long);
static int cmd_btnwait(reg_t *, long);
static int cmd_btnwait2(reg_t *, long);
static int cmd_caption(reg_t *, long);
static int cmd_cdfadeout(reg_t *, long);
static int cmd_cl(reg_t *, long);
static int cmd_click(reg_t *, long);
static int cmd_clickpos(reg_t *, long);
static int cmd_clickstr(reg_t *, long);
//static int cmd_clickvoice(reg_t *, long);
static int cmd_cmp(reg_t *, long);
static int cmd_csp(reg_t *, long);
static int cmd_date(reg_t *, long);
static int cmd_dec(reg_t *, long);
//static int cmd_defaultfont(reg_t *, long);
static int cmd_defaultspeed(reg_t *, long);
static int cmd_definereset(reg_t *, long);
static int cmd_defmp3vol(reg_t *, long);
static int cmd_defsevol(reg_t *, long);
static int cmd_defvoicevol(reg_t *, long);
static int cmd_delay(reg_t *, long);
static int cmd_div(reg_t *, long);
static int cmd_dwave(reg_t *, long);
static int cmd_dwavestop(reg_t *, long);
static int cmd_effect(reg_t *, long);
static int cmd_effectblank(reg_t *, long);
static int cmd_end(reg_t *, long);
static int cmd_erasetextwindow(reg_t *, long);
static int cmd_filelog(reg_t *, long);
static int cmd_game(reg_t *, long);
//static int cmd_getini(reg_t *, long);
static int cmd_getreg(reg_t *, long);
static int cmd_gettimer(reg_t *, long);
static int cmd_getversion(reg_t *, long);
static int cmd_globalon(reg_t *, long);
static int cmd_gosub(reg_t *, long);
static int cmd_goto(reg_t *, long);
static int cmd_humanz(reg_t *, long);
static int cmd_if(reg_t *, long);
static int cmd_inc(reg_t *, long);
static int cmd_inputstr(reg_t *, long);
static int cmd_intlimit(reg_t *, long);
static int cmd_itoa(reg_t *, long);
static int cmd_jumpb(reg_t *, long);
static int cmd_jumpf(reg_t *, long);
static int cmd_killmenu(reg_t *, long);
static int cmd_labellog(reg_t *, long);
static int cmd_ld(reg_t *, long);
static int cmd_len(reg_t *, long);
static int cmd_loadgame(reg_t *, long);
//static int cmd_locate(reg_t *, long);
static int cmd_lookbackbutton(reg_t *, long);
static int cmd_lookbackcolor(reg_t *, long);
//static int cmd_lookbackflush(reg_t *, long);
//static int cmd_lookbackvoice(reg_t *, long);
static int cmd_lsp(reg_t *, long);
static int cmd_lsph(reg_t *, long);
//static int cmd_menu_click_def(reg_t *, long);
//static int cmd_menu_click_page(reg_t *, long);
static int cmd_menu_full(reg_t *, long);
//static int cmd_menu_window(reg_t *, long);
static int cmd_menuselectcolor(reg_t *, long);
//static int cmd_menuselectvoice(reg_t *, long);
static int cmd_menusetwindow(reg_t *, long);
static int cmd_mesbox(reg_t *, long);
static int cmd_mod(reg_t *, long);
static int cmd_mode_ext(reg_t *, long);
static int cmd_monocro(reg_t *, long);
//static int cmd_mousecursor(reg_t *, long);
static int cmd_mov(reg_t *, long);
static int cmd_mov3(reg_t *, long);
static int cmd_mov10(reg_t *, long);
static int cmd_mp3(reg_t *, long);
static int cmd_mp3loop(reg_t *, long);
static int cmd_mp3save(reg_t *, long);
static int cmd_msp(reg_t *, long);
static int cmd_mul(reg_t *, long);
static int cmd_nega(reg_t *, long);
static int cmd_notif(reg_t *, long);
static int cmd_nsa(reg_t *, long);
static int cmd_numalias(reg_t *, long);
static int cmd_ofscpy(reg_t *, long);
static int cmd_play(reg_t *, long);
static int cmd_playonce(reg_t *, long);
static int cmd_playstop(reg_t *, long);
static int cmd_print(reg_t *, long);
static int cmd_puttext(reg_t *, long);
static int cmd_quakex(reg_t *, long);
static int cmd_quakey(reg_t *, long);
static int cmd_reset(reg_t *, long);
static int cmd_resettimer(reg_t *, long);
static int cmd_return(reg_t *, long);
//static int cmd_rlookback(reg_t *, long);
static int cmd_rmenu(reg_t *, long);
static int cmd_rmode(reg_t *, long);
static int cmd_rnd(reg_t *, long);
static int cmd_rnd2(reg_t *, long);
//static int cmd_roff(reg_t *, long);
static int cmd_savegame(reg_t *, long);
static int cmd_savename(reg_t *, long);
static int cmd_savenumber(reg_t *, long);
static int cmd_saveoff(reg_t *, long);
static int cmd_saveon(reg_t *, long);
static int cmd_select(reg_t *, long);
static int cmd_selectcolor(reg_t *, long);
//static int cmd_selectvoice(reg_t *, long);
static int cmd_selgosub(reg_t *, long);
static int cmd_selnum(reg_t *, long);
static int cmd_setcursor(reg_t *, long);
static int cmd_setspeed(reg_t *, long);
static int cmd_setwindow(reg_t *, long);
static int cmd_skip(reg_t *, long);
static int cmd_soundpressplgin(reg_t *, long);
static int cmd_spi(reg_t *, long);
static int cmd_stop(reg_t *, long);
static int cmd_stralias(reg_t *, long);
static int cmd_sub(reg_t *, long);
static int cmd_systemcall(reg_t *, long);
//static int cmd_tal(reg_t *, long);
static int cmd_textclear(reg_t *, long);
//static int cmd_textoff(reg_t *, long);
//static int cmd_texton(reg_t *, long);
static int cmd_textspeed(reg_t *, long);
static int cmd_transmode(reg_t *, long);
static int cmd_trap(reg_t *, long);
static int cmd_underline(reg_t *, long);
static int cmd_versionstr(reg_t *, long);
static int cmd_vsp(reg_t *, long);
static int cmd_wait(reg_t *, long);
static int cmd_waittimer(reg_t *, long);
static int cmd_wave(reg_t *, long);
static int cmd_waveloop(reg_t *, long);
static int cmd_wavestop(reg_t *, long);
static int cmd_windoweffect(reg_t *, long);

static struct command_table_tag {
	unsigned char *str;
	int (*funcp)(reg_t *, long);
} cmd_tbl[] = {
	{ "abssetcursor",	NULL },
	{ "add",		cmd_add },
	{ "amsp",		cmd_amsp },
	{ "arc",		cmd_arc },
	{ "autoclick",		cmd_autoclick },
	{ "avi",		NULL },
	{ "bg",			cmd_bg },
	{ "bgalia",		NULL },
	{ "blt",		cmd_blt },
	{ "br",			cmd_br },
	{ "btn",		cmd_btn },
	{ "btndef",		cmd_btndef },
	{ "btnwait",		cmd_btnwait },
	{ "btnwait2",		cmd_btnwait2 },
	{ "caption",		cmd_caption },
	{ "cdfadeout",		cmd_cdfadeout },
	{ "cl",			cmd_cl },
	{ "click",		cmd_click },
	{ "clickpos",		cmd_clickpos },
	{ "clickstr",		cmd_clickstr },
	{ "clickvoice",		NULL },
	{ "cmp",		cmd_cmp },
	{ "csp",		cmd_csp },
	{ "date",		cmd_date },
	{ "dec",		cmd_dec },
	{ "defaultfont",	NULL },
	{ "defaultspeed",	cmd_defaultspeed },
	{ "definereset",	cmd_definereset },
	{ "defmp3vol",		cmd_defmp3vol },
	{ "defsevol",		cmd_defsevol },
	{ "defvoicevol",	cmd_defvoicevol },
	{ "delay",		cmd_delay },
	{ "div",		cmd_div },
	{ "dwave",		cmd_dwave },
	{ "dwavestop",		cmd_dwavestop },
	{ "effect",		cmd_effect },
	{ "effectblank",	cmd_effectblank },
	{ "end",		cmd_end },
	{ "erasetextwindow",	cmd_erasetextwindow },
	{ "filelog",		cmd_filelog },
	{ "game",		cmd_game },
	{ "getini",		NULL },
	{ "getreg",		cmd_getreg },
	{ "gettimer",		cmd_gettimer },
	{ "getversion",		cmd_getversion },
	{ "globalon",		cmd_globalon },
	{ "gosub",		cmd_gosub },
	{ "goto",		cmd_goto },
	{ "humanz",		cmd_humanz },
	{ "if",			cmd_if },
	{ "inc",		cmd_inc },
	{ "inputstr",		cmd_inputstr },
	{ "intlimit",		cmd_intlimit },
	{ "itoa",		cmd_itoa },
	{ "jumpb",		cmd_jumpb },
	{ "jumpf",		cmd_jumpf },
	{ "killmenu",		cmd_killmenu },
	{ "labellog",		cmd_labellog },
	{ "ld",			cmd_ld },
	{ "len",		cmd_len },
	{ "loadgame",		cmd_loadgame },
	{ "locate",		NULL },
	{ "lookbackbutton",	cmd_lookbackbutton },
	{ "lookbackcolor",	cmd_lookbackcolor },
	{ "lookbackflush",	NULL },
	{ "lookbackvoice",	NULL },
	{ "lsp",		cmd_lsp },
	{ "lsph",		cmd_lsph },
	{ "menu_click_def ",	NULL },
	{ "menu_click_page",	NULL },
	{ "menu_full",		cmd_menu_full },
	{ "menu_window",	NULL },
	{ "menuselectcolor",	cmd_menuselectcolor },
	{ "menuselectvoice",	NULL },
	{ "menusetwindow",	cmd_menusetwindow },
	{ "mesbox",		cmd_mesbox },
	{ "mod",		cmd_mod },
	{ "mode_ext",		cmd_mode_ext },
	{ "monocro",		cmd_monocro },
	{ "mousecursor",	NULL },
	{ "mov",		cmd_mov },
	{ "mov3",		cmd_mov3 },
	{ "mov10",		cmd_mov10 },
	{ "mp3",		cmd_mp3 },
	{ "mp3loop",		cmd_mp3loop },
	{ "mp3save",		cmd_mp3save },
	{ "msp",		cmd_msp },
	{ "mul",		cmd_mul },
	{ "nega",		cmd_nega },
	{ "notif",		cmd_notif },
	{ "nsa",		cmd_nsa },
	{ "numalias",		cmd_numalias },
	{ "ofscpy",		cmd_ofscpy },
	{ "play",		cmd_play },
	{ "playonce",		cmd_playonce },
	{ "playstop",		cmd_playstop },
	{ "print",		cmd_print },
	{ "puttext",		cmd_puttext },
	{ "quakex",		cmd_quakex },
	{ "quakey",		cmd_quakey },
	{ "reset",		cmd_reset },
	{ "resettimer",		cmd_resettimer },
	{ "return",		cmd_return },
	{ "rlookback",		NULL },
	{ "rmenu",		cmd_rmenu },
	{ "rmode",		cmd_rmode },
	{ "rnd",		cmd_rnd },
	{ "rnd2",		cmd_rnd2 },
	{ "roff",		NULL },
	{ "savegame",		cmd_savegame },
	{ "savename",		cmd_savename },
	{ "savenumber",		cmd_savenumber },
	{ "saveoff",		cmd_saveoff },
	{ "saveon",		cmd_saveon },
	{ "select",		cmd_select },
	{ "selectcolor",	cmd_selectcolor },
	{ "selectvoice",	NULL },
	{ "selgosub",		cmd_selgosub },
	{ "selnum",		cmd_selnum },
	{ "setcursor",		cmd_setcursor },
	{ "setspeed",		cmd_setspeed },		/* !sd */
	{ "setwindow",		cmd_setwindow },
	{ "skip",		cmd_skip },
	{ "soundpressplgin",	cmd_soundpressplgin },
	{ "spi",		cmd_spi },
	{ "stop",		cmd_stop },
	{ "stralias",		cmd_stralias },
	{ "sub",		cmd_sub },
	{ "systemcall",		cmd_systemcall },
	{ "tal",		NULL },
	{ "textclear",		cmd_textclear },
	{ "textoff",		NULL },
	{ "texton",		NULL },
	{ "textspeed",		cmd_textspeed },
	{ "transmode",		cmd_transmode },
	{ "trap",		cmd_trap },
	{ "underline",		cmd_underline },
	{ "versionstr",		cmd_versionstr },
	{ "vsp",		cmd_vsp },
	{ "wait",		cmd_wait },
	{ "waittimer",		cmd_waittimer },
	{ "wave",		cmd_wave },
	{ "waveloop",		cmd_waveloop },
	{ "wavestop",		cmd_wavestop },
	{ "windoweffect",	cmd_windoweffect },
};

void
command_init(void)
{
	size_t s;

	for (s = 0; s < NELEMS(cmd_tbl); s++)
		symbol_add_command(cmd_tbl[s].str, cmd_tbl[s].funcp);
}

static long
cmd_get_number(reg_t *p)
{
	long v;

	switch (p->type) {
	case TOKEN_NUMBER:
		v = p->u.val;
		break;

	case TOKEN_VARNUM:
		v = varnum_get(p->u.val);
		break;

	default:
		_ASSERT(0);
		return 0;
	}

	return v;
}

static unsigned char *
cmd_get_string(reg_t *p)
{
	unsigned char *str;

	switch (p->type) {
	case TOKEN_STRING:
		str = p->u.str;
		break;

	case TOKEN_VARSTR:
		str = varstr_get(p->u.val);
		break;

	default:
		_ASSERT(0);
		return NULL;
	}

	return str;
}

static unsigned char *
cmd_get_label(reg_t *p)
{
	unsigned char *label;

	switch (p->type) {
	case TOKEN_LABEL:
		label = p->u.str;
		break;

	case TOKEN_STRING:
		label = p->u.str;
		break;

	case TOKEN_VARSTR:
		label = varstr_get(p->u.val);
		break;

	default:
		_ASSERT(0);
		return NULL;
	}

	return label + 1;	/* skip '*' */
}

/*--
 * command
 */
static int
cmd_add(reg_t *p, long narg)
{

	DPRINTF(("add: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	switch (p[0].type) {
	case TOKEN_VARNUM:
		varnum_set(p[0].u.val,
		    varnum_get(p[0].u.val) + cmd_get_number(&p[1]));
		break;

	case TOKEN_VARSTR:
		varstr_join(p[0].u.val, cmd_get_string(&p[1]));
		break;

	default:
		_ASSERT(0);
		return STATE_ERROR;
	}

	return STATE_COMMAND;
}

static int
cmd_amsp(reg_t *p, long narg)
{
	point_t point;
	long nspr, maskv;

	DPRINTF(("amsp: narg = %ld\n", narg));
	_ASSERT(narg == 4);

	nspr = cmd_get_number(&p[0]);
	point.x = cmd_get_number(&p[1]);
	point.y = cmd_get_number(&p[2]);
	maskv = cmd_get_number(&p[3]);
	set_sprite(nspr, &point, 1, maskv);

	return STATE_COMMAND;
}

static int
cmd_arc(reg_t *p, long narg)
{
	unsigned char *str;
	size_t s, len;

	DPRINTF(("arc: narg = %ld\n", narg));
	_ASSERT(narg == 1);
	_ASSERT(p[0].type == TOKEN_STRING);

	str = Estrdup(p[0].u.str);
	len = strlen(str);
	for (s = 0; s < len; s++) {
		if (str[s] == '|') {
			str[s] = '\0';
			break;
		}
	}
	_ASSERT(s < len);
	archive_open(core->arc, str);
	Efree(str);

	return STATE_COMMAND;
}

static int
cmd_autoclick(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("autoclick: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	return STATE_COMMAND;
}

static int
cmd_bg(reg_t *p, long narg)
{
	unsigned char *bg;

	DPRINTF(("bg: narg = %ld\n", narg));
	_ASSERT(narg == 2 || narg == 3);

	switch (p[0].type) {
	case TOKEN_STRING:
	case TOKEN_VARSTR:
		bg = cmd_get_string(&p[0]);
		_ASSERT(bg != NULL);
		screen_set_bg(bg, 1 /* XXX effect */);
		break;

	case TOKEN_COLOR:
		if (p[0].u.val == 0) {
			screen_set_bg("black", 1);
		} else if (p[0].u.val == 0xffffff) {
			screen_set_bg("white", 1);
		} else {
			_ASSERT(0);
			return STATE_ERROR;
		}
		break;

	default:
		_ASSERT(0);
		return STATE_ERROR;
	}

	return STATE_COMMAND;
}

static int
cmd_blt(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("blt: narg = %ld\n", narg));
	_ASSERT(narg == 8);

	/* XXX */
	/* dest x, dest y, dest width, dest height, src too... */

	return STATE_COMMAND;
}

static int
cmd_br(reg_t *p, long narg)
{

	DPRINTF(("br: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	display_message("\n", 1);

	return STATE_COMMAND;
}

static int
cmd_btn(reg_t *p, long narg)
{
	rect_t sel_rect, draw_rect;
	long v[7];
	int i;

	DPRINTF(("btn: narg = %ld\n", narg));
	_ASSERT(narg == 7);

	for (i = 0; i < 7; i++)
		v[i] = cmd_get_number(&p[i]);

	sel_rect.left = v[1];
	sel_rect.top = v[2];
	sel_rect.width = draw_rect.width = v[3];
	sel_rect.height = draw_rect.height = v[4];
	draw_rect.left = v[5];
	draw_rect.top = v[6];

	button_add(v[0], &sel_rect, &draw_rect);

	return STATE_COMMAND;
}

static int
cmd_btndef(reg_t *p, long narg)
{
	unsigned char *q;

	DPRINTF(("btndef: narg = %ld\n", narg));
	_ASSERT(narg == 1 || narg == 7);

	q = cmd_get_string(&p[0]);
	_ASSERT(q != NULL);
	button_init(q);

	return STATE_COMMAND;
}

static int
cmd_btnwait(reg_t *p, long narg)
{

	DPRINTF(("btnwait: narg = %ld\n", narg));
	_ASSERT(narg == 1);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	button_start(p[0].u.val);

	return STATE_BUTTONWAIT;
}

static int
cmd_btnwait2(reg_t *p, long narg)
{

	DPRINTF(("btnwait2: narg = %ld\n", narg));
	_ASSERT(narg == 1);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	button_start(p[0].u.val);

	return STATE_BUTTONWAIT;
}

static int
cmd_caption(reg_t *p, long narg)
{
	unsigned char *str;

	DPRINTF(("caption: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	str = cmd_get_string(&p[0]);
	_ASSERT(str != NULL);
	gui_caption(str);

	return STATE_COMMAND;
}

static int
cmd_cdfadeout(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("cdfadeout: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	return STATE_COMMAND;
}

static int
cmd_cl(reg_t *p, long narg)
{
	unsigned char *s;

	DPRINTF(("cl: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	s = cmd_get_string(&p[0]);
	_ASSERT(s != NULL);

	hide_chr(s[0], 1 /* XXX effect */);

	return STATE_COMMAND;
}

static int
cmd_click(reg_t *p, long narg)
{

	DPRINTF(("click: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	return STATE_CLICKWAIT;
}

static int
cmd_clickpos(reg_t *p, long narg)
{
	point_t point;

	DPRINTF(("clickpos: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_VARNUM);
	_ASSERT(p[1].type == TOKEN_VARNUM);

	mouse_getpos_lastclick(&point);
	varnum_set(p[0].u.val, point.x);
	varnum_set(p[1].u.val, point.y);

	return STATE_COMMAND;
}

static int
cmd_clickstr(reg_t *p, long narg)
{
	unsigned char *s;
	size_t len;

	DPRINTF(("clickstr: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	s = cmd_get_string(&p[0]);
	_ASSERT(s != NULL);
	len = strlen(s);
	_ASSERT((len % 2) == 0);
	text_set_clickstr(s, len / 2, cmd_get_number(&p[1]));

	return STATE_COMMAND;
}

static int
cmd_cmp(reg_t *p, long narg)
{
	unsigned char *q[2];
	int i;

	DPRINTF(("cmp: narg = %ld\n", narg));
	_ASSERT(narg == 3);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	for (i = 1; i < 3; i++)
		q[i-1] = cmd_get_string(&p[i]);

	varnum_set(p[0].u.val, strcasecmp(q[0], q[1]));

	return STATE_COMMAND;
}

static int
cmd_csp(reg_t *p, long narg)
{
	int v;

	DPRINTF(("csp: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	v = cmd_get_number(&p[0]);
	clear_sprite(v);

	return STATE_COMMAND;
}

static int
cmd_date(reg_t *p, long narg)
{
	struct tm *tm;
	time_t t;
	int i;

	DPRINTF(("date: narg = %ld\n", narg));
	_ASSERT(narg == 3);
	for (i = 0; i < 3; i++) {
		_ASSERT(p[i].type == TOKEN_VARNUM);
	}

	t = time(NULL);
	tm = localtime(&t);

	varnum_set(p[0].u.val, tm->tm_year);
	varnum_set(p[1].u.val, tm->tm_mon + 1);
	varnum_set(p[2].u.val, tm->tm_mday);

	return STATE_COMMAND;
}

static int
cmd_dec(reg_t *p, long narg)
{

	DPRINTF(("dec: narg = %ld\n", narg));
	_ASSERT(narg == 1);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val, varnum_get(p[0].u.val) - 1);

	return STATE_COMMAND;
}

static int
cmd_defaultspeed(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("defaultspeed: narg = %ld\n", narg));
	_ASSERT(narg == 3);

	return STATE_COMMAND;
}

static int
cmd_definereset(reg_t *p, long narg)
{

	DPRINTF(("definereset: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	save_globalvar();
	save_fchklog(core->arc);
	save_labellog();

	reset();
	chr_set_priority(25);
	chr_set_underline(479);
	is_globalon = 0;
	is_fchklog = 0;
	is_labellog = 0;

	/* stralias, numalias を解放しなければならない */

	core->block = BLOCK_DEFINE;
	label_jump("define");

	return STATE_COMMAND;
}

static int
cmd_defmp3vol(reg_t *p, long narg)
{

	UNUSED(p);
	DPRINTF(("defmp3vol: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_defsevol(reg_t *p, long narg)
{

	UNUSED(p);
	DPRINTF(("defsevol: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_defvoicevol(reg_t *p, long narg)
{

	UNUSED(p);
	DPRINTF(("defvoicevol: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_delay(reg_t *p, long narg)
{

	DPRINTF(("delay: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	waittimer = cmd_get_number(&p[0]);
	cancel_timer = 1;
	timer_set();

	return STATE_WAITTIMER;
}

static int
cmd_div(reg_t *p, long narg)
{

	DPRINTF(("div: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val,
	    varnum_get(p[0].u.val) / cmd_get_number(&p[1]));

	return STATE_COMMAND;
}

static int
cmd_dwave(reg_t *p, long narg)
{

	UNUSED(p);
	DPRINTF(("dwave: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_dwavestop(reg_t *p, long narg)
{

	UNUSED(p);
	DPRINTF(("dwavestop: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_effect(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("effect: narg = %ld\n", narg));
	_ASSERT(narg >= 2 && narg <= 4);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_effectblank(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("effectblank: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_end(reg_t *p, long narg)
{

	DPRINTF(("end: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	return STATE_END;
}

static int
cmd_erasetextwindow(reg_t *p, long narg)
{

	DPRINTF(("erasetextwindow: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	screen_text_window_effect_onoff(cmd_get_number(&p[0]));

	return STATE_COMMAND;
}

static int
cmd_filelog(reg_t *p, long narg)
{

	DPRINTF(("filelog: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	is_fchklog = 1;
	load_fchklog(core->arc);

	return STATE_COMMAND;
}

static int
cmd_game(reg_t *p, long narg)
{

	DPRINTF(("game: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	core->block = BLOCK_EXEC;
	label_jump("start");

	return STATE_COMMAND;
}

static int
cmd_getreg(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("getreg: narg = %ld\n", narg));
	_ASSERT(narg == 3);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_gettimer(reg_t *p, long narg)
{

	DPRINTF(("gettimer: narg = %ld\n", narg));
	_ASSERT(narg == 1);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val, timer_get());

	return STATE_COMMAND;
}

static int
cmd_getversion(reg_t *p, long narg)
{

	DPRINTF(("getversion: narg = %ld\n", narg));
	_ASSERT(narg == 1);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val, NSCR_VERSION);

	return STATE_COMMAND;
}

static int
cmd_globalon(reg_t *p, long narg)
{

	DPRINTF(("globalon: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	is_globalon = 1;
	load_globalvar();

	return STATE_COMMAND;
}

static int
cmd_gosub(reg_t *p, long narg)
{
	reg_t t;
	unsigned char *label;

	DPRINTF(("gosub: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	label = cmd_get_label(&p[0]);
	_ASSERT(label != NULL);

	t.type = TOKEN_OFFSET;
	t.u.val = flex_get_pos();
	push(&t);

	t.type = TOKEN_LINENO;
	t.u.val = lineno;
	push(&t);

	DSNAP(("gosub: label = %s\n", label));
	label_jump(label);

	return STATE_COMMAND;
}

static int
cmd_goto(reg_t *p, long narg)
{
	unsigned char *label;

	DPRINTF(("goto: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	label = cmd_get_label(&p[0]);
	_ASSERT(label != NULL);

	DSNAP(("goto: label = %s\n", label));
	label_jump(label);

	return STATE_COMMAND;
}

static int
cmd_humanz(reg_t *p, long narg)
{

	DPRINTF(("humanz: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	chr_set_priority(cmd_get_number(&p[0]));

	return STATE_COMMAND;
}

static int
cmd_if(reg_t *p, long narg)
{
	reg_t q[2], op;
	long v1, v2;
	long n = narg;
	unsigned char *str;
	int rv = 1;

	DPRINTF(("if: narg = %ld\n", narg));
	_ASSERT(p != NULL);
	_ASSERT(narg >= 2);

	while (n > 0) {
		if (--n < 0)
			break;
		q[0] = p[n];

		if (--n < 0)
			break;
		op = p[n];

		if (op.type == TOKEN_FCHK) {
			str = cmd_get_string(&q[0]);
			_ASSERT(str != NULL);
			if (!archive_is_access(core->arc, str)) {
				rv = 0;
				break;
			}
			continue;
		}
		if (op.type == TOKEN_LCHK) {
			str = cmd_get_label(&q[0]);
			_ASSERT(str != NULL);
			if (!label_is_access(str)) {
				rv = 0;
				break;
			}
			continue;
		}

		if (--n < 0)
			break;
		q[1] = p[n];

		v1 = cmd_get_number(&q[1]);
		v2 = cmd_get_number(&q[0]);

		DSNAP(("if: arg1 = %ld\n", v1));
		DSNAP(("if: arg2 = %ld\n", v2));

		switch (op.type) {
		case '=':
		case TOKEN_EQ:
			DSNAP(("if: ==\n"));
			if (!(v1 == v2))
				rv = 0;
			break;

		case TOKEN_NE:
			DSNAP(("if: !=\n"));
			if (!(v1 != v2))
				rv = 0;
			break;

		case '<':
			DSNAP(("if: <\n"));
			if (!(v1 < v2))
				rv = 0;
			break;

		case '>':
			DSNAP(("if: >\n"));
			if (!(v1 > v2))
				rv = 0;
			break;

		case TOKEN_LE:
			DSNAP(("if: <=\n"));
			if (!(v1 <= v2))
				rv = 0;
			break;

		case TOKEN_GE:
			DSNAP(("if: >=\n"));
			if (!(v1 >= v2))
				rv = 0;
			break;

		default:
			_ASSERT(0);
			return STATE_ERROR;
		}
		if (rv == 0)
			break;
	}
	_ASSERT(n >= 0);

	DSNAP(("if: rv = %d\n", rv));

	if (rv == 0) {
		skiplines(1);
		return STATE_COMMAND;
	}
	return STATE_NEXTCOMMAND;
}

static int
cmd_inc(reg_t *p, long narg)
{

	DPRINTF(("inc: narg = %ld\n", narg));
	_ASSERT(narg == 1);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val, varnum_get(p[0].u.val) + 1);

	return STATE_COMMAND;
}

static int
cmd_inputstr(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("inputstr: narg = %ld\n", narg));
	_ASSERT(narg == 4 || narg == 8);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_intlimit(reg_t *p, long narg)
{

	DPRINTF(("intlimit: narg = %ld\n", narg));
	_ASSERT(narg == 3);

	varnum_set_limit(cmd_get_number(&p[0]),
	    cmd_get_number(&p[1]), cmd_get_number(&p[2]));

	return STATE_COMMAND;
}

static int
cmd_itoa(reg_t *p, long narg)
{
	unsigned char tmp[256];

	DPRINTF(("itoa: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_VARSTR);

	snprintf(tmp, sizeof(tmp), "%ld", cmd_get_number(&p[1]));
	varstr_set(p[0].u.val, tmp);

	return STATE_COMMAND;
}

static int
cmd_jumpb(reg_t *p, long narg)
{

	DPRINTF(("jumpb: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	tilde_jump(LABEL_TILDE_JUMPB);

	return STATE_COMMAND;
}

static int
cmd_jumpf(reg_t *p, long narg)
{

	DPRINTF(("jumpf: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	tilde_jump(LABEL_TILDE_JUMPF);

	return STATE_COMMAND;
}

static int
cmd_killmenu(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("killmenu: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_labellog(reg_t *p, long narg)
{

	DPRINTF(("labellog: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	is_labellog = 1;
	load_labellog();

	return STATE_COMMAND;
}

static int
cmd_ld(reg_t *p, long narg)
{
	unsigned char *s;

	DPRINTF(("ld: narg = %ld\n", narg));
	_ASSERT(narg == 3 || narg == 4);

	s = cmd_get_string(&p[0]);
	load_chr(s[0], cmd_get_string(&p[1]), 1 /* XXX effect */);

	return STATE_COMMAND;
}

static int
cmd_len(reg_t *p, long narg)
{

	DPRINTF(("ld: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val, strlen(cmd_get_string(&p[1])));

	return STATE_COMMAND;
}

static int
cmd_lookbackcolor(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("cmd_lookbackcolor: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_loadgame(reg_t *p, long narg)
{
	long v;

	DPRINTF(("loadgame: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	v = cmd_get_number(&p[0]);
	UNUSED(v);

	return STATE_COMMAND;
}

static int
cmd_lookbackbutton(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("lookbackbutton: narg = %ld\n", narg));
	_ASSERT(narg == 4);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_lsp_subr(reg_t *p, long narg, int show)
{
	point_t point;
	unsigned char *filename;
	long nspr;

	_ASSERT(narg == 4 || narg == 5);
	if (narg == 5) {
		_ASSERT(p[4].type == TOKEN_NUMBER);
	}

	nspr = cmd_get_number(&p[0]);
	filename = cmd_get_string(&p[1]);
	point.x = cmd_get_number(&p[2]);
	point.y = cmd_get_number(&p[3]);
	load_sprite(nspr, filename, &point, show);
	if (narg == 5) {
		point.x = point.y = 0;
		set_sprite(nspr, &point, 1, cmd_get_number(&p[4]));
	}

	return STATE_COMMAND;
}

static int
cmd_lsp(reg_t *p, long narg)
{

	DPRINTF(("lsp: narg = %ld\n", narg));
	return cmd_lsp_subr(p, narg, 1);
}

static int
cmd_lsph(reg_t *p, long narg)
{

	DPRINTF(("lsph: narg = %ld\n", narg));
	return cmd_lsp_subr(p, narg, 0);
}

static int
cmd_menu_full(reg_t *p, long narg)
{

	DPRINTF(("menu_full: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_menuselectcolor(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("menuselectcolor: narg = %ld\n", narg));
	_ASSERT(narg == 3);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_menusetwindow(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("menusetwindow: narg = %ld\n", narg));
	_ASSERT(narg == 7);

	/* XXX */
	redraw(1);

	return STATE_COMMAND;
}

static int
cmd_mesbox(reg_t *p, long narg)
{
	unsigned char *title, *msg;

	DPRINTF(("mesbox: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	msg = cmd_get_string(&p[0]);
	title = cmd_get_string(&p[1]);

	/* XXX */
	DSNAP(("mesbox: title: %s", title));
	DSNAP(("mesbox: message: %s", msg));

	return STATE_COMMAND;
}

static int
cmd_mod(reg_t *p, long narg)
{

	DPRINTF(("mod: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val, varnum_get(p[0].u.val) % cmd_get_number(&p[1]));

	return STATE_COMMAND;
}

static int
cmd_mode_ext(reg_t *p, long narg)
{

	DPRINTF(("mode_ext: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_monocro(reg_t *p, long narg)
{
	long v;

	DPRINTF(("monocro: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	switch (p[0].type) {
	case TOKEN_COLOR:
		v = p[0].u.val;
		break;

	case TOKEN_STRING:
		if (strcasecmp(p[0].u.str, "off") == 0) {
			v = 0;
			break;
		}
		/*FALLTHROUGH*/
	default:
		_ASSERT(0);
		return STATE_ERROR;
	}

	screen_grayscale(v);

	return STATE_COMMAND;
}

static int
cmd_mov(reg_t *p, long narg)
{
	unsigned char *str;
	long v;

	DPRINTF(("mov: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	switch (p[0].type) {
	case TOKEN_VARNUM:
		v = cmd_get_number(&p[1]);
		varnum_set(p[0].u.val, v);
		break;

	case TOKEN_VARSTR:
		str = cmd_get_string(&p[1]);
		_ASSERT(str);
		varstr_set(p[0].u.val, str);
		break;

	default:
		_ASSERT(0);
		return STATE_ERROR;
	}

	return STATE_COMMAND;
}

static int
cmd_mov3(reg_t *p, long narg)
{
	unsigned char *str;
	long v;
	int i;

	DPRINTF(("mov3: narg = %ld\n", narg));
	_ASSERT(narg == 4);

	switch (p[0].type) {
	case TOKEN_VARNUM:
		for (i = 1; i < 4; i++) {
			v = cmd_get_number(&p[i]);
			varnum_set(p[0].u.val + i - 1, v);
		}
		break;

	case TOKEN_VARSTR:
		for (i = 1; i < 4; i++) {
			str = cmd_get_string(&p[i]);
			_ASSERT(str != NULL);
			varstr_set(p[0].u.val + i - 1, str);
		}
		break;

	default:
		_ASSERT(0);
		return STATE_ERROR;
	}

	return STATE_COMMAND;
}

static int
cmd_mov10(reg_t *p, long narg)
{
	unsigned char *str;
	long v;
	int i;

	DPRINTF(("mov10: narg = %ld\n", narg));
	_ASSERT(narg == 11);

	switch (p[0].type) {
	case TOKEN_VARNUM:
		for (i = 1; i < 11; i++) {
			v = cmd_get_number(&p[i]);
			varnum_set(p[0].u.val + i - 1, v);
		}
		break;

	case TOKEN_VARSTR:
		for (i = 1; i < 11; i++) {
			str = cmd_get_string(&p[i]);
			_ASSERT(str != NULL);
			varstr_set(p[0].u.val + i - 1, str);
		}
		break;

	default:
		_ASSERT(0);
		return STATE_ERROR;
	}

	return STATE_COMMAND;
}

static int
cmd_mp3(reg_t *p, long narg)
{

	UNUSED(p);
	DPRINTF(("mp3: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_mp3loop(reg_t *p, long narg)
{

	UNUSED(p);
	DPRINTF(("mp3loop: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_mp3save(reg_t *p, long narg)
{

	UNUSED(p);
	DPRINTF(("mp3save: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_msp(reg_t *p, long narg)
{
	point_t point;
	long nspr, maskv;

	DPRINTF(("msp: narg = %ld\n", narg));
	_ASSERT(narg == 4);

	nspr = cmd_get_number(&p[0]);
	point.x = cmd_get_number(&p[1]);
	point.y = cmd_get_number(&p[2]);
	maskv = cmd_get_number(&p[3]);
	set_sprite(nspr, &point, 0, maskv);

	return STATE_COMMAND;
}

static int
cmd_mul(reg_t *p, long narg)
{

	DPRINTF(("mul: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val, varnum_get(p[0].u.val) * cmd_get_number(&p[1]));

	return STATE_COMMAND;
}

static int
cmd_nega(reg_t *p, long narg)
{

	DPRINTF(("nega: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	screen_nega(cmd_get_number(&p[0]));

	return STATE_COMMAND;
}

static int
cmd_notif(reg_t *p, long narg)
{
	reg_t q[2], op;
	long v1, v2;
	long n = narg;
	unsigned char *cgp;
	int rv = 1;

	DPRINTF(("notif: narg = %ld\n", narg));
	_ASSERT(p != NULL);
	_ASSERT(narg >= 2);

	while (n > 0) {
		if (--n < 0)
			break;
		q[0] = p[n];

		if (--n < 0)
			break;
		op = p[n];

		if (op.type == TOKEN_FCHK) {
			cgp = cmd_get_string(&q[0]);
			_ASSERT(cgp != NULL);
			if (archive_is_access(core->arc, cgp)) {
				rv = 0;
				break;
			}
			continue;
		}

		if (--n < 0)
			break;
		q[1] = p[n];

		v1 = cmd_get_number(&q[1]);
		v2 = cmd_get_number(&q[0]);

		DSNAP(("notif: arg1 = %ld\n", v1));
		DSNAP(("notif: arg2 = %ld\n", v2));

		switch (op.type) {
		case '=':
		case TOKEN_EQ:
			DSNAP(("notif: ==\n"));
			if (v1 == v2)
				rv = 0;
			break;

		case TOKEN_NE:
			DSNAP(("notif: !=\n"));
			if (v1 != v2)
				rv = 0;
			break;

		case '<':
			DSNAP(("notif: <\n"));
			if (v1 < v2)
				rv = 0;
			break;

		case '>':
			DSNAP(("notif: >\n"));
			if (v1 > v2)
				rv = 0;
			break;

		case TOKEN_LE:
			DSNAP(("notif: <=\n"));
			if (v1 <= v2)
				rv = 0;
			break;

		case TOKEN_GE:
			DSNAP(("notif: >=\n"));
			if (v1 >= v2)
				rv = 0;
			break;

		default:
			_ASSERT(0);
			return STATE_ERROR;
		}
		if (rv == 0)
			break;
	}
	_ASSERT(n >= 0);

	DSNAP(("notif: rv = %d\n", rv));

	if (rv == 0) {
		skiplines(1);
		return STATE_COMMAND;
	}
	return STATE_NEXTCOMMAND;
}

static int
cmd_nsa(reg_t *p, long narg)
{
	unsigned char tmpfilename[16];
	int i;

	DPRINTF(("nsa: narg = %ld\n", narg));
	_ASSERT(narg == 0);
	_ASSERT(p == NULL);

	archive_open(core->arc, "arc.nsa");

	strlcpy(tmpfilename, "arc?.nsa", sizeof(tmpfilename));
	for (i = 1; i < 10; i++) {
		tmpfilename[3] = '0' + i;
		archive_open(core->arc, tmpfilename);
	}

	return STATE_COMMAND;
}

static int
cmd_numalias(reg_t *p, long narg)
{

	DPRINTF(("numalias: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_STRING);
	_ASSERT(p[1].type == TOKEN_NUMBER);

	symbol_add_numalias(p[0].u.str, p[1].u.val);

	return STATE_COMMAND;
}

static int
cmd_ofscpy(reg_t *p, long narg)
{

	DPRINTF(("ofscpy: narg = %ld\n", narg));
	_ASSERT(narg == 0);
	_ASSERT(p == NULL);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_play(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("play: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_playonce(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("playonce: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_playstop(reg_t *p, long narg)
{

	DPRINTF(("playstop: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_print(reg_t *p, long narg)
{

	DPRINTF(("print: narg = %ld\n", narg));
	_ASSERT(narg >= 1 && narg <= 3);
	_ASSERT(p[0].type == TOKEN_NUMBER);

	if (p[0].u.val != 0)
		redraw(p[0].u.val);

	return STATE_COMMAND;
}

static int
cmd_puttext(reg_t *p, long narg)
{
	unsigned char *s;

	DPRINTF(("puttext: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	s = cmd_get_string(&p[0]);
	display_message(s, strlen(s));

	return STATE_COMMAND;
}

static int
cmd_quakex(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("quakex: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	/* XXX */
	newpage();

	return STATE_COMMAND;
}

static int
cmd_quakey(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("quakey: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	/* XXX */
	newpage();

	return STATE_COMMAND;
}

static int
cmd_return(reg_t *p, long narg)
{
	reg_t t;

	DPRINTF(("return: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	t = pop();
	_ASSERT(t.type == TOKEN_LINENO);
	lineno = t.u.val;

	t = pop();
	_ASSERT(t.type == TOKEN_OFFSET);
	fseek(yyin, t.u.val, SEEK_SET);
	yyrestart(yyin);

	return STATE_COMMAND;
}

static int
cmd_reset(reg_t *p, long narg)
{

	DPRINTF(("reset: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	save_globalvar();
	save_fchklog(core->arc);
	save_labellog();

	reset();
	/* Already exec mode */
	label_jump("start");

	return STATE_COMMAND;
}

static int
cmd_resettimer(reg_t *p, long narg)
{

	DPRINTF(("resettimer: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	timer_set();

	return STATE_COMMAND;
}

static int
cmd_rmenu(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("rmenu: narg = %ld\n", narg));
	_ASSERT((narg % 2) == 0);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_rmode(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("rmode: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_rnd(reg_t *p, long narg)
{

	DPRINTF(("rnd: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val, random() % cmd_get_number(&p[1]));

	return STATE_COMMAND;
}

static int
cmd_rnd2(reg_t *p, long narg)
{
	long rndmin, rndmax;

	DPRINTF(("rnd2: narg = %ld\n", narg));
	_ASSERT(narg == 3);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	rndmin = cmd_get_number(&p[1]);
	rndmax = cmd_get_number(&p[2]);
	varnum_set(p[0].u.val, (random() % (rndmax - rndmin)) + rndmin);

	return STATE_COMMAND;
}

static int
cmd_savegame(reg_t *p, long narg)
{
	long v;

	DPRINTF(("savegame: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */
	v = cmd_get_number(&p[0]);
	UNUSED(v);

	return STATE_COMMAND;
}

static int
cmd_savename(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("savename: narg = %ld\n", narg));
	_ASSERT(narg == 3);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_savenumber(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("savenumber: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_saveoff(reg_t *p, long narg)
{

	DPRINTF(("saveoff: narg = %ld\n", narg));
	_ASSERT(narg == 0);
	_ASSERT(p == NULL);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_saveon(reg_t *p, long narg)
{

	DPRINTF(("saveon: narg = %ld\n", narg));
	_ASSERT(narg == 0);
	_ASSERT(p == NULL);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_select(reg_t *p, long narg)
{
	long n;
	long i;

	DPRINTF(("select: narg = %ld\n", narg));
	_ASSERT((narg % 2) == 0);

	n = narg / 2;
	select_init(n);

	for (i = 0; i < n; i++)
		select_add(i, p[i * 2].u.str, p[i * 2 + 1].u.str);

	select_start(SELECT_SELECT, -1);

	return STATE_SELECTWAIT;
}

static int
cmd_selectcolor(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("selectcolor: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_COLOR);
	_ASSERT(p[1].type == TOKEN_COLOR);

	sys_alloc_color(p[0].u.val, &core->selection.color[0]);
	sys_alloc_color(p[1].u.val, &core->selection.color[1]);

	return STATE_COMMAND;
}

static int
cmd_selgosub(reg_t *p, long narg)
{
	long n;
	long i;

	DPRINTF(("selgosub: narg = %ld\n", narg));
	_ASSERT((narg % 2) == 0);

	n = narg / 2;
	select_init(n);

	for (i = 0; i < n; i++)
		select_add(i, p[i * 2].u.str, p[i * 2 + 1].u.str);

	select_start(SELECT_SELGOSUB, -1);

	return STATE_SELECTWAIT;
}

static int
cmd_selnum(reg_t *p, long narg)
{
	unsigned char *str;
	long i;

	DPRINTF(("selnum: narg = %ld\n", narg));
	_ASSERT(narg >= 2);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	select_init(narg - 1);

	for (i = 1; i < narg; i++) {
		str = cmd_get_string(&p[i]);
		_ASSERT(str != NULL);
		select_add(i - 1, str, NULL);
	}

	select_start(SELECT_SELNUM, p[0].u.val);

	return STATE_SELECTWAIT;
}

static int
cmd_setcursor(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("setcursor: narg = %ld\n", narg));
	_ASSERT(narg == 4);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_setspeed(reg_t *p, long narg)
{

	DPRINTF(("setspeed: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_setwindow(reg_t *p, long narg)
{
	long v[16];
	unsigned char *filename;
	long i;

	DPRINTF(("setwindow: narg = %ld\n", narg));
	_ASSERT(narg >= 13 || narg <= 16);

	for (i = 0; i < 11; i++) {
		v[i] = cmd_get_number(&p[i]);
	}
	switch (p[11].type) {
	case TOKEN_COLOR:
		v[11] = p[11].u.val;
		filename = NULL;
		break;

	case TOKEN_STRING:
		v[11] = 0;
		filename = p[11].u.str;
		break;

	default:
		_ASSERT(0);
		return STATE_ERROR;
	}
	for (i = 12; i < narg; i++) {
		v[i] = cmd_get_number(&p[i]);
	}

	text_setwindow(v, narg, filename);

	return STATE_COMMAND;
}

static int
cmd_skip(reg_t *p, long narg)
{
	long v;

	DPRINTF(("skip: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	v = cmd_get_number(&p[0]);
	DSNAP(("skip: skipline = %ld\n", p[0].u.val));
	skiplines(v + (token == TOKEN_NL ? -1 : 0));

	return STATE_COMMAND;
}

static int
cmd_soundpressplgin(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("soundpressplgin: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_stop(reg_t *p, long narg)
{

	DPRINTF(("stop: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_spi(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("spi: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_stralias(reg_t *p, long narg)
{

	DPRINTF(("stralias: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_STRING);
	_ASSERT(p[1].type == TOKEN_STRING);

	return STATE_COMMAND;
}

static int
cmd_sub(reg_t *p, long narg)
{

	DPRINTF(("sub: narg = %ld\n", narg));
	_ASSERT(narg == 2);
	_ASSERT(p[0].type == TOKEN_VARNUM);

	varnum_set(p[0].u.val, varnum_get(p[0].u.val) - cmd_get_number(&p[1]));

	return STATE_COMMAND;
}

static int
cmd_systemcall(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("systemcall: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_textclear(reg_t *p, long narg)
{

	DPRINTF(("textclear: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	newpage();
	redraw(1);

	return STATE_COMMAND;
}

static int
cmd_textspeed(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("textspeed: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_transmode(reg_t *p, long narg)
{
	const unsigned char *transmode_def[] = {
		"copy",
		"leftup",
		"rightup"
		"alpha"
	};
	unsigned char *transmode;
	size_t s;

	DPRINTF(("transmode: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	transmode = cmd_get_string(&p[0]);
	for (s = 0; s < NELEMS(transmode_def); s++)
		if (strcasecmp(transmode, transmode_def[s]) == 0)
			break;
	_ASSERT(s < NELEMS(transmode_def));

	set_transmode_default(s);

	return STATE_COMMAND;
}

static int
cmd_trap(reg_t *p, long narg)
{

	DPRINTF(("trap: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	switch (p[0].type) {
	case TOKEN_STRING:
		if (strcasecmp(p[0].u.str, "off") == 0) {
			trapon = 0;
			is_trap = 0;
			if (trap_label)
				Efree(trap_label);
			trap_label = NULL;
		} else {
			_ASSERT(0);
			return STATE_ERROR;
		}
		break;

	case TOKEN_LABEL:
		trapon = 1;
		is_trap = 0;
		if (trap_label) {
			if (strcasecmp(trap_label, p[0].u.str) != 0) {
				Efree(trap_label);
				trap_label = Estrdup(p[0].u.str);
			}
		} else {
			trap_label = Estrdup(p[0].u.str);
		}
		break;

	default:
		_ASSERT(0);
		return STATE_ERROR;
	}

	return STATE_COMMAND;
}

static int
cmd_underline(reg_t *p, long narg)
{

	DPRINTF(("underline: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	chr_set_underline(cmd_get_number(&p[0]));

	return STATE_COMMAND;
}

static int
cmd_versionstr(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("versionstr: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_vsp(reg_t *p, long narg)
{

	DPRINTF(("vsp: narg = %ld\n", narg));
	_ASSERT(narg == 2);

	show_sprite(cmd_get_number(&p[0]), cmd_get_number(&p[1]));

	return STATE_COMMAND;
}

static int
cmd_wait(reg_t *p, long narg)
{

	DPRINTF(("wait: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	waittimer = cmd_get_number(&p[0]);
	cancel_timer = 0;
	timer_set();

	return STATE_WAITTIMER;
}

static int
cmd_waittimer(reg_t *p, long narg)
{

	DPRINTF(("waittimer: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	waittimer = cmd_get_number(&p[0]);
	cancel_timer = 0;
	timer_set();

	return STATE_WAITTIMER;
}

static int
cmd_wave(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("wave: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_waveloop(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("waveloop: narg = %ld\n", narg));
	_ASSERT(narg == 1);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_wavestop(reg_t *p, long narg)
{

	DPRINTF(("wavestop: narg = %ld\n", narg));
	_ASSERT(p == NULL);
	_ASSERT(narg == 0);

	/* XXX */

	return STATE_COMMAND;
}

static int
cmd_windoweffect(reg_t *p, long narg)
{

	UNUSED(p);

	DPRINTF(("windoweffect: narg = %ld\n", narg));
	_ASSERT(narg >= 1 && narg <= 3);

	/* XXX */

	return STATE_COMMAND;
}
