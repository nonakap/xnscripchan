/*	$Id: main.c,v 1.12 2002/01/09 18:14:34 nonaka Exp $	*/

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

#include <locale.h>

#include "nscr.h"
#include "archive.h"
#include "label.h"

nscr_core_t _core;
nscr_core_t *core = &_core;

int trapon;
int is_trap;
unsigned char *trap_label;

int
main(int argc, char *argv[])
{
	struct timeval tv;
	object_t *arc;
	FILE *fp;

	setlocale(LC_ALL, "");
	gettimeofday(&tv, NULL);
	srandom(tv.tv_sec);

	if (argc > 1) {
		fp = fopen(argv[1], "rb");
		if (fp != NULL) {
			/* 引数で指定されるスクリプトはエンコードされていない */
			is_encoded = 0;
			goto script_read;
		}
	}

	fp = fopen("0.txt", "rb");
	if (fp != NULL) {
		/* 本当は 1.txt... を読み込まなければならない */
		is_encoded = 0;
		goto script_read;
	}

	fp = fopen("nscript.dat", "rb");
	if (fp != NULL) {
		is_encoded = 1;
		goto script_read;
	}

	fprintf(stderr, "Can't read script file.\n");
	exit(1);

script_read:
	yyin = fp;
	prescan(yyin);

	arc = archive_new(0, NULL);
	core->arc = arc;

	command_init();
	image_init();
	screen_init();

	if (gui_init(SCREEN_WIDTH, SCREEN_HEIGHT)) {
		fprintf(stderr, "gui_init() error\n");
		exit(1);
	}

	reset();
	core->block = BLOCK_DEFINE;
	label_jump("define");
	engine_exec();

	gui_close();

	save_globalvar();
	save_fchklog(core->arc);
	save_labellog();

	fclose(fp);
	return 0;
}

void
reset(void)
{
	long v[16];

	init_stack();
	init_var();

	timer_term();
	button_destroy();
	select_destroy();

	trapon = 0;
	is_trap = 0;
	if (trap_label) {
		Efree(trap_label);
		trap_label = NULL;
	}

	v[0] = 8;
	v[1] = 16;
	v[2] = 23;
	v[3] = 16;
	v[4] = 26;
	v[5] = 26;
	v[6] = 0;
	v[7] = 2;
	v[8] = 20;
	v[9] = 1;
	v[10] = 1;
	v[11] = 0x999999;
	v[12] = 0;
	v[13] = 0;
	v[14] = 639;
	v[15] = 479;
	text_setwindow(v, 16, NULL);

	sys_alloc_color(0xffffff, &CTEXT->color);
	sys_alloc_color(0xffffff, &core->selection.color[0]);
	sys_alloc_color(0x999999, &core->selection.color[1]);

	state = STATE_COMMAND;
}

void
yyerror(const char *str)
{

	ERRORMSG(("yyerror: %s\n", str));
}
