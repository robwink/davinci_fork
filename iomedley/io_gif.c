/*
 * io_gif.c
 *
 * Jim Stewart
 * 11 Jun 2002
 *
 * Based on io_pnm.c from iomedley, giflib/libungif 4.1.0 docs,
 * llib v1.1.7 source, and xxgif.c from xv-3.10a.
 *
 */

#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include "iomedley.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <gif_lib.h>
#include <string.h>
#include <sys/types.h>

/* GIF magic number taken from Linux magic file. */

#define GIF_MAGIC "GIF8"
#define GIF_MAGIC_LEN 4

/* Prototypes. */

int iom_isGIF(FILE* /* Open filehandle */
              );
int iom_GetGIFHeader(FILE*,              /* Open filehandle */
                     char*,              /* Filename */
                     struct iom_iheader* /* Image data & geometry */
                     );
int iom_ReadGIF(FILE*,          /* Open filehandle */
                char*,          /* Filename */
                int*,           /* x dim output */
                int*,           /* y dim output */
                int*,           /* z dim output */
                int*,           /* bits per pixel output */
                unsigned char** /* data output */
                );
int iom__ExtractColor(unsigned char*,                             /* Image data */
                      unsigned int,                               /* x, y, z dimensions */
                      unsigned int, unsigned int, unsigned short, /* z to extract */
                      unsigned char**                             /* zplane data output */
                      );
int iom_WriteGIF(char*,               /* Filename */
                 unsigned char*,      /* Data */
                 struct iom_iheader*, /* Header/geometry */
                 int                  /* Force overwrite */
                 );

/* Global vars. */

/* This is used when there's no colormap. */

static int EGApalette[16][3] = {{0, 0, 0},       {0, 0, 128},     {0, 128, 0},     {0, 128, 128},
                                {128, 0, 0},     {128, 0, 128},   {128, 128, 0},   {200, 200, 200},
                                {100, 100, 100}, {100, 100, 255}, {100, 255, 100}, {100, 255, 255},
                                {255, 100, 100}, {255, 100, 255}, {255, 255, 100}, {255, 255, 255}};

/* These are used while reading interlaced GIFs. */

static int InterlacedOffset[] = {0, 4, 2, 1};
static int InterlacedJumps[]  = {8, 8, 4, 2};

/* Functions. */

void PrintGifError(int ErrorCode)
{
	const char* Err = GifErrorString(ErrorCode);

	if (Err != NULL)
		fprintf(stderr, "GIF-LIB error: %s.\n", Err);
	else
		fprintf(stderr, "GIF-LIB undefined error %d.\n", ErrorCode);
}

int iom_isGIF(FILE* fp)
{

	/* Returns 1 if fp is a GIF file, 0 otherwise. */

	char magic[GIF_MAGIC_LEN];
	int i, c;

	rewind(fp);

	for (i = 0; i < GIF_MAGIC_LEN; i++) {
		if ((c   = fgetc(fp)) == EOF) return 0;
		magic[i] = (unsigned char)c;
	}

	if (!memcmp(magic, GIF_MAGIC, GIF_MAGIC_LEN))
		return 1;
	else
		return 0;
}

int iom_GetGIFHeader(FILE* fp, char* filename, struct iom_iheader* h)
{

	int x, y, z, bits;
	unsigned char* data;

	if (!iom_ReadGIF(fp, filename, &x, &y, &z, &bits, &data)) {
		return 0;
	}

	iom_init_iheader(h);

	h->size[0] = z;
	h->size[1] = x;
	h->size[2] = y;
	h->org     = iom_BIP;

	/* Bits in sample always 8 as defined by GIF spec. */

	h->eformat = iom_MSB_INT_1;
	h->format  = iom_BYTE;

	h->data = data;

	return 1;
}

