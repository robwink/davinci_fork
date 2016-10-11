/*
 * io_png.c
 *
 * Jim Stewart
 * 29 Jun 2002
 *
 * Based on io_pnm.c from iomedley, libpng's example.c, and
 * http://www.libpng.org/pub/png/libpng-manual.html examples.
 *
 */

#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_LIBZ

#include "iomedley.h"
#include "png.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* FIX: remove */
#define PNG_DEBUG 3

#define PNG_MAGIC_LEN 8

/* Back-compatibility with old-ass versions of libpng that don't define
   png_jmpbuf. */

#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

int iom_isPNG(FILE* /* Open filehandle */
              );
int iom_GetPNGHeader(FILE*,              /* Open filehandle */
                     char*,              /* Filename */
                     struct iom_iheader* /* Image data & geometry */
                     );
int iom_ReadPNG(FILE*,          /* Open filehandle */
                char*,          /* Filename */
                int*,           /* x dim output */
                int*,           /* y dim output */
                int*,           /* z dim output */
                int*,           /* bits per pixel output */
                unsigned char** /* data output */
                );
int iom_WritePNG(char*,               /* Filename */
                 unsigned char*,      /* Data */
                 struct iom_iheader*, /* Header/geometry */
                 int                  /* Force overwrite */
                 );

/* Returns 1 if fp is a PNG file, 0 otherwise. */
int iom_isPNG(FILE* fp)
{

	unsigned char magic[PNG_MAGIC_LEN];

	rewind(fp);

	fread(magic, 1, PNG_MAGIC_LEN, fp);

	if (png_sig_cmp(magic, 0, PNG_MAGIC_LEN)) {
		return 0;
	}

	return 1;
}

static int* toInts(unsigned short* in, size_t n)
{
	int* out = (int*)realloc(in, n * sizeof(int));

	while (n-- > 0) {
		out[n] = (int)in[n] & 0x0000FFFF;
	}

	return out;
}

int iom_GetPNGHeader(FILE* fp, char* filename, struct iom_iheader* h)
{
	int x, y, z, bits;
	unsigned char* data;

	if (!iom_ReadPNG(fp, filename, &x, &y, &z, &bits, &data)) {
		return 0;
	}

	iom_init_iheader(h);

	h->size[0] = z;
	h->size[1] = x;
	h->size[2] = y;
	h->org     = iom_BIP;

	/* All PNGs are scaled to 8 or 16 bits by iom_ReadPNG(). */

	if (bits == 8) {
		h->eformat = iom_MSB_INT_1;
		h->format  = iom_BYTE;
	} else if (bits == 16) {
		/* h->eformat = iom_MSB_INT_2;
		h->format = iom_SHORT; */
		/* switch to next higher data width - since we don't have 16-bit unsigned shorts */
		data = (unsigned char*)toInts((unsigned short*)data, (size_t)x * (size_t)y * (size_t)z);
		h->eformat = iom_MSB_INT_4;
		h->format  = iom_INT;
	} else {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: can't handle %d-bit PNG data\n", bits);
		}
		return 0;
	}

	h->data = data;

	return 1;
}

/*
* iom_ReadPNG()
*
* Reads an open PNG file and returns image data and geometry.
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
int iom_ReadPNG(FILE* fp, char* filename, int* xout, int* yout, int* zout, int* bits, unsigned char** dout)
{
	png_uint_32 x, y;
	unsigned int z;
	int bit_depth; /* Bits per channel. */
	unsigned int color_type;
	unsigned int i;
	size_t row_stride; /* Bytes per scanline. */
	unsigned char* data;

	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep* row_pointers = NULL;

	/* Open file using default longjmp error handling. */

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: png_create_read_struct()\n");
		}
		return 0;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: info png_create_info_struct()\n");
		}
		/* Cleanup png_ptr; info_ptr and end_info not allocated. */
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 0;
	}

	/* Setup error handling jump point. */

	if (setjmp(png_jmpbuf(png_ptr))) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: libpng encountered error reading %s\n", filename);
		}
		if (row_pointers) {
			free(row_pointers);
		}
		/* Cleanup png_ptr and info_ptr; not using end_info. */
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return 0;
	}

	/* Rewind and start reading. */

	rewind(fp);
	png_init_io(png_ptr, fp);

	/* Read PNG header/geometry info. */

	png_read_info(png_ptr, info_ptr);

