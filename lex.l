%{
/*	$Id: lex.l,v 1.15 2002/12/09 16:51:42 nonaka Exp $	*/

/*
 * Copyright (c) 2001 NONAKA Kimihiro <aw9k-nnk@asahi-net.or.jp>
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

#include <unistd.h>

#include "nscr.h"
#include "symbol.h"

#ifndef TEXTBUFSIZE
#define	TEXTBUFSIZE	128
#endif /* TEXTBUFSIZE */

static int var_subr(int, unsigned char **, long *);
static int yylex_subr(void);

int lineno = 1;
int is_encoded = 0;

#undef	YY_INPUT
#define YY_INPUT(buf,result,max_size)					\
	if (yy_current_buffer->yy_is_interactive) {			\
		int c = '*', n;						\
		for (n = 0;						\
		    n < max_size && (c = getc(yyin)) != EOF;		\
		    ++n ) {						\
			buf[n] = (char)(is_encoded ? (c ^ 0x84) : c);	\
			if (buf[n] == '\n') {				\
				n++;					\
				break;					\
			}						\
		}							\
		if (c == EOF && ferror(yyin)) {				\
			YY_FATAL_ERROR("input in flex scanner failed");	\
		}							\
		result = n;						\
	} else {							\
		if (((result = fread(buf, 1, max_size, yyin)) == 0)	\
		    && ferror(yyin)) {					\
			YY_FATAL_ERROR("input in flex scanner failed");	\
		} else {						\
			if (is_encoded) {				\
				int s;					\
				for (s = 0; s < result; s++)		\
					buf[s] ^= 0x84;			\
			}						\
		}							\
	}
%}
num	[0-9]
alpha	[A-Za-z]
_alpha	[_A-Za-z]
alnum	[0-9A-Za-z]
_alnum	[_0-9A-Za-z]
%%
[ \t]	;
;.*	/* comment */;
^~	/* label */;
^\n$	lineno++;

==	return TOKEN_EQ;
!=|<>	return TOKEN_NE;
&&	return TOKEN_AND;
\<=	return TOKEN_LE;
>=	return TOKEN_GE;
fchk	return TOKEN_FCHK;
lchk	return TOKEN_LCHK;

\$|% {
	reg_t reg;

	bzero(&reg, sizeof(reg));
	reg.type = yytext[0] == '$' ? TOKEN_SYMBOL_VARSTR : TOKEN_SYMBOL_VARNUM;
	push(&reg);
}

!{alnum}* {
	yylval.str = Estrdup(yytext);
	return TOKEN_JSTRING;
}

\*{_alpha}[^ \t\n]* {
	yylval.str = Estrdup(yytext);
	return TOKEN_LABEL;
}

-?{num}+ {
	unsigned char *s;
	long val;
	int kind;

	val = atol(yytext);
	s = NULL;
	kind = var_subr(TOKEN_NUMBER, &s, &val);

	switch (kind) {
	case TOKEN_NUMBER:
	case TOKEN_VARNUM:
	case TOKEN_VARSTR:
		yylval.val = val;
		break;

	default:
		_ASSERT(0);
		return 0;
	}
	return kind;
}

{alpha}{_alnum}* {
	unsigned char *s;
	long val;
	int kind;

	val = 0;
	s = yytext;
	kind = var_subr(TOKEN_STRING, &s, &val);

	switch (kind) {
	case TOKEN_STRING:
		yylval.str = Estrdup(s);
		break;

	case TOKEN_VARSTR:
	case TOKEN_VARNUM:
		yylval.val = val;
		break;

	default:
		_ASSERT(0);
		return 0;
	}
	return kind;
}

#{alnum}{alnum}{alnum}{alnum}{alnum}{alnum}/[^{alnum}] {
	/* skip '#' */
	yylval.val = strtol(yytext + 1, NULL, 16);
	return TOKEN_COLOR;
}

\"[^\"\n]*\" {
	/* nuke '"' */
	yytext[yyleng - 1] = '\0';

	/* skip '"' */
	yylval.str = Estrdup(yytext + 1);
	return TOKEN_STRING;
}

