/*	$Id: var.c,v 1.9 2002/01/08 18:13:27 nonaka Exp $	*/

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

#define	NVAR	10000

int is_globalon = 0;
unsigned char globalsav_filename[PATH_MAX] = "gloval.sav";

static long varnum[NVAR];
static long varnum_min[NVAR];
static long varnum_max[NVAR];
static unsigned char *varstr[NVAR];

void
init_var(void)
{
	int i;

	for (i = 0; i < NVAR; i++) {
		if (varstr[i] != NULL)
			Efree(varstr[i]);
	}

	for (i = 0; i < NVAR; i++) {
		varnum[i] = 0;
		varnum_min[i] = INT_MIN;
		varnum_max[i] = INT_MAX;
		varstr[i] = (unsigned char *)Estrdup("");
	}
}

void
varnum_set(long no, long val)
{

	_ASSERT(no >= 0 && no < NVAR);

	if (val < varnum_min[no])
		val = varnum_min[no];
	else if (val > varnum_max[no])
		val = varnum_max[no];
	varnum[no] = val;
}

long
varnum_get(long no)
{
	long val;

	_ASSERT(no >= 0 && no < NVAR);

	/* いちおー */

	val = varnum[no];
	if (val < varnum_min[no])
		val = varnum_min[no];
	else if (val > varnum_max[no])
		val = varnum_max[no];
	return val;
}

void
varnum_set_limit(long no, long minnum, long maxnum)
{

	_ASSERT(no >= 0 && no < NVAR);
	_ASSERT(minnum < maxnum);

	varnum_min[no] = minnum;
	varnum_max[no] = maxnum;
}

void
varstr_set(long no, unsigned char *str)
{
	size_t len_dest, len_src;

	_ASSERT(no >= 0 && no < NVAR);

	len_dest = strlen(varstr[no]) + 1;
	len_src = strlen(str) + 1;
	if (len_dest < len_src)
		varstr[no] = (unsigned char *)Erealloc(varstr[no], len_src);
	strlcpy(varstr[no], str, len_src);
}

unsigned char *
varstr_get(long no)
{

	_ASSERT(no >= 0 && no < NVAR);

	return varstr[no];
}

void
varstr_join(long no, unsigned char *str)
{
	size_t len;

	_ASSERT(no >= 0 && no < NVAR);
	_ASSERT(str != NULL);

	len = strlen(varstr[no]) + strlen(str) + 1;
	varstr[no] = (unsigned char *)Erealloc(varstr[no], len);
	strlcat(varstr[no], str, len);
}

int
save_globalvar(void)
{
	FILE *fp;
	size_t s, size;
	int i;

	if (!is_globalon)
		return 0;

	fp = fopen(globalsav_filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "save: can't open %s\n", globalsav_filename);
		return 1;
	}

	for (i = GLOBALVAR_MIN; i < GLOBALVAR_MAX; i++) {
		put_le32(fp, varnum[i]);

		size = strlen(varstr[i]) + 1;	/* +1 for NUL */
		for (s = 0; s < size; s++)
			put_byte(fp, varstr[i][s]);
	}

	fclose(fp);

	return 0;
}

int
load_globalvar(void)
{
	FILE *fp;
	unsigned char *str;
	long off, size;
	int i;

	if (!is_globalon)
		return 0;

	fp = fopen(globalsav_filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "load: can't open %s\n", globalsav_filename);
		return 1;
	}

	for (i = GLOBALVAR_MIN; i < GLOBALVAR_MAX; i++) {
		varnum[i] = get_le32(fp);

		off = ftell(fp);
		while (get_byte(fp) != '\0')
			continue;
		size = ftell(fp) - off + 1;	/* +1 for NUL */
		str = (unsigned char *)Emalloc(size);

		fseek(fp, off, SEEK_SET);
		get_string(fp, str, size);

		if (varstr[i] != NULL)
			Efree(varstr[i]);
		varstr[i] = str;
	}

	return 0;
}