#if 1
	png_get_IHDR(png_ptr, info_ptr, &x, &y, &bit_depth, &color_type, NULL, NULL, NULL);
#endif

#if 0
  /* FIX: why aren't these found?  png api docs are outdated.. */
  x = png_get_width(png_ptr, info_ptr);
  y = png_get_height(png_ptr, info_ptr); 
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
#endif

	/* Extract packed pixels of bit depths 1,2,4 into full bytes. */
	png_set_packing(png_ptr);

	/* Expand paletted colors into separate RGB bytes. */
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	/* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel. */
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	/* Expand paletted or RGB images with transparency to full alpha channels
	 * so the data will be available as RGBA quartets.
	 * JAS: I assume we want this.
	 */

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	/* FIX: any other conversions?  libpng is hella complicated. */

	/* ---- */

	/* This might not be necessary with the conversions used, but won't hurt. */
	png_read_update_info(png_ptr, info_ptr);

	z          = png_get_channels(png_ptr, info_ptr);
	row_stride = png_get_rowbytes(png_ptr, info_ptr);
	bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

#ifndef WORDS_BIGENDIAN
	if (bit_depth > 8) {
		png_set_swap(png_ptr);
	}
#endif

#if 0
  /* FIX: remove this block? */
  png_get_IHDR(png_ptr, info_ptr, &x, &y, &bit_depth, &color_type,
	       NULL, NULL, NULL);
  z = png_get_channels(png_ptr, info_ptr);
  row_stride = png_get_rowbytes(png_ptr, info_ptr);
#endif

	/* Allocate memory to hold image, and setup row_pointers array to point
	 * to each row.
	 */

	data = (unsigned char*)malloc(((size_t)y) * row_stride * ((size_t)z));
	if (!data) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: unable to allocate %ld bytes for data in iom_ReadPNG()\n",
			        ((size_t)y) * row_stride * ((size_t)z));
		}
		return 0;
	}

	row_pointers = (png_bytep*)malloc(((size_t)y) * sizeof(png_bytep));
	if (!row_pointers) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr,
			        "ERROR: unable to allocate %ld bytes for row-pointers in iom_ReadPNG()\n",
			        ((size_t)y) * sizeof(png_bytep));
		}
		free(data);
		return 0;
	}

	for (i = 0; i < y; i++) {
		row_pointers[i] = data + (((size_t)i) * row_stride);
	}

	/* Read entire image. */
	png_read_image(png_ptr, row_pointers);

	/* Finish reading the file.  This is theoretically required. */
	png_read_end(png_ptr, info_ptr);

	/* Cleanup. */

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	if (row_pointers) {
		free(row_pointers);
	}

	*xout = x;
	*yout = y;
	*zout = z;
	*bits = bit_depth;
	*dout = data;

	return 1;
}

/*
* Writes the image data to a PNG.
*
* Returns: 1 on success, 0 on failure.
*
* filename   PNG file for output.
* data       Image data matrix.
* h          iomedley header containing image geometry and org info.
* force      Boolean, force file overwrite.
*
*/
int iom_WritePNG(char* filename, unsigned char* indata, struct iom_iheader* h, int force)
{
	FILE* fp;

	unsigned char* data;
	unsigned int x, y, z;
	size_t row_stride;
	unsigned int i;
	unsigned int color_type, bit_depth;
	int free_data = 0;

	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep* row_pointers = NULL;

	// Check file accessibility.
	if (!force && file_exists(filename)) {
		if (iom_is_ok2print_errors()) {
			fprintf(stderr, "File %s already exists.\n", filename);
		}
		return 0;
	}

	/* Yank geometry and organization details out of iomedley header. */

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
			fprintf(stderr, "ERROR: org %d not supported by iom_WriteGIF()", h->org);
		}
		return 0;
	}

/* Check for supported formats and setup some header vars. */