int iom_ReadGIF(FILE* fp, char* filename, int* xout, int* yout, int* zout, int* bits, unsigned char** dout)
{

	/*
	 * iom_ReadGIF()
	 *
	 * Reads an open GIF file and returns image data and geometry.
	 *
	 * Returns: 1 on success, 0 on failure.
	 *
	 * Args:
	 *
	 * fp         open file
	 * xout       image width passback
	 * yout       image height passback
	 * zout       number/bands passback (always 3; API consistency)
	 * bits       bits per sample passback (always 8;  API consistency)
	 * dout       image data buffer passback (allocated by this function)
	 *
	 */

	unsigned int x, y, z;
	unsigned int i, j;
	unsigned char* data = NULL;
	unsigned char *r, *g, *b;
	unsigned int colormap_idx;
	short color_warning = 0; /* Non-fatal cmap error */
	GifFileType* gft;
	ColorMapObject* colormap;
	GifRecordType grt;
	GifPixelType* gifdata;
	int err;

/* Open file. */

#if 0  
  /* For some reason this doesn't work, so I'm reopening the file. */
  rewind(fp);

  if ((gft = DGifOpenFileHandle(fileno(fp))) == NULL) {
    if (iom_is_ok2print_sys_errors())
      PrintGifError();
    return 0;
  }

#endif

	if ((gft = DGifOpenFileName(filename, &err)) == NULL) {
		if (iom_is_ok2print_sys_errors()) PrintGifError(err);
		return 0;
	}

#if 0
  /* This may do much of what the while () loop below does, if implemented. */
  if (DGifSlurp(gft) != GIF_OK) {
    if (iom_is_ok2print_sys_errors())
      PrintGifError();
    DGifCloseFile(gft);
    return 0;
  }
#endif

	/*
	 * Read the GIF records (can be in arbitrary order) til we find an image.
	 * Ignoring pretty much everything else.
	 * Converting to 24-bit RGB image.
	 * This code was lifted nearly unchanged from llib-1.1.7 source.
	 *
	 */

	data = NULL;

	while (data == NULL) {

		grt = UNDEFINED_RECORD_TYPE;

		if (DGifGetRecordType(gft, &grt) == GIF_ERROR) {
			if (iom_is_ok2print_sys_errors()) PrintGifError(gft->Error);
			DGifCloseFile(gft, NULL);
			return 0;
		}

		/* The only record we care about is IMAGE_DESC_RECORD_TYPE. */

		if (grt == IMAGE_DESC_RECORD_TYPE) {

			if (DGifGetImageDesc(gft) == GIF_ERROR) {
				if (iom_is_ok2print_sys_errors()) PrintGifError(gft->Error);
				DGifCloseFile(gft, NULL);
				return 0;
			}

			x = gft->Image.Width;
			y = gft->Image.Height;

			gifdata = (GifPixelType*)malloc(x * y);

			if (gft->Image.Interlace) {
				for (i = 0; i < 4; i++) {
					for (j = InterlacedOffset[i]; j < y; j += InterlacedJumps[i]) {
						if (DGifGetLine(gft, gifdata + (j * x), x) == GIF_ERROR) {
							if (iom_is_ok2print_sys_errors()) PrintGifError(gft->Error);
							DGifCloseFile(gft, NULL);
							return 0;
						}
					}
				}
			} else {
				if (DGifGetLine(gft, gifdata, x * y) == GIF_ERROR) {
					if (iom_is_ok2print_sys_errors()) PrintGifError(gft->Error);
					DGifCloseFile(gft, NULL);
					return 0;
				}
			}

			/* Convert image to 24-bit RGB unless the individual RGB values are equal
		   (means we probably wrote it from a 1-band source).  If there's no
		   colormap, convert to 24-bit RGB using the default EGA palette snagged
		   from XV. */

			/* Load the EGA palette if there's no colormap. */

			if (gft->SColorMap) {
				colormap = gft->SColorMap;
			} else {
				/* No colormap, using EGA palette repeated 16 times. */
				if (iom_is_ok2print_sys_errors()) {
					fprintf(
					    stderr,
					    "Warning: GIF file %s contained no colormap; using default EGA palette.\n",
					    filename);
				}

				colormap = (ColorMapObject*)malloc(sizeof(ColorMapObject));
				if (!colormap) {
					if (iom_is_ok2print_sys_errors()) {
						fprintf(stderr, "Unable to allocate %ld bytes for image data.\n",
						        sizeof(ColorMapObject));
					}
					if (gifdata) free(gifdata);
					DGifCloseFile(gft, NULL);
					return 0;
				}

				colormap->Colors = (GifColorType*)malloc(sizeof(GifColorType) * 256);
				if (!colormap->Colors) {
					if (iom_is_ok2print_sys_errors()) {
						fprintf(stderr, "Unable to allocate %ld bytes for image data.\n",
						        sizeof(GifColorType) * 256);
					}
					free(colormap);
					free(gifdata);
					DGifCloseFile(gft, NULL);
					return 0;
				}

				colormap->ColorCount   = 256;
				colormap->BitsPerPixel = 8;
				for (i = 0; i < 256; i++) {
					colormap->Colors[i].Red   = EGApalette[i & 15][0];
					colormap->Colors[i].Green = EGApalette[i & 15][1];
					colormap->Colors[i].Blue  = EGApalette[i & 15][2];
				}
			}

			for (i = 0; i < colormap->ColorCount; i++) {
				if (!((colormap->Colors[i].Red == colormap->Colors[i].Green) &&
				      (colormap->Colors[i].Red == colormap->Colors[i].Blue))) {
					break;
				}
			}

			if (i == colormap->ColorCount) {
				/* They were all equal. */
				z = 1;
			} else {
				z = 3;
			}

			if (z == 3) {
				/* 3-band, allocate memory and grab RGB values from colormap. */
				data = (unsigned char*)malloc(x * y * 3);
				if (!data) {
					if (iom_is_ok2print_sys_errors()) {
						fprintf(stderr, "Unable to allocate %d bytes for image data.\n", x * y * 3);
					}
					if (!gft->SColorMap) {
						free(colormap->Colors);
						free(colormap);
					}
					free(gifdata);
					DGifCloseFile(gft, NULL);
					return 0;
				}
				for (i = 0; i < y; i++) {
					for (j = 0; j < x; j++) {
						colormap_idx = *(gifdata + (i * x) + j);
						/* Calculate the new pixel location in the 3-band copy,
						   using BIP organization. */
						r = data + (i * x * 3) + (j * 3);
						g = r + 1;
						b = r + 2;
						/* Make sure this color value exists in the colormap. */
						if (colormap_idx > gft->SColorMap->ColorCount) {
							/* Don't want to die here since everything else is ok, but carp if it's
							 * the first. */
							if (!color_warning && iom_is_ok2print_sys_errors()) {
								color_warning = 1;
								fprintf(stderr,
								        "Warning: GIF file %s contains some invalid color data.\n",
								        filename);
							}
							/* Using the last entry in the colormap for all invalid colors. */
							colormap_idx = gft->SColorMap->ColorCount - 1;
						}
						*r = gft->SColorMap->Colors[colormap_idx].Red;
						*g = gft->SColorMap->Colors[colormap_idx].Green;
						*b = gft->SColorMap->Colors[colormap_idx].Blue;
					}
				}
			} else {
				/* 1-band, just use the raw data. */
				data = gifdata;
			}
		}
		/* Ignore any of the other record types. */
	}

	/* Cleanup. */
	/* FIX: We close file, caller currently 'expects' open filehandle when done. */

	if (!gft->SColorMap && colormap) {
		if (colormap->Colors) free(colormap->Colors);
		free(colormap);
	}

	if (gifdata && z == 3) free(gifdata);

	DGifCloseFile(gft, NULL);

	/* Set everything we need to pass back. */

	*xout = x;
	*yout = y;
	*zout = z;
	*bits = 8; /* GIFs are always 8-bit. */
	*dout = data;

	return 1;
}

