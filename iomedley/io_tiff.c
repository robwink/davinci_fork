/*
 * io_tiff.c
 *
 * Jim Stewart
 * 14 Jun 2002
 *
 * Based on io_pnm.c from iomedley, xvtiff.c from xv v3.10a, and examples at
 * http://www.libtiff.org/libtiff.html and
 * http://www.cs.wisc.edu/graphics/Courses/cs-638-1999/libtiff_tutorial.htm
 *
 * This code looks Bad in <120 columns.
 *
 */

#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#include <sys/types.h>
#include <string.h>
#include <tiffio.h>

#include "iomedley.h"

/* Photometric names. */

static const char * const Photometrics[] = {
  "MINISWHITE",
  "MINISBLACK",
  "RGB",
  "PALETTE",
  "MASK",
  "SEPARATED",
  "YCBCR",
  "CIELAB" };

/* Magic numbers as defined in Linux magic number file. */

#define TIFF_MAGIC_BIGEND    "MM\x00\x2a"
#define TIFF_MAGIC_LITTLEEND    "II\x2a\x00"
#define BIG_TIFF_MAGIC_BIGEND    "MM\x00\x2b"
#define BIG_TIFF_MAGIC_LITTLEEND    "II\x2b\x00"

#define MIN(x,y) ((x)<(y)? (x): (y))
#define MAX(x,y) ((x)>(y)? (x): (y))

#define FOUR_GIG 4294967296UL

int    iom_isTIFF(FILE *);
int    iom_GetTIFFHeader(FILE *, char *, struct iom_iheader *);
int    iom_ReadTIFF(FILE *, char *, int *, int *, int *, int *, unsigned char **, int *, int*);
int    iom_WriteTIFF(char *, unsigned char *, struct iom_iheader *, int);

static int uchar_overflow(unsigned char* data, size_t size);
static int int_overflow(unsigned char* data, size_t size);
static int ushort_overflow(unsigned char* data, size_t size);



/* Error handlers used by libtiff.
   They're also used directly by the code below. */