#if 0
  /* FIX: why is eformat zeroed out? */
  if (h->eformat == iom_MSB_INT_1 && h->format == iom_BYTE) {
    bit_depth = 8;
  } else if (h->eformat == iom_MSB_INT_2 && h->format == iom_SHORT) {
    bit_depth = 16;
  } else {
    /* FIX: handle?  ok2p etc */
    /* NOTE: need to change row_stride calculation below if other bit
       depths are supported! */
    fprintf(stderr, "ERROR: iom_WritePNG() input must be 8/16 bit MSB in DV_UINT8/DV_INT16\n");
    fprintf(stderr, "eformat = %d\tformat = %d\n", h->eformat, h->format);
    return 0;
  }
#endif

#if 1
	if (h->format == iom_BYTE) {
		bit_depth = 8;
	} else if (h->format == iom_SHORT) {
		bit_depth = 16;
	} else {
		/* NOTE: need to change row_stride calculation below if other bit depths are supported! */
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: iom_WritePNG() input must be 8/16 bit MSB in DV_UINT8/DV_INT16\n");
			fprintf(stderr, "eformat = %d\tformat = %d\n", h->eformat, h->format);
		}
		return 0;
	}
#endif

	if (z == 1) {
		color_type = PNG_COLOR_TYPE_GRAY;
	} else if (z == 3) {
		color_type = PNG_COLOR_TYPE_RGB;
	} else {
		/* FIX: z == 4 assume RGBA */
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: iom_WritePNG() input must be 1/3 band\n");
		}
		return 0;
	}

	/* Convert data to BIP if not already BIP. */

	if (h->org == iom_BIP || z == 1) {
		data = indata;
	} else {
		/* iom__ConvertToBIP allocates memory, don't forget to free it! */
		if (!iom__ConvertToBIP(indata, h, &data)) {
			return 0;
		}
		free_data = 1;
	}

	/* Open file. */

	fp = fopen(filename, "wb"); /* FIX: handle force */
	if (fp == NULL) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: couldn't open %s for writing\n", filename);
		}
		png_destroy_write_struct(&png_ptr, &info_ptr);
		if (free_data) {
			free(data);
		}
		return 0;
	}

	/* Setup libpng interface structures. */

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: png_create_write_struct()\n");
		}
		fclose(fp);
		if (free_data) {
			free(data);
		}
		return 0;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: info png_create_info_struct()\n");
		}
		fclose(fp);
		/* Cleanup png_ptr; info_ptr and end_info not allocated. */
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		if (free_data) {
			free(data);
		}
		return 0;
	}

	/* Setup error handling jump point. */

	if (setjmp(png_jmpbuf(png_ptr))) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: libpng encountered error writing %s\n", filename);
		}
		/* Cleanup png_ptr and info_ptr; not using end_info. */
		if (free_data) {
			free(data);
		}
		if (row_pointers) {
			free(row_pointers);
		}
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 0;
	}

	/* Start I/O. */

	png_init_io(png_ptr, fp);

	/* Setup header w/geometry and other info. */

	png_set_IHDR(png_ptr, info_ptr, x, y, bit_depth, color_type, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* FIX: Need palette?  Need anything else? */

	png_write_info(png_ptr, info_ptr);

#ifndef WORDS_BIGENDIAN
	if (bit_depth > 8) {
		png_set_swap(png_ptr);
	}
#endif

	/* Setup row pointers and write image. */

	row_pointers = (png_bytep*)malloc(((size_t)y) * sizeof(png_bytep));
	if (!row_pointers) {
		if (free_data) {
			free(data);
		}
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "ERROR: unable to allocate %ld bytes in iom_ReadPNG()\n",
			        ((size_t)y) * sizeof(png_bytep));
		}
		return 0;
	}

	/* Calculate row stride.  NOTE that this needs to change if bit depths
	   other than 8 or 16 are supported! */

	row_stride = ((size_t)x) * ((size_t)z) * (bit_depth / 8);

	for (i = 0; i < y; i++) {
		row_pointers[i] = data + (((size_t)i) * row_stride);
	}

	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);

	/* Cleanup. */

	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);

	if (row_pointers) {
		free(row_pointers);
	}

	if (free_data) {
		free(data);
	}

	return 1;
}

#endif
