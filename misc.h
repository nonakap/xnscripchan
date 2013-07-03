/*	$Id: misc.h,v 1.4 2002/01/11 17:02:47 nonaka Exp $	*/

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

#ifndef	__MISC_H__
#define	__MISC_H__

enum {
	OBJECT_CORE = 0,
	OBJECT_ARCHIVE,
	OBJECT_SCENARIO,
	OBJECT_WAVE,
	OBJECT_IMAGE,
	OBJECT_IMAGE_GGD,
	OBJECT_IMAGE_GG0,
	OBJECT_IMAGE_JPEG,
	OBJECT_IMAGE_BMP,
	OBJECT_IMAGE_PNG,
	OBJECT_VRAM,
	OBJECT_UNKNOWN
};

struct _object_tag {
	int type;
	int ref_count;
};
typedef struct _object_tag object_t;

#if defined(NDEBUG)

void	*Emalloc(size_t size);
void	*Ecalloc(size_t number, size_t size);
void	*Erealloc(void *ptr, size_t size);
void	Efree(void *ptr);
char	*Estrdup(const char *str);

#else /* !NDEBUG */

#define	Emalloc(s)	Emalloc_func((s), __FILE__, __LINE__)
#define	Ecalloc(n,s)	Ecalloc_func((n), (s), __FILE__, __LINE__)
#define	Erealloc(p,s)	Erealloc_func((p), (s), __FILE__, __LINE__)
#define	Efree(p)	Efree_func((p), __FILE__, __LINE__)
#define	Estrdup(s)	Estrdup_func((s), __FILE__, __LINE__)

void	*Emalloc_func(size_t size, const char *file, int line);
void	*Ecalloc_func(size_t number, size_t size, const char *file,
	    int line);
void	*Erealloc_func(void *ptr, size_t size, const char *file,
	    int line);
void	Efree_func(void *ptr, const char *file, int line);
char	*Estrdup_func(const char *str, const char *file, int line);

#endif /* NDEBUG */

#if defined(WORS_BIGENDIAN)

#define	get_mem_16	get_mem_be16
#define	get_mem_24	get_mem_be24
#define	get_mem_32	get_mem_be32
#define	put_mem_16	put_mem_be16
#define	put_mem_24	put_mem_be24
#define	put_mem_32	put_mem_be32

#else /* !WORS_BIGENDIAN */

#define	get_mem_16	get_mem_le16
#define	get_mem_24	get_mem_le24
#define	get_mem_32	get_mem_le32
#define	put_mem_16	put_mem_le16
#define	put_mem_24	put_mem_le24
#define	put_mem_32	put_mem_le32

#endif /* WORS_BIGENDIAN */

unsigned char	get_byte(FILE *fp);
void		get_bytes(FILE *fp, unsigned char *buf, size_t size);
size_t		get_string(FILE *fp, unsigned char *buf, size_t size);
unsigned short	get_le16(FILE *fp);
unsigned int	get_le32(FILE *fp);
unsigned short	get_be16(FILE *fp);
unsigned int	get_be32(FILE *fp);
void		put_byte(FILE *fp, int v);
void		put_le16(FILE *fp, unsigned short v);
void	 	put_le32(FILE *fp, unsigned int v);
unsigned short	get_mem_le16(unsigned char *p);
unsigned int	get_mem_le24(unsigned char *p);
unsigned int	get_mem_le32(unsigned char *p);
unsigned short	get_mem_be16(unsigned char *p);
unsigned int	get_mem_be24(unsigned char *p);
unsigned int	get_mem_be32(unsigned char *p);
void		put_mem_le16(unsigned char *p, unsigned short v);
void		put_mem_le24(unsigned char *p, unsigned int v);
void		put_mem_le32(unsigned char *p, unsigned int v);

unsigned short	sjis2euc(unsigned short sjis);
unsigned char	*sjis2euc_str(unsigned char *str);
void		mem_dump(const unsigned char *p, size_t size,
		    const unsigned char *prefix);
int		calc_hash(char *s, int num);

#endif /* __PROJECT_MISC_H__ */
