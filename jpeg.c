/*	$Id: jpeg.c,v 1.4 2002/01/13 21:43:46 nonaka Exp $	*/

/*
 * Hanashizume no Matsuri for X
 * Copyright (c) 2000 TF <tf@denpa.org>
 * All rights reserverd.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by TF.
 * 4. The name of the author may not be used to endorse or promote products
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
 *
 * xhana-Id: jpeg.c,v 1.5 2001/01/30 16:53:25 tf Exp
 */

#include "nscr.h"

#include <jconfig.h>
#include <jpeglib.h>

#define PPM_MAXVAL 255
#define BYTESPERSAMPLE 1
#define SIZEOF(object) ((size_t) sizeof(object))

typedef struct djpeg_dest_struct *djpeg_dest_ptr;

struct djpeg_dest_struct {
  JMETHOD(void, start_output, (j_decompress_ptr cinfo,
			       djpeg_dest_ptr dinfo));
  JMETHOD(void, put_pixel_rows, (j_decompress_ptr cinfo,
				 djpeg_dest_ptr dinfo,
				 JDIMENSION rows_supplied));
  JMETHOD(void, finish_output, (j_decompress_ptr cinfo,
				djpeg_dest_ptr dinfo));
  FILE * output_file;
  JSAMPARRAY buffer;
  JDIMENSION buffer_height;
};

typedef struct {
  struct djpeg_dest_struct pub;	/* public fields */

  /* Usually these two pointers point to the same place: */
  char *iobuffer;		/* fwrite's I/O buffer */
  JSAMPROW pixrow;		/* decompressor output buffer */
  size_t buffer_width;		/* width of I/O buffer */
  JDIMENSION samples_per_row;	/* JSAMPLEs per output row */
  int current_row;
} ppm_dest_struct;

typedef ppm_dest_struct *ppm_dest_ptr;

extern djpeg_dest_ptr jinit_write_ppm JPP((j_decompress_ptr cinfo));

static char *outimg;

image_t *
get_jpeg(FILE *input_file, long offset, size_t encode_size)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  djpeg_dest_ptr dest_mgr;
  JDIMENSION num_scanlines;
  image_t *p;

  _ASSERT(input_file != NULL);
  _ASSERT(encode_size > 0);

  UNUSED(encode_size);

  dest_mgr = NULL;

  /* Initialize the JPEG decompression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  fseek(input_file, offset, SEEK_SET);

  /* Specify data source for decompression */
  jpeg_stdio_src(&cinfo, input_file);

  /* Read file header, set default decompression parameters */
  jpeg_read_header(&cinfo, TRUE);

  p = image_new();
  p->width = cinfo.image_width;
  p->height = cinfo.image_height;
  p->bpp = 8 * BPP;
  p->data_size = p->width * p->height * BPP;
  outimg = (char *)Ecalloc(p->data_size, 1);

  dest_mgr = jinit_write_ppm(&cinfo);
  dest_mgr->output_file = stdout;

  /* Start decompressor */
  jpeg_start_decompress(&cinfo);

  /* Process data */
  while (cinfo.output_scanline < cinfo.output_height) {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
					dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows)(&cinfo, dest_mgr, num_scanlines);
  }

  /* Finish decompression and release memory.
   * I must do it in this order because output module has allocated memory
   * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
   */
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  p->data = outimg;

  return p;
}

METHODDEF(void)
put_pixel_rows(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr)dinfo;

  UNUSED(cinfo);
  UNUSED(rows_supplied);

  memmove(outimg + dest->buffer_width * dest->current_row, dest->iobuffer, dest->buffer_width);
  dest->current_row++;
}

/*
 * The module selection routine for PPM format output.
 */
GLOBAL(djpeg_dest_ptr)
jinit_write_ppm(j_decompress_ptr cinfo)
{
  ppm_dest_ptr dest;

  /* Create module interface object, fill in method pointers */
  dest = (ppm_dest_ptr)
      (*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_IMAGE,
				  SIZEOF(ppm_dest_struct));

  /* Calculate output image dimensions so we can allocate space */
  jpeg_calc_output_dimensions(cinfo);

  /* Create physical I/O buffer.  Note we make this near on a PC. */
  dest->samples_per_row = cinfo->output_width * cinfo->out_color_components;
  dest->buffer_width = dest->samples_per_row * (BYTESPERSAMPLE * SIZEOF(char));
  dest->iobuffer = (char *)(*cinfo->mem->alloc_small)
    ((j_common_ptr)cinfo, JPOOL_IMAGE, dest->buffer_width);
  dest->current_row = 0;

  /* We will fwrite() directly from decompressor output buffer. */
  /* Synthesize a JSAMPARRAY pointer structure */
  /* Cast here implies near->far pointer conversion on PCs */
  dest->pixrow = (JSAMPROW) dest->iobuffer;
  dest->pub.buffer = & dest->pixrow;
  dest->pub.buffer_height = 1;
  dest->pub.put_pixel_rows = put_pixel_rows;
  
  return (djpeg_dest_ptr)dest;
}
