/*
 * io_jpeg.c
 *
 * Jim Stewart
 * 11 Jun 2002
 *
 * Based on io_pnm.c from iomedley and example.c from libjpeg-6b source.
 *
 */

#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#include <sys/types.h>
#include <string.h>
#include "iomedley.h"
#include <jpeglib.h>
#include <setjmp.h>

/* JPEG magic number taken from Linux magic file. */

#define JPEG_MAGIC	"\xff\xd8"

void	my_error_exit(j_common_ptr);
int	iom_isJPEG(FILE *);
int	iom_GetJPEGHeader(FILE *, char *, struct iom_iheader *);
int	iom_ReadJPEG(FILE *, char *, int *, int *, int *, int *, unsigned char **);
int	iom_WriteJPEG(char *, unsigned char *, struct iom_iheader *, int);

/* Setup structs used for jpeglib error handling.  By default jpeglib will exit() on error. */

struct my_error_mgr {

  struct jpeg_error_mgr	pub;
  jmp_buf		setjmp_buffer;

};

typedef struct my_error_mgr	*my_error_ptr;

void
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr	myerr = (my_error_ptr) cinfo->err;

  /* FIX: display error message? */
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

/* */

int
iom_isJPEG(FILE *fp)
{

  /* Returns 1 if fp is a JPEG file, 0 otherwise. */

  unsigned char	magic[2];
  int		i, c;

  rewind(fp);

  for (i = 0; i < 2; i++) {
    if ((c = fgetc(fp)) == EOF)
      return 0;
    magic[i] = (unsigned char) c;
  }

  if (!strncmp(magic, JPEG_MAGIC, 2)) {
    return 1;
  } else {
    return 0;
  }

}

int
iom_GetJPEGHeader(FILE *fp, char *filename, struct iom_iheader *h)
{

  int		x, y, z, bits;
  unsigned char	*data;

  if (!iom_ReadJPEG(fp, filename, &x, &y, &z, &bits, &data)) {
    return 0;
  }

  iom_init_iheader(h);

  if (z == 1) {
    h->size[0] = x;
    h->size[1] = y;
    h->size[2] = z;
    h->org = iom_BSQ;
  } else {		/* z == 3 */
    h->size[0] = z;
    h->size[1] = x;
    h->size[2] = y;
    h->org = iom_BIP;
  }

  /* Bits in sample should always be 8 unless libjpeg is compiled strangely. */
  h->eformat = iom_MSB_INT_1;
  h->format = iom_BYTE;

  h->data = data;

  return 1;

}

int
iom_ReadJPEG(FILE *fp, char *filename, int *xout, int *yout, int *zout, int *bits, unsigned char **dout)
{

  struct jpeg_decompress_struct	cinfo;
  struct my_error_mgr		jerr;
  JSAMPARRAY			buffer;
  int				x, y, z;
  int				row_stride;	/* Bytes per scanline. */
  int				header_ok;
  unsigned char			*data;

  /* Setup error handling. */

  cinfo.err = jpeg_std_error(&jerr.pub);	/* FIX: tie into iomedley's error routines? */
  jerr.pub.error_exit = my_error_exit;

  if (setjmp(jerr.setjmp_buffer)) {
    /* All jpeglib errors will jump to here; cleanup and return.  Caller closes file. */
    jpeg_destroy_decompress(&cinfo);
    return 0;
  }

  jpeg_create_decompress(&cinfo);

  /* Rewind file, read header, and start decompression. */

  rewind(fp);
  jpeg_stdio_src(&cinfo, fp);

  header_ok = jpeg_read_header(&cinfo, TRUE);
  if (header_ok != JPEG_HEADER_OK) {
    jpeg_destroy_decompress(&cinfo);
    return 0;
  }

  (void) jpeg_start_decompress(&cinfo);

  x = cinfo.output_width;
  y = cinfo.output_height;
  z = cinfo.output_components;

  /* buffer is a sampling array that holds one row of image data. */

  row_stride = x * z;
  buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  data = (unsigned char *) malloc(x * y * z);

  /* Read lines one at a time info buffer. */
  /* FIX: read more at once? */

  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);	/* FIX: check return value? */
    memcpy(data + (cinfo.output_scanline - 1) * row_stride, buffer[0], row_stride);
  }

  /* Cleanup.  Caller closes file. */

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  *xout = x;
  *yout = y;
  *zout = z;
  *bits = 8;	/* This should always be true unless libjpeg is compiled bizarrely.  FIX: Not sure how to check. */
  *dout = data;

  return 1;

}