\"[^\"\n]*/\n {
	unsigned char buf[256];
	int len;

	snprintf(buf, sizeof(buf), "unterminated string in line %d\n", lineno);
	yyerror(buf);

	len = strlen(yytext + 1);
	if (len != 0) {
		/* skip '"' */
		yylval.str = Estrdup(yytext + 1);

		return TOKEN_STRING;
	}
}

\n {
	lineno++;
	return TOKEN_NL;
}

.	return yylex_subr();
%%
static int
var_subr(int kind, unsigned char **s, long *val)
{
	reg_t reg;
	symbol_t *p;

	for (;;) {
		reg = pop();
		switch (reg.type) {
		case TOKEN_SYMBOL_VARNUM:
			switch (kind) {
			case TOKEN_NUMBER:
				break;

			case TOKEN_STRING:
				p = symbol_lookup(CALIAS->symbol, *s);
				_ASSERT(p != NULL);
				*val = p->u.val;
				break;

			case TOKEN_VARSTR:
				*s = varstr_get(*val);
				p = symbol_lookup(CALIAS->symbol, *s);
				_ASSERT(p != NULL);
				*val = p->u.val;
				break;

			case TOKEN_VARNUM:
				*val = varnum_get(*val);
				break;

			default:
				_ASSERT(0);
				return 0;
			}
			kind = TOKEN_VARNUM;
			break;

		case TOKEN_SYMBOL_VARSTR:
			switch (kind) {
			case TOKEN_NUMBER:
				break;

			case TOKEN_STRING:
				p = symbol_lookup(CALIAS->symbol, *s);
				_ASSERT(p != NULL);
				*val = p->u.val;
				break;

			case TOKEN_VARSTR:
				*s = varstr_get(*val);
				p = symbol_lookup(CALIAS->symbol, *s);
				_ASSERT(p != NULL);
				*val = p->u.val;
				break;

			case TOKEN_VARNUM:
				*val = varnum_get(*val);
				break;

			default:
				_ASSERT(0);
				return 0;
			}
			kind = TOKEN_VARSTR;
			break;

		default:
			push(&reg);
			goto out;
		}
	}

out:
	return kind;
}

static int
yylex_subr(void)
{
	int c = (unsigned char)yytext[0];

	if (c & 0x80) {
		unsigned char *p;
		int iskanji2nd = 0;
		int n = 1;
		int off = 0;
		unsigned char w;

		p = (unsigned char *)Emalloc(TEXTBUFSIZE);
		do {
			if (!iskanji2nd) {
				if ((c & 0x80) == 0)
					break;

				w = c ^ 0x40;
				if ((w >= 0xa0) && (w <= 0xdf)) {
					p[off++] = c;
					iskanji2nd = 1;
				} else {
					_ASSERT(0);
					yyerror("broken SJIS code\n");
					break;
				}
			} else {
				if ((c >= 0x40 && c <= 0x7e)
				    || (c >= 0x80 && c <= 0xfc)) {
					p[off++] = c;
					iskanji2nd = 0;
				} else {
					_ASSERT(0);
					yyerror("broken SJIS code\n");
					break;
				}
			}

			if (off + 1 >= TEXTBUFSIZE * n) {
				n++;
				p = Erealloc(p, TEXTBUFSIZE * n);
			}
		} while ((c = input()) != NULL);
		unput(c);
		p[off] = '\0';

		yylval.str = p;
		return TOKEN_JSTRING;
	}
	return c;
}

long
flex_get_pos(void)
{

	return ftell(yyin)
	    - (yy_n_chars - (yy_c_buf_p - YY_CURRENT_BUFFER->yy_ch_buf));
}

void
skiplines(int line)
{

	if (line == 0)
		return;

	_ASSERT(0 < lineno + line && lineno + line < scan_lineno);
	DSNAP(("lineno = %d -> %d\n", lineno, lineno + line));

	fseek(yyin, linepos[lineno + line], SEEK_SET);
	lineno += line;
	yyrestart(yyin);
}
