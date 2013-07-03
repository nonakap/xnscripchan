/*	$Id: _parse.c,v 1.16 2002/01/15 17:43:07 nonaka Exp $	*/

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
#include "archive.h"
#include "label.h"
#include "symbol.h"

static int print_token(reg_t *);
static int is_command(reg_t *);

static int state_command(reg_t *);
static int state_arg(reg_t *);
static int state_arg_next(reg_t *);
static int state_check_image(reg_t *);
static int state_check_paren(reg_t *);
static int state_check_token0(reg_t *);
static int state_check_token1(reg_t *);

static int do_exec(reg_t *);

static int (*state_funcp[STATE_NUM])(reg_t *) = {
	NULL,		/* error */
	state_command,	/* command */
	state_arg,	/* arg */
	state_arg_next,	/* arg next */
	state_check_image,
	state_check_paren,
	state_check_token0,
	state_check_token1,
	NULL,		/* eval */
	NULL,		/* exec */
	NULL,		/* end */
	NULL,		/* buttonwait */
	NULL,		/* selectwait */
	NULL,		/* clickwait */
	NULL,		/* clickpagewait */
	NULL,		/* effectwait */
	NULL,		/* nextcommand */
	NULL,		/* waittimer */
};

int token;
int state = STATE_COMMAND;
int next_state = STATE_COMMAND;
long skipline = 0;

static reg_t narg = { TOKEN_NARG, 0, NULL, {0} };
static reg_t command = { TOKEN_UNKNOWN, 0, NULL, {0} };

static int pickup_token;

static int
print_token(reg_t *reg)
{
	switch (reg->type) {
	case TOKEN_COMMAND:
	case TOKEN_STRING:
	case TOKEN_JSTRING:
	case TOKEN_LABEL:
	case TOKEN_FCHK:
		DPRINTF(("%s", reg->u.str));
		break;

	case '&':
	case '<':
	case '>':
	case '=':
	case ',':
	case ':':
	case '_':
	case '@':
		DPRINTF(("%c", reg->type));
		break;

	default:
		break;
	}

	return reg->type;
}

int
parse(void)
{
	reg_t reg;

	if (CMSG->remain > 0) {
		state = display_string();
		return 0;
	}

	token = yylex();
	if (token == 0)
		return 1;

	reg.type = token;
	reg.u.arg = yylval.arg;
	if (state_funcp[state])
		state = (*state_funcp[state])(&reg);
	_ASSERT(state != STATE_ERROR);

	if (state == STATE_EXEC)
		do_exec(&reg);

	if (state == STATE_END)
		return 1;

	if (trapon && is_trap) {
		is_trap = 0;
		label_jump(trap_label + 1);
		state = STATE_COMMAND;
	}

	return 0;
}

static int
is_command(reg_t *reg)
{
	symbol_t *p;

	p = symbol_lookup(reg->u.str);
	if (p == NULL)
		return 0;
	if (p->type != SYMBOL_COMMAND)
		return 0;

	if ((strcasecmp(reg->u.str, "select") == 0)
	    || (strcasecmp(reg->u.str, "selgosub") == 0)
	    || (strcasecmp(reg->u.str, "selnum") == 0)) {
		reg->subtype = TOKEN_SELECT_COMMAND;
	} else {
		reg->subtype = TOKEN_COMMAND;
	}

	reg->type = TOKEN_COMMAND;
	reg->func = p->u.func;
	narg.u.val = 0;

	return 1;
}