int
iom_WriteJPEG(char *filename, unsigned char *indata, struct iom_iheader *h, int force)
{

  int		x, y, z;
  JSAMPROW	scanline[1];
  FILE		*fp = NULL;
  unsigned char *data;

  struct jpeg_compress_struct	cinfo;
  struct jpeg_error_mgr 	jerr;

  if (h->format != iom_BYTE) {
    if (iom_is_ok2print_unsupp_errors()) {
      fprintf(stderr, "Cannot write %s data in a JPEG file.\n",
	      iom_FORMAT2STR[h->format]);
    }
    return 0;
  }

  z = iom_GetBands(h->size, h->org);

  /* Make sure data is 1-band (monochrome) or 3-band (RGB). */

  if (z != 1 && z != 3) {
    if (iom_is_ok2print_unsupp_errors()) {
      fprintf(stderr, "Cannot write JPEG files with depths other than 1 or 3.\n");
      fprintf(stderr, "See file %s line %d.\n", __FILE__, __LINE__);
    }
    return 0;
  }

  /* Convert data to BIP if not already BIP. */

  if (h->org == iom_BIP) {
    data = indata;
  } else {
    /* iom__ConvertToBIP allocates memory, don't forget to free it! */
    if (!iom__ConvertToBIP(indata, h, &data)) {
      return 0;
    }
  }

  /* Check for file existance if force overwrite not set. */

  if (!force && access(filename, F_OK) == 0) {
    if (iom_is_ok2print_errors()) {
      fprintf(stderr, "File %s already exists.\n", filename);
    }
    if (h->org != iom_BIP) {
      free(data);
    }
    return 0;
  }
  
  if ((fp = fopen(filename, "wb")) == NULL) {
    if (iom_is_ok2print_sys_errors()) {
      fprintf(stderr, "Unable to write file %s. Reason: %s.\n",
	      filename, strerror(errno));
    }
    if (h->org != iom_BIP) {
      free(data);
    }
    return 0;
  }

  /* Use libjpeg to write JPEG file. */

  cinfo.err = jpeg_std_error(&jerr);	/* Setup libjpeg error handler. FIX: this causes exit on err? */
  jpeg_create_compress(&cinfo);		/* Obtain libjpeg compression object. */
  jpeg_stdio_dest(&cinfo, fp);

  /* Set compression parameters */

  x = iom_GetSamples(h->size, h->org);
  y = iom_GetLines(h->size, h->org);

  cinfo.image_width = x;
  cinfo.image_height = y;
  cinfo.input_components = z;

  if (z == 1)
    cinfo.in_color_space = JCS_GRAYSCALE;
  else
    cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  /* FIX: Set quality? here?
     jpeg_set_quality(&cinfo, quality, TRUE);
  */

  jpeg_start_compress(&cinfo, TRUE);

  /* Write each line. FIX: can we write them all at once? */

  while (cinfo.next_scanline < cinfo.image_height) {
    scanline[0] = data + (cinfo.next_scanline * cinfo.image_width * z);
    (void) jpeg_write_scanlines(&cinfo, scanline, 1);
  }

  /* Cleanup. */

  jpeg_finish_compress(&cinfo);
  fclose(fp);
  jpeg_destroy_compress(&cinfo);

  if (h->org != iom_BIP) {
    free(data);
  }

  return 1;

}
