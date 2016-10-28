#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include "iomedley.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static int get_int(FILE* fp);
static int getbit(FILE* fp);
int iom_WritePNM(char* fname, unsigned char* data, struct iom_iheader* h, int force_write);
int iom_ReadPNM(FILE* fp, char* filename, int* xout, int* yout, int* zout, int* bits,
                unsigned char** dout, int* data_offset);

int iom_isPNM(FILE* fp)
{
	char id, format;

	rewind(fp);

	id     = fgetc(fp);
	format = fgetc(fp);

	if (id == 'P' && strchr("123456", format)) {
		return 1;
	}

	return 0;
}

int iom_GetPNMHeader(FILE* fp, char* fname, struct iom_iheader* h)
{
	int x, y, z;
	unsigned char* data = NULL;
	int bits;
	int offset;

	if (!iom_isPNM(fp)) {
		return 0;
	}

	/* offset = ftell(fp); */

	/**
	 *** Get format
	 **/
	if (!iom_ReadPNM(fp, fname, &x, &y, &z, &bits, &data, &offset)) {
		return 0;
	}

	iom_init_iheader(h);

	/* h->dptr = offset; */

	if (z == 1) {
		h->size[0] = x;
		h->size[1] = y;
		h->size[2] = z;
		h->org     = iom_BSQ;
	} else {
		h->size[0] = z;
		h->size[1] = x;
		h->size[2] = y;
		h->org     = iom_BIP;
	}
	/* h->org = (z == 1 ? iom_BSQ : iom_BIP); */ /* data organization */
	                                             /*
	                                                  iom_GetSamples(h->size, h->org) = x;
	                                                  iom_GetBands(h->size, h->org) = z;
	                                                  iom_GetLines(h->size, h->org) = y;
	                                                  */

	if (bits == 16) {
		h->eformat = iom_MSB_INT_2;
		h->format  = iom_SHORT;
	} else {
		h->eformat = iom_MSB_INT_1;
		h->format  = iom_BYTE;
	}

	/*
	     h->offset = 0;
	     h->gain = 0;
	     h->prefix[0] = h->prefix[1] = h->prefix[2] = 0;
	     h->suffix[0] = h->suffix[1] = h->suffix[2] = 0;
	     */

	h->data = data;

	return (1);
}

int iom_WritePNM(char* fname, unsigned char* data, struct iom_iheader* h, int force_write)
{
	size_t x, y, z;
	FILE* fp = NULL;

	if (h->format != iom_BYTE) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "Cannot write %s data in a PNM file.\n", iom_ifmt_to_str(h->format));
		}
		return 0;
	}

	x = iom_GetSamples(h->size, h->org);
	y = iom_GetLines(h->size, h->org);
	z = iom_GetBands(h->size, h->org);

	if (z != 1 && z != 3) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "Cannot write PNM files with depths other than 1 or 3.\n");
			fprintf(stderr, "See file %s  line %d.\n", __FILE__, __LINE__);
		}
		return 0;
	}

	if (z == 3 && h->org != iom_BIP) {
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "For depth 3 data must be in BIP organization.");
		}
		return 0;
	}

	if (!force_write && file_exists(fname)) {
		if (iom_is_ok2print_errors()) {
			fprintf(stderr, "File %s already exists.\n", fname);
		}
		return 0;
	}

	if ((fp = fopen(fname, "wb")) == NULL) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "Unable to write file %s. Reason: %s.\n", fname, strerror(errno));
		}
		return 0;
	}

	fprintf(fp, "P%c\n%ld %ld\n255\n", (z == 1 ? '5' : '6'), x, y);

	/*
	 ** Ordinarily we would be byte-swapping data here but there is no
	 ** need to do so, since we will only be writing byte data.
	 */
	if (fwrite(data, z, x * y, fp) != (x * y) || ferror(fp)) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "Error writing to file %s. Reason: %s.\n", fname, strerror(errno));
		}
		fclose(fp);
		unlink(fname);
		return 0;
	}

	fclose(fp);

	return 1;
}

#define CHECK_READ_1(read_call, item_bytes, items, file_type, file_name)                            \
	if ((read_call) != (items)) {                                                                   \
		if (iom_is_ok2print_unsupp_errors()) {                                                      \
			fprintf(stderr, "Error reading %ld bytes from %s file %s.\n", ((item_bytes) * (items)), \
			        (file_type), ((file_name) == NULL ? "(null)" : (file_name)));                   \
		}                                                                                           \
		return 0;                                                                                   \
	}

#define CHECK_MALLOC_1(malloc_call, bytes)                             \
	if ((malloc_call) == NULL) {                                       \
		if (iom_is_ok2print_unsupp_errors()) {                         \
			fprintf(stderr, "Error allocating %ld bytes.\n", (bytes)); \
		}                                                              \
		return 0;                                                      \
	}

#define MAX_VAL_FAILURE(maxval, file_type, filename)                                \
	if (iom_is_ok2print_unsupp_errors()) {                                          \
		fprintf(stderr, "Unable to read %s file %s. Odd maxval %d.\n", (file_type), \
		        ((filename) == NULL ? "(null)" : (filename)), (maxval));            \
	}                                                                               \
	return (0);