static int
state_command(reg_t *reg)
{

	switch (reg->type) {
	case ':':
	case TOKEN_NL:
		return STATE_COMMAND;

	case TOKEN_STRING:
		if (!is_command(reg))
			break;

		DPRINTF(("command: %s ", reg->u.str));
		command = *reg;
		return STATE_ARG;

	case TOKEN_JSTRING:
		display_message(reg->u.str, strlen(reg->u.str));
		Efree(reg->u.str);		/* :-) */
		return STATE_COMMAND;

	case TOKEN_VARNUM: {
		unsigned char buf[256];	/* XXX */

		DPRINTF(("show varnum[%ld] = 0x%lx\n",
		    reg->u.val, varnum_get(reg->u.val)));
		snprintf(buf, sizeof(buf), "%ld", varnum_get(reg->u.val));
		display_message(buf, strlen(buf));
		}
		return STATE_COMMAND;

	case TOKEN_VARSTR: {
		unsigned char *ptr = varstr_get(reg->u.val);

		DPRINTF(("show varstr[%ld] = %s\n", reg->u.val, ptr));
		display_message(ptr, strlen(ptr));
		}
		return STATE_COMMAND;

	case TOKEN_COLOR: {
		unsigned char buf[256];

		DPRINTF(("set text color = 0x%06lx\n", reg->u.val));
		snprintf(buf, sizeof(buf), "#%06lx", reg->u.val);
		display_message(buf, strlen(buf));
		}
		return STATE_COMMAND;

	case '_':
		display_message("_", 1);
		return STATE_COMMAND;

	case '@':
		display_message("@\n", 2);
		return STATE_COMMAND;

	case '\\':
		display_message("\\", 1);
		return STATE_COMMAND;

	case TOKEN_LABEL:
		label_access(reg->u.str + 1);	/* skip '*' */
		Efree(reg->u.str);		/* :-) */
		return STATE_COMMAND;

	case '+':
		/* XXX: ignore */
		/* command: token 43 in line 43024 (月姫) */
		return STATE_COMMAND;

	case TOKEN_NUMBER:
		/* XXX: ignore */
		/* command: token 274 in line 39725 (歌月十夜) */
		return STATE_COMMAND;
	}
	ERRORMSG(("command: token %d in line %d\n", print_token(reg), lineno));
	return STATE_ERROR;
}

static int
state_arg(reg_t *reg)
{

	switch (reg->type) {
	case ':':
		push(&narg);
		push(&command);
		return STATE_EXEC;

	case TOKEN_NL:
		if (command.subtype == TOKEN_SELECT_COMMAND)
			return STATE_ARG;

		push(&narg);
		push(&command);
		return STATE_EXEC;

	case TOKEN_LABEL:
	case TOKEN_COLOR:
	case TOKEN_VARNUM:
	case TOKEN_VARSTR:
	case TOKEN_NUMBER:
		push(reg);
		narg.u.val++;
		DPRINTF(("%d", print_token(reg)));
		return STATE_ARG_NEXT;

	case TOKEN_FCHK:
		push(reg);
		narg.u.val++;
		DPRINTF(("fchk"));
		return STATE_ARG;

	case TOKEN_LCHK:
		push(reg);
		narg.u.val++;
		DPRINTF(("lchk"));
		return STATE_ARG;

	case TOKEN_JSTRING:
		display_message(reg->u.str, strlen(reg->u.str));
		Efree(reg->u.str);	/* :-) */

		push(&narg);
		push(&command);
		return STATE_EXEC;

	case TOKEN_STRING: {
		symbol_t *p;

		p = symbol_lookup(reg->u.str);
		if (p != NULL) {
			switch (p->type) {
			case SYMBOL_NUMALIAS:
				Efree(reg->u.str);
				reg->u.str = NULL;

				reg->type = TOKEN_NUMBER;
				reg->u.val = p->u.val;
				break;

			case SYMBOL_STRALIAS:
				Efree(reg->u.str);
				reg->u.str = NULL;

				reg->type = TOKEN_STRING;
				reg->u.str = varstr_get(p->u.val);
				break;

			case SYMBOL_COMMAND:
				/* XXX: rmenu の reset でひっかかるのね… */
				break;
			}
		}
		}

		push(reg);
		narg.u.val++;
		DPRINTF(("%d", print_token(reg)));
		return STATE_ARG_NEXT;

	case '(':
		/* XXX */
		return STATE_CHECK_IMAGE;
	}
	ERRORMSG(("arg: token %d in line %d\n", print_token(reg), lineno));
	return STATE_ERROR;
}

