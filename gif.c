/*	$Id: gif.c,v 1.1 2002/12/05 18:00:44 nonaka Exp $	*/

#if defined(USE_LIBUNGIF)

/*
 * Copyright (c) 2002 NONAKA Kimihiro <aw9k-nnk@asahi-net.or.jp>
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
#include "image.h"

#include <gif_lib.h>

image_t *
get_gif(FILE *fp, long offset, size_t encode_size)
{
	GifFileType *filetypep;
	GifRecordType rectype;
	size_t size;
	image_t *img = NULL;
	unsigned char sig[4];
	int fd;
	int rv;
	int i, j;

	UNUSED(encode_size);

	fseek(fp, offset, SEEK_SET);
	bzero(sig, sizeof(sig));
	size = fread(sig, 1, 3, fp);
	_ASSERT(size == 3);
	_ASSERT(memcmp(sig, "GIF", 3) == 0);

	fseek(fp, offset, SEEK_SET);
	fd = fileno(fp);
	lseek(fd, offset, SEEK_SET);
	filetypep = DGifOpenFileHandle(fd);
	_ASSERT(filetypep != NULL);

	do {
		rv = DGifGetRecordType(filetypep, &rectype);
		_ASSERT(rv == GIF_OK);

		switch (rectype) {
		case IMAGE_DESC_RECORD_TYPE:
		{
			GifRowType *rows;
			ColorMapObject *cmap;
			unsigned char *data;
			int color_count;

			rv = DGifGetImageDesc(filetypep);
			_ASSERT(rv == GIF_OK);
			_ASSERT(filetypep->Image.Width <= 32767);
			_ASSERT(filetypep->Image.Height <= 32767);

			img = image_new();
			img->width = filetypep->Image.Width;
			img->height = filetypep->Image.Height;
			img->bpp = 8 * BPP;
			img->data_size = img->width * img->height * BPP;
			img->data = Ecalloc(img->data_size, 1);

			_ASSERT(!filetypep->Image.Interlace);

			rows = Emalloc(img->height * sizeof(GifRowType *));
			for (i = 0; i < img->height; ++i) {
				rows[i] = Emalloc(img->width * sizeof(GifPixelType));
				rv = DGifGetLine(filetypep,rows[i],img->width);
				_ASSERT(rv == GIF_OK);
			}

			if (filetypep->Image.ColorMap)
				cmap = filetypep->Image.ColorMap;
			else
				cmap = filetypep->SColorMap;
			color_count = cmap->ColorCount;

			/* XXX: trans */
			data = img->data;
			for (i = 0; i < img->height; ++i) {
				for (j = 0; j < img->width; ++j) {
					*data++=cmap->Colors[rows[i][j]].Red;
					*data++=cmap->Colors[rows[i][j]].Green;
					*data++=cmap->Colors[rows[i][j]].Blue;
				}
			}

			for (i = 0; i < img->height; ++i)
				Efree(rows[i]);
			Efree(rows);
		}
			rectype = TERMINATE_RECORD_TYPE;
			break;

		case EXTENSION_RECORD_TYPE:
		{
			GifByteType *ext = NULL;
			int extcode;

			rv = DGifGetExtension(filetypep, &extcode, &ext);
			_ASSERT(rv == GIF_OK);
			for (;ext != NULL; ext = NULL) {
				rv = DGifGetExtensionNext(filetypep, &ext);
				_ASSERT(rv == GIF_OK);
			}
		}
			break;

		case TERMINATE_RECORD_TYPE:
			break;

		case UNDEFINED_RECORD_TYPE:
		case SCREEN_DESC_RECORD_TYPE:
		default:
			/* Can't happen. */
			_ASSERT(rectype != UNDEFINED_RECORD_TYPE);
			_ASSERT(rectype != SCREEN_DESC_RECORD_TYPE);
			_ASSERT(0);
			break;
		}
	} while (rectype != TERMINATE_RECORD_TYPE);

	DGifCloseFile(filetypep);
	fseek(fp, offset, SEEK_SET);

	return img;
}

#endif	/* USE_LIBUNGIF */
