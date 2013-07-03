/*	$Id: archive.h,v 1.2 2002/01/09 18:14:34 nonaka Exp $	*/

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

#ifndef	__NSCR_ARCHIVE_H__
#define	__NSCR_ARCHIVE_H__

#include <sys/types.h>

#include "misc.h"

struct _arcfile_tag {
	object_t		obj;

	FILE			**fp;

	unsigned char		*name;
	long			offset;
	size_t			size;

	int			is_compress;
	size_t			decode_size;

	int			access;

	struct _arcfile_tag	*prev;
	struct _arcfile_tag	*next;
};
typedef struct _arcfile_tag arcfile_t;

struct file_desc {
	FILE		*fp;
	unsigned char	name[PATH_MAX];
};
typedef struct file_desc file_desc_t;

struct _archive_tag {
	object_t	obj;

	file_desc_t	**fd;
	int		fd_num;
	int		fd_max;

	int		(*calc_hash)(char *, int);
	int 		hash_num;

	int		*table_count;
	arcfile_t	**table;
};
typedef struct _archive_tag archive_t;

struct _arcquery_tag {
	object_t	obj;

	FILE		*fp;
	long		offset;
	size_t		size;

	int		is_compress;
	size_t		decode_size;
};
typedef struct _arcquery_tag arcquery_t;

object_t	*archive_new(int hash_num, int (*calc_func)(char *, int));
int		archive_open(object_t *obj, unsigned char *arc_name);
void		archive_delete(object_t *obj);
int		archive_query(object_t *obj, unsigned char *name,
		    arcquery_t *aqp);
int		archive_is_access(object_t *obj, unsigned char *name);
void		archive_print(object_t *obj);
int		archive_extract(object_t *obj, unsigned char *name);

#endif /* __NSCR_ARCHIVE_H__ */