int iom_ReadPNM(FILE* fp, char* filename, int* xout, int* yout, int* zout, int* bits,
                unsigned char** dout, int* data_offset)
{
	char id, format;
	size_t x, y, z, count;
	size_t i, j, k;
	int d;
	int bitshift;
	int maxval;
	unsigned char* data;

	/**
	 *** Get format
	 **/
	rewind(fp);

	id     = fgetc(fp);
	format = fgetc(fp);

	if (id != 'P') {
		return (0);
	}
	if (format < '1' || format > '6') {
		return (0);
	}
	x = get_int(fp);
	y = get_int(fp);
	z = 1;

	*data_offset = ftell(fp);

	count = 0;
	switch (format) {
	case '1': /* plain pbm format */
		CHECK_MALLOC_1(data = (unsigned char*)calloc(1, x * y), 1 * x * y);
		for (i = 0; i < y; i++) {
			for (j = 0; j < x; j++) {
				data[count++] = getbit(fp);
			}
		}
		*bits = 1;
		break;
	case '2': /* plain pgm format */
		maxval       = get_int(fp);
		*data_offset = ftell(fp);
		if (maxval == 255) {
			CHECK_MALLOC_1(data = (unsigned char*)calloc(1, x * y), 1 * x * y);
			k = x * y;
			for (i = 0; i < k; i++) {
				data[count++] = get_int(fp);
			}
			*bits = 8;
		} else if (maxval == 65535) {
			unsigned short* sdata;
			CHECK_MALLOC_1(sdata = (unsigned short*)calloc(2, x * y), 2 * x * y);
			k = x * y;
			for (i = 0; i < k; i++) {
				sdata[count++] = get_int(fp);
			}
			data  = (unsigned char*)sdata;
			*bits = 16;
		} else {
			MAX_VAL_FAILURE(maxval, "pgm", filename);
		}
		break;
	case '3': /* plain ppm format */
		maxval       = get_int(fp);
		*data_offset = ftell(fp);
		CHECK_MALLOC_1(data = (unsigned char*)calloc(3, x * y), 3 * x * y);
		for (i = 0; i < y; i++) {
			for (j = 0; j < x; j++) {
				data[count++] = get_int(fp);
				data[count++] = get_int(fp);
				data[count++] = get_int(fp);
			}
		}
		z     = 3;
		*bits = 8;
		break;
	case '4': /* raw pbm format */
		      /**
		       *** this code is roughly identical to the libpbm code.
		       *** It does not appear to be correctly compatable with
		       *** the code to convert raw to plain. Oh well.
		       **/
		CHECK_MALLOC_1(data = (unsigned char*)calloc(1, x * y), 1 * x * y);
		for (i = 0; i < y; i++) {
			bitshift = -1;
			for (j = 0; j < x; j++) {
				if (bitshift == -1) {
					d        = fgetc(fp);
					bitshift = 7;
				}
				data[count++] = ((d >> bitshift) & 1);
				bitshift--;
			}
		}
		*bits = 1;
		break;
	case '5': /* raw pgm format */
		maxval       = get_int(fp);
		*data_offset = ftell(fp);
		if (maxval <= 255) {
			CHECK_MALLOC_1(data = (unsigned char*)calloc(1, x * y), 1 * x * y);
			CHECK_READ_1(fread(data, 1, x * y, fp), 1, x * y, "pgm", filename);
			*bits = 8;
		} else if (maxval == 65535) {
			CHECK_MALLOC_1(data = (unsigned char*)calloc(2, x * y), 2 * x * y);
			CHECK_READ_1(fread(data, 2, x * y, fp), 2, x * y, "pgm", filename);
			*bits = 16;

/*
 ** I am guessing byte-swapping may be required here.
 */
#ifndef WORDS_BIGENDIAN
			swab(data, data, 2 * x * y);
#endif /* WORDS_BIGENDIAN */

		} else {
			MAX_VAL_FAILURE(maxval, "pgm", filename);
		}
		break;
	case '6': /* raw ppm format */
		maxval       = get_int(fp);
		*data_offset = ftell(fp);
		if (maxval > 255) {
			MAX_VAL_FAILURE(maxval, "ppm", filename);
		}
		CHECK_MALLOC_1(data = (unsigned char*)calloc(3, x * y), 3 * x * y);
		CHECK_READ_1(fread(data, 3, x * y, fp), 3, x * y, "ppm", filename);
		z     = 3;
		*bits = 8;
		break;
	default:
		if (iom_is_ok2print_unsupp_errors()) {
			fprintf(stderr, "Unsupported PNM format %d.\n", format);
		}
		return (0);
	}

	*xout = x;
	*yout = y;
	*zout = z;
	*dout = data;

	return (1);
}

static int get_int(FILE* fp)
{
	int i;
	int c;

	while ((c = getc(fp)) != EOF) {
		if (c == '#') {
			while (c != '\n') c = getc(fp);
		}
		if (c >= '0' && c <= '9') {
			i = 0;
			while (c >= '0' && c <= '9') {
				i = i * 10 + (c - '0');
				c = getc(fp);
			}
			return (i);
		}
	}
	return (-1);
}

static int getbit(FILE* fp)
{
	int c;

	while ((c = getc(fp)) != EOF) {
		if (c == '#') {
			while (c != '\n') c = getc(fp);
		}
		if (c == '0' || c == '1') {
			return (c - '0');
		}
	}
	return (-1);
}
