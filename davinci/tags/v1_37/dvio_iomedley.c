/*
 * dvio_iomedley.c
 *
 * Jim Stewart
 * 26 Jun 2002
 *
 * Common interface to all iomedley image format read/write routines.
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "dvio.h"

/* FIX: put these in a header */

extern int iom_isBMP(FILE *);
extern int iom_isGIF(FILE *);
extern int iom_isJPEG(FILE *);
extern int iom_isTIFF(FILE *);
extern int iom_isPNG(FILE *);

#if 0
extern int iom_isPNM(FILE *);
#endif

extern int iom_GetBMPHeader(FILE *, char *, struct iom_iheader *);
extern int iom_GetGIFHeader(FILE *, char *, struct iom_iheader *);
extern int iom_GetJPEGHeader(FILE *, char *, struct iom_iheader *);
extern int iom_GetTIFFHeader(FILE *, char *, struct iom_iheader *);

#if 0
extern int iom_GetPNMHeader(FILE *, char *, struct iom_iheader *);
#endif

extern int iom_WriteBMP();
extern int iom_WriteGIF();
extern int iom_WriteJPEG();
extern int iom_WriteTIFF();

#if 0
extern int iom_WritePNM();
extern int iom_WritePPM();
extern int iom_WritePGM();
#endif

/* Create lookup table for external iomedley image functions. */

typedef	int (*_iom_is_func)(FILE *);
typedef	int (*_iom_header_func)(FILE *, char *, struct iom_iheader *);
typedef int (*_iom_write_func)(char *, unsigned char *, struct iom_iheader *, int);

typedef struct {
  const char		*type;		/* Canonical name. */
  const char		**extensions;	/* Davinci types/file extensions. */
  _iom_is_func		is;		/* Function to test magic number. */
  _iom_header_func	header;		/* Funtion to read header & data. */
  _iom_write_func	write;		/* Function to write new file. */
  unsigned short int    maxbytes;       /* Max bytes per pixel. */
} iom_io_interface;

/* Recognized types/file extensions, used during file creation. */

static const char *gif_extensions[]  = { "gif", NULL };
static const char *jpeg_extensions[] = { "jpg", "jpeg", NULL };
static const char *bmp_extensions[]  = { "bmp", NULL };
static const char *tiff_extensions[] = { "tif", "tiff", NULL };

#if 0
static const char *pnm_extensions[]  = { "pnm", NULL };
static const char *ppm_extensions[]  = { "ppm", NULL };
static const char *pgm_extensions[]  = { "pgm", NULL };
#endif

#ifdef HAVE_LIBPNG
extern int iom_GetPNGHeader(FILE *, char *, struct iom_iheader *);
extern int iom_WritePNG();
static const char *png_extensions[]  = { "png", NULL };
#endif

static iom_io_interface	interfaces[] = {
  { "GIF",  gif_extensions,  iom_isGIF,  iom_GetGIFHeader,  iom_WriteGIF,  1 },
  { "JPEG", jpeg_extensions, iom_isJPEG, iom_GetJPEGHeader, iom_WriteJPEG, 1 },
  { "TIFF", tiff_extensions, iom_isTIFF, iom_GetTIFFHeader, iom_WriteTIFF, 2 },
  { "BMP",  bmp_extensions,  iom_isBMP,  iom_GetBMPHeader,  iom_WriteBMP,  1 },
#ifdef HAVE_LIBPNG
  { "PNG",  png_extensions,  iom_isPNG,  iom_GetPNGHeader,  iom_WritePNG,  2 },
#endif
#if 0
  { "PNM",  pnm_extensions,  iom_isPNM,  iom_GetPNMHeader,  iom_WritePNM,  1 }, /* FIX: check maxbytes */
  { "PPM",  pnm_extensions,  iom_isPNM,  iom_GetPNMHeader,  iom_WritePPM,  1 },
  { "PGM",  pnm_extensions,  iom_isPNM,  iom_GetPNMHeader,  iom_WritePGM,  1 },
#endif
  { NULL,   NULL,            NULL,       NULL,              NULL,          0 },
};

/*********/

Var *
dv_LoadIOM(FILE *fp, char *filename, struct iom_iheader *s)
{

  struct iom_iheader	h;
  unsigned char		*data;
  Var			*v;
  char			hbuf[HBUFSIZE];
  unsigned short	interface;

  /* Check iom_isFOO for each interface. */
  /* FIX: preread file header and pass that to isFOO to eliminate
     consecutive rewind()/read(). */

  interface = 0;

  while (interfaces[interface].is != NULL) {
    if ((*interfaces[interface].is)(fp)) {
      break;
    }
    interface++;
  }

  if (interfaces[interface].is == NULL) {
    return NULL;
  }

  /* Read file and populate header, including image data. */

  if (!(*interfaces[interface].header)(fp, filename, &h)) {
    return NULL;
  }

  if (s != NULL) {
    iom_MergeHeaderAndSlice(&h, s);
  }

  data = iom_read_qube_data(-1, &h);	/* Sending fd -1 because we always set h->data. */
  if (iom_is_ok2print_progress()) {
    /* sorta a hack because iom_read_qube_data output is ugly */
    fprintf(stderr, "\n");
  }

  if (data) {
    v = iom_iheader2var(&h);
    V_DATA(v) = data;
  } else {
    v = NULL;
  }

  if (VERBOSE > 1) {
    sprintf(hbuf, "%s: %s %s image: %dx%dx%d, %d bits",
	    filename, iom_Org2Str(h.org),
	    interfaces[interface].type,
	    iom_GetSamples(h.dim, h.org), 
	    iom_GetLines(h.dim, h.org), 
	    iom_GetBands(h.dim, h.org), 
	    iom_NBYTESI(h.format) * 8);
    parse_error(hbuf);
  }

  iom_cleanup_iheader(&h);

  return v;

}

int
dv_WriteIOM(Var *obj, char *filename, char *type, int force)
{

  struct		iom_iheader h;
  int			status;
  unsigned short	interface, ext, extmatch;

  /* Check recognized file types/extensions for each interface. */

  interface = extmatch = 0;
  while (!extmatch && (interfaces[interface].extensions != NULL)) {
    ext = 0;
    while (interfaces[interface].extensions[ext] != NULL) {
      if (!strcasecmp(interfaces[interface].extensions[ext], type)) {
	extmatch = 1;
	break;
      }
      ext++;
    }
    if (!extmatch) {
      interface++;
    }
  }

  if (interfaces[interface].write == NULL) {
    return 0;
  }

  if (V_TYPE(obj) != ID_VAL || NBYTES(V_FORMAT(obj)) > interfaces[interface].maxbytes) {
    sprintf(error_buf, "Data for %s file must be %d bit.", interfaces[interface].type, interfaces[interface].maxbytes * 8);
    parse_error(NULL);
    return 0;
  }

  /* FIX: check depth (?) and org (?) */

  var2iom_iheader(obj, &h);	

  if (VERBOSE > 1)  {
    fprintf(stderr, "Writing %s: %dx%dx%d %s file.\n",
	    filename, 
	    iom_GetSamples(h.size,h.org),
	    iom_GetLines(h.size,h.org),
	    iom_GetBands(h.size,h.org),
	    interfaces[interface].type);
  }

  status = (*interfaces[interface].write)(filename, V_DATA(obj), &h, force);
  iom_cleanup_iheader(&h);

  if (status == 0){
    parse_error("Writing of %s file %s failed.\n",
		interfaces[interface].type, filename);
    return 0;
  }
    
  return 1;

}
