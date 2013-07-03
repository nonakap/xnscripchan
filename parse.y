%{
/*	$Id: parse.y,v 1.7 2002/01/18 18:23:39 nonaka Exp $	*/

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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(char *);
%}
%union {
	void *arg;
	char *str;
	long val;
}

%token TOKEN_UNKNOWN
%token TOKEN_STATE
%token TOKEN_NL TOKEN_AND TOKEN_GE TOKEN_LE TOKEN_EQ TOKEN_NE
%token TOKEN_FCHK TOKEN_LCHK
%token TOKEN_SYMBOL_VARNUM TOKEN_SYMBOL_VARSTR
%token <str> TOKEN_COMMAND TOKEN_STRING TOKEN_JSTRING TOKEN_LABEL
%token <val> TOKEN_NUMBER TOKEN_VARNUM TOKEN_VARSTR TOKEN_COLOR
%token <val> TOKEN_NARG TOKEN_OFFSET TOKEN_LINENO
%%
script		:
		;
%%