static void
tiff_warning_handler(const char *module, const char *fmt, va_list ap) {

  if (iom_is_ok2print_warnings()) {
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

}

static void
tiff_error_handler(const char *module, const char *fmt, va_list ap) {

  if (iom_is_ok2print_errors()) {
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

}

/* iom_isTIFF(FILE *fp)
 *
 * Magic number check.
 *
 * Returns: 1 if fp is a TIFF file, 0 otherwise.
 *
 */

int
iom_isTIFF(FILE *fp)
{

  unsigned char    magic[4];
  int        i, c;
  
  rewind(fp);

  for (i = 0; i < 4; i++) {
    if ((c = fgetc(fp)) == EOF)
      return 0;
    magic[i] = (unsigned char) c;
  }

  if (!strncmp(magic, TIFF_MAGIC_BIGEND, 4))
    return 1;
  else if (!strncmp(magic, TIFF_MAGIC_LITTLEEND, 4))
    return 1;
  if (!strncmp(magic, BIG_TIFF_MAGIC_BIGEND, 4))
    return 1;
  else if (!strncmp(magic, BIG_TIFF_MAGIC_LITTLEEND, 4))
    return 1;
  else
    return 0;

}

/* iom_GetTIFFHeader() */

int
iom_GetTIFFHeader(FILE *fp, char *filename, struct iom_iheader *h)
{

  int        x, y, z, bits, org, type;
  unsigned char    *data;
  size_t i, dsize;

  if (!iom_ReadTIFF(fp, filename, &x, &y, &z, &bits, &data, &org, &type))
    return 0;

  iom_init_iheader(h);

  h->org = org;

  switch(h->org){
  case iom_BSQ:
      h->size[0] = x;
      h->size[1] = y;
      h->size[2] = z;
     break;
  case iom_BIP:
      h->size[0] = z;
      h->size[1] = x;
      h->size[2] = y;
     break;
  default:
  case iom_BIL:
      h->size[0] = x;
      h->size[1] = z;
      h->size[2] = y;
     break;
  }

  h->data = data;
  dsize = ((size_t)x)*((size_t)y)*((size_t)z);

  int byte_size = bits/8;

  /* type == -1 means sampleformat tag was missing and we assume unsigned int type */
  if (bits == 8) {
    if( type == SAMPLEFORMAT_INT && uchar_overflow(h->data, dsize) < dsize ) {    /* upgrade to shorts for davinci if necessary */
          h->data =(unsigned char*)realloc(h->data, dsize*byte_size*2);
          short* temp_s = (short*)h->data;
          for(i=dsize; i>0; i--)
            temp_s[i-1] = *((char*)(&h->data[i-1]));

          h->format = iom_SHORT;
          h->eformat = iom_NATIVE_INT_2;

    } else {        /* type is SAMPLEFORMAT_UINT or it will fit in an unsigned byte (we assume this if type == -1)*/
        h->format = iom_BYTE;
        h->eformat = iom_NATIVE_INT_1;
    }

  } else if (bits == 16) {
    if( (type == SAMPLEFORMAT_UINT || type == 0) && ushort_overflow(h->data, dsize) < dsize ) { /* upgrade to ints if necessary */
          h->data =(unsigned char*)realloc(h->data, dsize*byte_size*2);
          int* temp_i = (int*)h->data;
          for(i=dsize; i>0; i--)
            temp_i[i-1] = *((unsigned short*)(&h->data[(i-1)*byte_size]));

          h->format = iom_INT;
          h->eformat = iom_NATIVE_INT_4;

    } else {                    /*type == SAMPLEFORMAT_INT or it fits */
      h->format = iom_SHORT;
      h->eformat = iom_NATIVE_INT_2;
    }

  } else if (bits == 32) {
    if( type == SAMPLEFORMAT_UINT ) { 
      if( int_overflow(h->data, dsize) < dsize) {      /* upgrade to doubles if necessary*/
        h->data =(unsigned char*)realloc(h->data, dsize*byte_size*2);
        double* temp_d = (double*)h->data;
        for(i=dsize; i>0; i--)
            temp_d[i-1] = *((unsigned int*)(&h->data[(i-1)*byte_size]));
        
        h->format = iom_DOUBLE;
        h->eformat = iom_MSB_IEEE_REAL_8;

      } else {      /* we can store the data in signed int -no upgrade necessary */
        h->format = iom_INT;
        h->eformat = iom_NATIVE_INT_4;
      }

    } else if( type == SAMPLEFORMAT_INT ) {
      h->format = iom_INT;
      h->eformat = iom_NATIVE_INT_4;
    } else {                    /* assume float: type == SAMPLEFORMAT_IEEEFP or type == 0 */
      h->format = iom_FLOAT;
      h->eformat = iom_MSB_IEEE_REAL_4;
    }
  } else if( bits == 64 ) {
    double* temp_d = (double*)h->data;
    if( type == SAMPLEFORMAT_INT ) {
      for(i=0; i<dsize; i++) {
        temp_d[i] = *((int *)&h->data[i*byte_size]); /*converting from int or unsigned int to double */
      }
    }
    else if( type == SAMPLEFORMAT_UINT ) {
      for(i=0; i<dsize; i++) {
        temp_d[i] = *((unsigned int *)&h->data[i*byte_size]); /*converting from int or unsigned int to double */
      }
    }
    /* 64 bits will always be double or converted to double (type == 0 or SAMPLEFORMAT_IEEEFP skip the conversion) */
    h->format = iom_DOUBLE;
    h->eformat = iom_MSB_IEEE_REAL_8;
  }

  return 1;

}

int
iom_ReadTIFF(FILE *fp, char *filename, int *xout, int *yout, int *zout, 
             int *bits, unsigned char **dout, int *orgout, int *type)
{

  TIFF      *tifffp;
  uint32    x, y, row, tile_width, tile_height, tile_x, tile_y, tile_z, i;
  uint32    actual_tile_width, actual_tile_height;
  tdata_t   buffer;
  size_t    row_stride, src_offset, dest_offset;
  tsize_t   tile_size;                  /* Bytes per tile. */
  tsize_t   tile_row_size, row_bytes;
  ttile_t   tiles_read;
  unsigned char    *data;
  unsigned short z, bits_per_sample, planar_config, plane, photometric, orient;
  int x_tile_idx, y_tile_idx, z_tile_idx, x_tiles, y_tiles, tiff_read_error;

  rewind(fp);

  /* TIFF warning/error handlers.
     Sneakily using them to report our own errors too.
  */

  TIFFSetWarningHandler(tiff_warning_handler);
  TIFFSetErrorHandler(tiff_error_handler);

#if 0
  if ((tifffp = TIFFFdOpen(fileno(fp), filename, "r")) == NULL) {
      TIFFError(NULL, "ERROR: unable to open file %s", filename);
    return 0;
  }
#endif

  if ((tifffp = TIFFOpen(filename, "r")) == NULL) {
    TIFFError(NULL, "ERROR: unable to open file %s", filename);
    return 0;
  }

  /* FIX: check for multiple images per file? */

  TIFFGetFieldDefaulted(tifffp, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);    /* Not always set in file. */

  if (bits_per_sample != 8 && bits_per_sample != 16 && bits_per_sample != 32 && bits_per_sample != 64) {
    TIFFError(NULL, "File %s contains an unsupported (%d) bits-per-sample.", filename, bits_per_sample);
    TIFFClose(tifffp);
    return 0;
  }

  TIFFGetFieldDefaulted(tifffp, TIFFTAG_ORIENTATION, &orient);
  if (orient != ORIENTATION_TOPLEFT) {
    TIFFError(NULL, "TIFF images with orientation other than ORIENTATION_TOPLEFT are not currently supported.");
    TIFFClose(tifffp);
    return 0;
  }

#if 0
  /* Flip orientation so that image comes in X order.
     Taken from xv's xvtiff.c.  Why not both ORIENTATION_TOPLEFT? */

  TIFFGetFieldDefaulted(tifffp, TIFFTAG_ORIENTATION, &orient);
  switch (orient) {
  case ORIENTATION_TOPLEFT:
  case ORIENTATION_TOPRIGHT:
  case ORIENTATION_LEFTTOP:
  case ORIENTATION_RIGHTTOP:   orient = ORIENTATION_BOTLEFT;   break;

  case ORIENTATION_BOTRIGHT:
  case ORIENTATION_BOTLEFT:
  case ORIENTATION_RIGHTBOT:
  case ORIENTATION_LEFTBOT:    orient = ORIENTATION_TOPLEFT;   break;
  }

  TIFFSetField(tifffp, TIFFTAG_ORIENTATION, orient);
#endif

  if (!TIFFGetField(tifffp, TIFFTAG_IMAGEWIDTH, &x)) {
    TIFFError(NULL, "TIFF image width tag missing.");
    TIFFClose(tifffp);
    return 0;
  }

  if (!TIFFGetField(tifffp, TIFFTAG_IMAGELENGTH, &y)) {
    TIFFError(NULL, "TIFF image length tag missing.");
    TIFFClose(tifffp);
    return 0;
  }

  uint16 tiff_type = 0;
  if( !TIFFGetField(tifffp, TIFFTAG_SAMPLEFORMAT, &tiff_type) ) {
    fprintf(stderr, "SAMPLEFORMAT tag missing; assumptions will be made in data format determination\n"); 
  }

  TIFFGetFieldDefaulted(tifffp, TIFFTAG_SAMPLESPERPIXEL, &z);            /* Not always set in file. */

  /* http://libtiff.maptools.org/man/TIFFGetField.3tiff.html */
  /* http://www.awaresystems.be/imaging/tiff/tifftags/planarconfiguration.html */
  TIFFGetFieldDefaulted(tifffp, TIFFTAG_PLANARCONFIG, &planar_config); /* Not always set in file. */

  if (!TIFFGetField(tifffp, TIFFTAG_PHOTOMETRIC, &photometric)) {
    TIFFError(NULL, "TIFF photometric tag missing.");
    TIFFClose(tifffp);
    return 0;
  }

  if (photometric == PHOTOMETRIC_PALETTE) {
        TIFFError(NULL, "TIFF file %s contains an unsupported color data format %s.\n", filename, Photometrics[photometric]);
        TIFFClose(tifffp);
        return 0;

    // TODO: when implementing TIFF Palette support, need to get the color map data
      //if (!TIFFGetField(tifffp, TIFFTAG_COLORMAP, &red, &green, &blue)) {}
  }

  // read in an entire tiled image
  if (TIFFGetField(tifffp, TIFFTAG_TILEWIDTH,  &tile_width) && 
      TIFFGetField(tifffp, TIFFTAG_TILELENGTH, &tile_height)) {

    tile_size = TIFFTileSize(tifffp);
    //tile_count = TIFFNumberOfTiles(tifffp);
    tile_row_size = TIFFTileRowSize(tifffp);

    x_tiles = (int) ceil(x / (double) tile_width); // number of tiles in x-direction
    y_tiles = (int) ceil(y / (double) tile_height); // number of tiles in y-direction


    row_stride = x * bits_per_sample / 8; // Assume BSQ: each band will be read separately
    if (planar_config == PLANARCONFIG_CONTIG) // BIP: read all planes at once
      row_stride *= z;

    buffer = (tdata_t) _TIFFmalloc(tile_size);
    data = (unsigned char *) malloc((size_t) x * (size_t) y * (size_t) z * bits_per_sample / 8);

    if (data == NULL || buffer == NULL) {
      if (iom_is_ok2print_sys_errors())
        fprintf(stderr, "Unable to allocate memory in io_tiff.c/io_ReadTIFF().\n");
      if (buffer) _TIFFfree(buffer);
      if (data) free(data);
      TIFFClose(tifffp);
      return 0;
    }

    tiff_read_error = 0; // start with no-error condition
    for (tile_y = 0; tiff_read_error > -1 && tile_y < y; tile_y += tile_height) {
      y_tile_idx = tile_y / tile_height;
      actual_tile_height = MIN(y, tile_y + tile_height) - tile_y;

      for (tile_x = 0; tiff_read_error > -1 && tile_x < x; tile_x += tile_width) {
        x_tile_idx = tile_x / tile_width;
        actual_tile_width = MIN(x, tile_x + tile_width) - tile_x;

        // BIP: read all bands at once
        // BSQ: read one band at a time
        for (tile_z = 0; tiff_read_error > -1 && tile_z < (planar_config == PLANARCONFIG_CONTIG? 1: z); tile_z++) {

          tiff_read_error = TIFFReadTile(tifffp, buffer, tile_x, tile_y, /* slice */ 0, /* sample */tile_z);
             ///* sample */ planar_config == PLANARCONFIG_CONTIG? z: tile_z);

          if (tiff_read_error > -1){
            for (i = 0; i < actual_tile_height; i++) {
              src_offset = i * tile_row_size;

              dest_offset = (row_stride * tile_y) 
                  + (x_tile_idx * tile_row_size) 
                  + (i * row_stride) 
                  + (tile_z * y * row_stride);

                memcpy(data + dest_offset, buffer + src_offset, actual_tile_width * (tile_row_size / tile_width));
            }
          }
        }
      }
    }

    if (buffer) _TIFFfree(buffer);
    TIFFClose(tifffp);

    if(tiff_read_error < 0) {
      if (data) free(data);
      return 0;
    }

    *xout = x;
    *yout = y;
    *zout = z;
    *bits = bits_per_sample;

    if (photometric != PHOTOMETRIC_PALETTE) {
      *dout = data;
    }
    *type = tiff_type;

    if (planar_config == PLANARCONFIG_CONTIG) {
      *orgout = iom_BIP;
    } else if (planar_config == PLANARCONFIG_SEPARATE) {
      *orgout = iom_BSQ;
    } else {
      TIFFError(
          NULL,
          "TIFF Input file %s is organized in Band-Interleaved-by-Line format. \
            This organization is not currently supported for this tiled file type.",
          filename);
      if (data) free(data);
      return 0;
    }

    return 1;
  }

  /* Untiled read - read entire scanlines */

  /* read in gray scale image */
  row_stride = TIFFScanlineSize(tifffp); /* Bytes per scanline. */

  buffer = (tdata_t) _TIFFmalloc(row_stride);
  data = (unsigned char *) malloc(((size_t)y) * row_stride);

  /* FIX: deal with endian issues? */

  tiff_read_error = 0; // start with no-error condition
  if (planar_config == PLANARCONFIG_CONTIG) {
    for (row = 0; tiff_read_error > -1 && row < y; row++) {
        tiff_read_error = TIFFReadScanline(tifffp, buffer, row, 0); /* 4th arg ignored in this planar config. */
        if (tiff_read_error > -1)
            memcpy(data + ((size_t)row) * row_stride, buffer, row_stride);
    }
  } else {
      for (plane = 0; tiff_read_error > -1 && plane < z; plane++) {
        for (row = 0; tiff_read_error > -1 && row < y; row++) {
            tiff_read_error = TIFFReadScanline(tifffp, buffer, row, plane);
            if (tiff_read_error > -1)
                memcpy(data + ((size_t)row) * row_stride, buffer, row_stride);
        }
      }
  }
  _TIFFfree(buffer);
  TIFFClose(tifffp);

  if(tiff_read_error < 0) {
    if (data)
      free(data);
    return 0;
  }

  if (iom_is_ok2print_progress()) {
    printf("TIFF photometric is %s\n", Photometrics[photometric]);
  }

  *xout = x;
  *yout = y;
  *zout = z;
  *bits = bits_per_sample;
  if (photometric != PHOTOMETRIC_PALETTE) {
    *dout = data;
  }
  *type = tiff_type;

  if (z == 1)
    *orgout = iom_BSQ;
  else
    if (planar_config == PLANARCONFIG_CONTIG)
      *orgout = iom_BIP;
    else
      *orgout = iom_BIL;

  return 1;

}

int
iom_WriteTIFF(char *filename, unsigned char *indata, struct iom_iheader *h, int force)
{

  int        i, x, y, z, out_size[3];
  tstrile_t        row;
  tsize_t    row_stride, rows_per_strip, strips_per_image;
  size_t     bytes_remaining, src_sample_idx, out_sample_idx;
  TIFF        *tifffp = NULL;
  unsigned char *data = NULL;
  unsigned short bits_per_sample, bytes_per_sample;
  tsample_t         fillorder;
  TIFFDataType   sample_fmt = -1;
  tstrip_t strip;
  toff_t  offset, offset2;
  char writeMode[4] = "w";

  /* Check for file existence if force overwrite not set. */

  if (!force && access(filename, F_OK) == 0) {
    if (iom_is_ok2print_errors()) {
      fprintf(stderr, "File %s already exists.\n", filename);
    }
    return 0;
  }

  if (h->format == iom_BYTE) {
    bits_per_sample = 8;
    fillorder = (h->eformat == iom_MSB_INT_1 ? FILLORDER_MSB2LSB : FILLORDER_LSB2MSB);
    sample_fmt = SAMPLEFORMAT_UINT;
  } else if (h->format == iom_SHORT) {
    bits_per_sample = 16;
    fillorder = (h->eformat == iom_MSB_INT_2 ? FILLORDER_MSB2LSB : FILLORDER_LSB2MSB);
    sample_fmt = SAMPLEFORMAT_INT;
  } else if (h->format == iom_INT) {
    bits_per_sample = 32;
    fillorder = (h->eformat == iom_MSB_INT_4 ? FILLORDER_MSB2LSB : FILLORDER_LSB2MSB);
    sample_fmt = SAMPLEFORMAT_INT;
  } else if (h->format == iom_FLOAT) {
    bits_per_sample = 32;
    fillorder = (h->eformat == iom_MSB_IEEE_REAL_4 ? FILLORDER_MSB2LSB : FILLORDER_LSB2MSB);
    sample_fmt = SAMPLEFORMAT_IEEEFP;
  } else if (h->format == iom_DOUBLE) {
    bits_per_sample = 64;
    fillorder = (h->eformat == iom_MSB_IEEE_REAL_8 ? FILLORDER_MSB2LSB : FILLORDER_LSB2MSB);
    sample_fmt = SAMPLEFORMAT_IEEEFP;
  } else {
    if (iom_is_ok2print_errors()) {
      fprintf(stderr, "Cannot write %s data in a TIFF file.\n", iom_FORMAT2STR[h->format]);
    }
    return 0;
  }

  z = iom_GetBands(h->size, h->org);

  TIFFSetWarningHandler(tiff_warning_handler);
  TIFFSetErrorHandler(tiff_error_handler);

  /* write BigTiff if the data size is large */
  if ((iom_iheaderDataSize(h) * iom_iheaderItemBytesI(h)) > FOUR_GIG){
     strcat(writeMode, "8");
  }

  if ((tifffp = TIFFOpen(filename, writeMode)) == NULL) {
    if (iom_is_ok2print_sys_errors()) {
      fprintf(stderr, "Unable to write file %s. Reason: %s.\n", filename, strerror(errno));
    }
    return 0;
  }

  /* Set TIFF image parameters. */

  x = iom_GetSamples(h->size, h->org);
  y = iom_GetLines(h->size, h->org);

  TIFFSetField(tifffp, TIFFTAG_IMAGEWIDTH, x);
  TIFFSetField(tifffp, TIFFTAG_IMAGELENGTH, y);
  TIFFSetField(tifffp, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
  TIFFSetField(tifffp, TIFFTAG_SAMPLESPERPIXEL, z);
  TIFFSetField(tifffp, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tifffp, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tifffp,  TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
  TIFFSetField(tifffp,  TIFFTAG_PREDICTOR, PREDICTOR_NONE);

  if (sample_fmt != -1){
    TIFFSetField(tifffp,  TIFFTAG_SAMPLEFORMAT, sample_fmt);
  }
  if ((z == 3 || z == 4) && h->format == iom_BYTE ) {
    TIFFSetField(tifffp, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  } else {
    TIFFSetField(tifffp, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  }

  if ((z == 2 || z == 4) && h->format == iom_BYTE) {
    // Identify that we have an alpha channel.
    // This is a goofy way to pass an extra value, but everyone seems to do it.
    unsigned short  sample_info[1];
    sample_info[0] = EXTRASAMPLE_ASSOCALPHA;
    TIFFSetField(tifffp, TIFFTAG_EXTRASAMPLES, 1, &sample_info[0]);
  }

  bytes_per_sample = bits_per_sample / 8;

  row_stride = ((size_t)x) * ((size_t)z) * (bytes_per_sample);

  rows_per_strip = MAX(1, (int)ceil((8*1024)/(double)row_stride));
  TIFFSetField(tifffp, TIFFTAG_ROWSPERSTRIP, rows_per_strip);
  strips_per_image = TIFFNumberOfStrips(tifffp);

  bytes_remaining = ((size_t)row_stride) * y;

  /* we only allow BIP output */
  out_size[0] = z;
  out_size[1] = x;
  out_size[2] = y;

  data = (unsigned char *) calloc(row_stride, rows_per_strip);
  if (data==NULL) {
      if (iom_is_ok2print_sys_errors()) {
          fprintf(stderr, "Unable to allocate memory in io_tiff.c/io_WriteTIFF().\n");
      }
      return 0;
  }

  /* keep track of where we are in the output file */
  out_sample_idx = 0;

  // Write the file out one strip at a time.
  for(strip=0; strip < strips_per_image; strip++){
    tsize_t curr_strip_size = MIN(((size_t)row_stride)*rows_per_strip, bytes_remaining);

    for (i=0; i<curr_strip_size; i+=bytes_per_sample) {
      iom_Xpos(out_sample_idx, iom_BIP, out_size, &x, &y, &z); // get x,y,z location of output pixel
      src_sample_idx = iom_Cpos(x, y, z, h->org, h->size); // get linear offset of x,y,z in input org
      memcpy(&data[i], indata+(src_sample_idx*bytes_per_sample), bytes_per_sample);
      out_sample_idx++;
    }
    if (TIFFWriteEncodedStrip(tifffp, strip, data, curr_strip_size) < 0){
      (void) TIFFClose(tifffp);

      free(data);
      return 0;
    }
    bytes_remaining -= curr_strip_size;
    memset(data, 0, ((size_t)row_stride)*rows_per_strip);
  }

  (void) TIFFClose(tifffp);

  free(data);

  return 1;

}

static int
uchar_overflow(unsigned char* data, size_t size)
{
    int i;
    for(i=0; i<size; i++) {
        if(*((char*)&data[i]) < 0)
            break;
    }
    return i;
}

static int
int_overflow(unsigned char* data, size_t size)
{
    int i;
    for(i=0; i<size; i++) {
        if(*((unsigned int*)&data[i*4]) > INT_MAX)
            break;
    }
    return i;
}

static int
ushort_overflow(unsigned char* data, size_t size)
{
    size_t i;
    for(i=0; i<size; i++) {
        if(*((short*)&data[i*2]) < 0)
            break;
    }
    return i;
}