int iom__ExtractColor(unsigned char* data, unsigned int x, unsigned int y, unsigned int z,
                      unsigned short z_to_extract, unsigned char** dout)
{

	/*
	 * Extracts a single z plane as a 2D array.  data MUST be in BIP format.
	 *
	 * Returns: 1 on success, 0 on error.
	 *
	 * Args:
	 *
	 * h                  iom_iheader containing geometry and data.
	 * z_to_extract       Color/z plane to extract (ie 0,1,2 for RGB data).
	 * dout               Set to point to 2 dimensional array of bytes
	 *                    representing the color data requested.
	 */

	int i, j;
	unsigned char pixel;
	unsigned char* buffer;

	buffer = (unsigned char*)malloc(x * y);
	if (buffer == NULL) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: Unable to allocate %d bytes in iom__ExtractColor()!\n", x * y);
		}
		return 0;
	}
	for (i = 0; i < y; i++) {
		for (j = 0; j < x; j++) {
			pixel                   = (unsigned char)*(data + (i * x * z) + (j * z) + z_to_extract);
			*(buffer + (i * x) + j) = pixel;
		}
	}

	*dout = buffer;

	return 1;
}

int iom_WriteGIF(char* filename, unsigned char* indata, struct iom_iheader* h, int force)
{

	/*
	 * Writes the image data to a GIF,
	 * quantizing down to 256 colors if necessary.
	 *
	 * Returns: 1 on success, 0 on failure.
	 *
	 * filename   GIF file for output.
	 * data       Image data matrix.
	 * h          iomedley header containing image geometry and org info.
	 * force      Boolean, force file overwrite.
	 *
	 */

	/* FIX: centralize all the memory cleanups somewhere, maybe ala PNG's destruct functions */

	unsigned char* data;
	GifByteType* rgb[3] = {NULL, NULL, NULL}; /* Input data for quantizer. */
	GifByteType* qdata;                       /* Quantized pixel data. */
	GifColorType qcolormap[256];              /* Quantized colormap. */
	ColorMapObject* GIFcolormap;              /* Colormap to write to file. */
	int num_colors = 256;                     /* Max size of colormap. */
	unsigned int x, y, z;
	unsigned int i;
	int err;
	GifFileType* gft;

	/* Check file accessibility. */

	if (!force && file_exists(filename)) {
		if (iom_is_ok2print_errors()) {
			fprintf(stderr, "File %s already exists.\n", filename);
		}
		return 0;
	}

	/* Extract image geometry based on organization. */

	if (h->org == iom_BIP) {
		z = h->size[0];
		x = h->size[1];
		y = h->size[2];
	} else if (h->org == iom_BSQ) {
		x = h->size[0];
		y = h->size[1];
		z = h->size[2];
	} else if (h->org == iom_BIL) {
		x = h->size[0];
		z = h->size[1];
		y = h->size[2];
	} else {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: org %d not supported by iom_WriteGIF()\n", h->org);
		}
		return 0;
	}

	/* Only handles 1- or 3-band images. */

	if (z != 1 && z != 3) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr,
			        "ERROR: %d-band data not supported by iom_WriteGIF() (must be 1 or 3)\n", z);
		}
		return 0;
	}

	/* Convert 3-band data to BIP if not already BIP. */

	if (h->org == iom_BIP || z == 1) {
		data = indata;
	} else {
		/* iom__ConvertToBIP allocates memory, don't forget to free it! */
		if (!iom__ConvertToBIP(indata, h, &data)) {
			return 0;
		}
	}

	/* Quantize colors if 3-band, or create colormap if 1-band. */

	if (z == 3) {

		/* Extract each band into a separate plane. */

		for (i = 0; i < z; i++) {
			if (!iom__ExtractColor(data, x, y, z, i, &rgb[i])) {
				if (h->org != iom_BIP) {
					free(data);
				}
				return 0;
			}
		}

		/* Allocate buffer for quantized data. */

		qdata = (GifByteType*)malloc(x * y);
		if (qdata == NULL) {
			if (iom_is_ok2print_unsupp_errors()) {
				fprintf(stderr, "ERROR: Unable to allocate %d bytes in iom_WriteGIF()\n", x * y);
			}
			if (h->org != iom_BIP) {
				free(data);
			}
			if (z == 3) {
				for (i = 0; i < z; i++) {
					if (rgb[i]) {
						free(rgb[i]);
					}
				}
			}
			return 0;
		}

		/* Quantize using giflib's routine. */

		if (GifQuantizeBuffer(x, y, &num_colors, rgb[0], rgb[1], rgb[2], qdata, qcolormap) == GIF_ERROR) {
			if (iom_is_ok2print_unsupp_errors()) {
				fprintf(stderr, "ERROR: Unable to quantize colors in iom_WriteGIF()\n");
			}
			if (h->org != iom_BIP) {
				free(data);
			}
			if (qdata) {
				free(qdata);
			}
			for (i = 0; i < z; i++) {
				if (rgb[i]) {
					free(rgb[i]);
				}
			}
			return 0;
		}

	} else {

		/* For 1-band images, just fill the colormap with 0-255 for lack of
		   anything better to do.  Setting RGB equal to each other for
		   conversion back to 1-band on reads. */

		qdata = data;

		/* FIX: maybe do 0, 255, 1-254 so black/white are first entries. */
		for (i = 0; i < 256; i++) {
			qcolormap[i].Red = qcolormap[i].Blue = qcolormap[i].Green = i;
		}
	}

	/* FIX: maybe just round num_colors up to the next power of 2 and use that instead of 256? */
	GIFcolormap = GifMakeMapObject(256, qcolormap);

	if ((gft = EGifOpenFileName(filename, (force ? 0 : 1), &err)) == NULL) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "ERROR: Unable to open file %s for write.\n", filename);
			PrintGifError(err);
		}
		if (h->org != iom_BIP && z != 1) {
			free(data);
		}
		if (z == 3 && qdata) {
			free(qdata);
		}
		for (i = 0; i < z; i++) {
			if (rgb[i]) {
				free(rgb[i]);
			}
		}
		GifFreeMapObject(GIFcolormap);
		return 0;
	}