static int
state_arg_next(reg_t *reg)
{
	reg_t tmpcmd, tmparg;

	switch (reg->type) {
	case ':':
	case TOKEN_NL:
		push(&narg);
		push(&command);
		return STATE_EXEC;

	case ',':
		DPRINTF((", "));
		return STATE_ARG;

	case TOKEN_STRING:
		tmpcmd = *reg;
		tmparg = narg;

		if (!is_command(reg))
			break;

		push(&tmparg);
		push(&command);

		narg = tmparg;
		*reg = tmpcmd;
		return STATE_EXEC;

	case TOKEN_JSTRING:
		display_message(reg->u.str, strlen(reg->u.str));
		Efree(reg->u.str);	/* :-) */

		push(&narg);
		push(&command);
		return STATE_EXEC;

	case '&':
	case TOKEN_AND:
		DPRINTF((" && "));
		return STATE_ARG;

	case '<':
	case '>':
	case '=':
	case TOKEN_EQ:
	case TOKEN_NE:
	case TOKEN_GE:
	case TOKEN_LE:
		push(reg);
		narg.u.val++;
		DPRINTF((" %d ", print_token(reg)));
		return STATE_ARG;

	case '\\':	/* XXX */
		return STATE_ARG_NEXT;
	}
	ERRORMSG(("arg_next: token %d in line %d\n", print_token(reg), lineno));
	return STATE_ERROR;
}

/*
 * ダサすぎ…
 */
static int
state_check_image(reg_t *reg)
{

	switch (reg->type) {
	case TOKEN_STRING:
		pickup_token = !archive_is_access(core->arc, reg->u.str);
		return STATE_CHECK_PAREN;

	case TOKEN_VARSTR:
		pickup_token = !archive_is_access(core->arc,
		    varstr_get(reg->u.val));
		return STATE_CHECK_PAREN;
	}
	ERRORMSG(("image: token %d in line %d\n", print_token(reg), lineno));
	return STATE_ERROR;
}

static int
state_check_paren(reg_t *reg)
{

	switch (reg->type) {
	case ')':
		return STATE_CHECK_TOKEN0;
	}
	ERRORMSG(("paren: token %d in line %d\n", print_token(reg), lineno));
	return STATE_ERROR;
}

static int
state_check_token0(reg_t *reg)
{

	switch (reg->type) {
	case TOKEN_STRING:
	case TOKEN_VARSTR:
		if (pickup_token == 0) {
			push(reg);
			narg.u.val++;
		}
		return STATE_CHECK_TOKEN1;
	}
	ERRORMSG(("token0: token %d in line %d\n", print_token(reg), lineno));
	return STATE_ERROR;
}

static int
state_check_token1(reg_t *reg)
{

	switch (reg->type) {
	case TOKEN_STRING:
	case TOKEN_VARSTR:
		if (pickup_token == 1) {
			push(reg);
			narg.u.val++;
		}
		return STATE_ARG_NEXT;
	}
	ERRORMSG(("token1: token %d in line %d\n", print_token(reg), lineno));
	return STATE_ERROR;
}

static int
do_exec(reg_t *reg)
{
	reg_t cmd, narg;
	reg_t *p;
	int i, n;

	DPRINTF(("\n"));

	cmd = pop();
	narg = pop();
	_ASSERT(cmd.type == TOKEN_COMMAND);
	_ASSERT(cmd.func != NULL);
	_ASSERT(narg.type == TOKEN_NARG);

	n = narg.u.val;
	if (n > 0) {
		p = (reg_t *)Emalloc(sizeof(reg_t) * n);
		for (i = n - 1; i >= 0; i--) {
			p[i] = pop();
		}
	} else {
		p = NULL;
	}

	DPRINTF(("[lineno = %d] ", lineno));
	state = cmd.func(p, n);
	_ASSERT(state != STATE_ERROR);
	switch (state) {
	case STATE_NEXTCOMMAND:
		state = state_command(reg);
		break;

	default:
		break;
	}

	Efree(cmd.u.str);
	for (i = 0; i < n; i++) {
		switch (p[i].type) {
		case TOKEN_STRING:
		case TOKEN_JSTRING:
		case TOKEN_LABEL:
			Efree(p[i].u.str);
			break;
		}
	}
	if (n > 0) {
		Efree(p);
	}

	return 0;
}