#if 0
  /* This works fine in libungif 4.x, but it causes giflib 3.0 to segfault. */
  /* Might want to use 87a anyway, if this is every turned on. */
  EGifSetGifVersion("89a");
#endif

	//NOTE(rswinkle) this is a legacy giflib function, shouldn't be calling it directly
	if (EGifPutScreenDesc(gft, x, y, 8, /* bits per pixel */
	                      0,            /* background color index */
	                      GIFcolormap) == GIF_ERROR) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "ERROR: GIFLIB error in iom_WriteGIF()\n");
			PrintGifError(gft->Error);
		}
		if (h->org != iom_BIP && z != 1) {
			free(data);
		}
		if (z == 3 && qdata) {
			free(qdata);
		}
		for (i = 0; i < z; i++) {
			if (rgb[i]) {
				free(rgb[i]);
			}
		}
		GifFreeMapObject(GIFcolormap);
		return 0;
	}

	if (EGifPutImageDesc(gft, 0 /* left */, 0 /* top */, x, y, 0, /* interlaced */
	                     NULL /* colormap */) == GIF_ERROR) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "ERROR: GIFLIB error in iom_WriteGIF()\n");
			PrintGifError(gft->Error);
		}
		if (h->org != iom_BIP && z != 1) {
			free(data);
		}
		if (z == 3 && qdata) {
			free(qdata);
		}
		for (i = 0; i < z; i++) {
			if (rgb[i]) {
				free(rgb[i]);
			}
		}
		GifFreeMapObject(GIFcolormap);
		return 0;
	}

	/* Write everything out at once. */
	if (EGifPutLine(gft, qdata, x * y) == GIF_ERROR) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "ERROR: GIFLIB error in iom_WriteGIF()\n");
			PrintGifError(gft->Error);
		}
		if (h->org != iom_BIP && z != 1) {
			free(data);
		}
		if (z == 3 && qdata) {
			free(qdata);
		}
		for (i = 0; i < z; i++) {
			if (rgb[i]) {
				free(rgb[i]);
			}
		}
		return 0;
	}

	/* Close file and clean up. */

	EGifCloseFile(gft, NULL);

	if (h->org != iom_BIP && z != 1) {
		free(data);
	}

	if (z == 3 && qdata) {
		free(qdata);
	}

	/* Free colormaps. */

	for (i = 0; i < z; i++) {
		if (rgb[i]) {
			free(rgb[i]);
		}
	}
	GifFreeMapObject(GIFcolormap);

	return 1;
}
