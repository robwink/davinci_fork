#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include "iomedley.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 ** Write data as ER Mapper ERS data set
 **/

#ifdef WORDS_BIGENDIAN
static char byte_order[] = "MSBFirst";
#else  /* little endian */
static char byte_order[] = "LSBFirst";
#endif /* WORDS_BIGENDIAN */

static char ers_header[] =
    "DatasetHeader Begin\n\
	Version = \"5.0\"\n\
	DataSetType = ERStorage\n\
	DataType = Raster\n\
	ByteOrder = %s\n\
	CoordinateSpace Begin\n\
		Datum = \"RAW\"\n\
		Projection = \"RAW\"\n\
		CoordinateType = RAW\n\
		Rotation = 0:0:0.0\n\
	CoordinateSpace End\n\
	RasterInfo Begin\n\
		CellType = %s\n\
		NrOfLines = %d\n\
		NrOfCellsPerLine = %d\n\
		NrOfBands = %d\n\
	RasterInfo End\n\
DatasetHeader End\n";

/*
** NOTE:
**
** This function should be broken down into two parts:
** 1) WriteERS() which is "iomedley" independent
** 2) iom_WriteERS() which is "iomedley" dependent
**    This function should be moved to iomedley.c
*/

int iom_WriteERS(char* fname, void* data, struct iom_iheader* h, int force_write)
{
	char format[256];
	char lblfname[512]; /* Label file name */
	int x, y, z;
	FILE* fp = NULL;
	int items_in, items_out;

	sprintf(lblfname, "%s.ers", fname);

	if (!force_write && (file_exists(fname) || file_exists(lblfname))) {
		fprintf(stderr, "File %s or %s exists.\n", fname, lblfname);
		return 0;
	}

	x = iom_GetSamples(h->size, h->org);
	y = iom_GetLines(h->size, h->org);
	z = iom_GetBands(h->size, h->org);

	if (z > 1 && h->org != iom_BIL) {
		if (iom_is_ok2print_errors()) {
			fprintf(stderr, "ERS files must be BIL format");
		}
		return 0;
	}

	if ((fp = fopen(fname, "wb")) == NULL) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "Unable to open %s. Reason: %s.\n", fname, strerror(errno));
		}
		return 0;
	}

	/**
	 ** write data
	 **/
	items_in  = iom_iheaderDataSize(h);
	items_out = fwrite(data, iom_iheaderItemBytesI(h), items_in, fp);

	if (items_in != items_out) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "Write to %s failed. Reason: %s.\n", fname, strerror(errno));
		}
		fclose(fp);
		unlink(fname);
		return 0;
	}

	fclose(fp);

	/*
	** write header
	*/
	if ((fp = fopen(lblfname, "w")) == NULL) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "Unable to write %s. Reason: %s.\n", lblfname, strerror(errno));
		}
		unlink(fname);
		return 0;
	}

	switch (h->format) {
	case iom_BYTE: sprintf(format, "Unsigned8BitInteger"); break;
	case iom_SHORT: sprintf(format, "Signed16BitInteger"); break;
	case iom_INT: sprintf(format, "Signed32BitInteger"); break;
	case iom_FLOAT: sprintf(format, "IEEE4ByteReal"); break;
	case iom_DOUBLE: sprintf(format, "IEEE8ByteReal"); break;
	}

	items_out = fprintf(fp, ers_header, byte_order, format, y, x, z);
	if (items_out < 0) {
		if (iom_is_ok2print_sys_errors()) {
			fprintf(stderr, "Unable to write to %s. Reason: %s.\n", lblfname, strerror(errno));
		}
		fclose(fp);
		unlink(fname);
		unlink(lblfname);
		return 0;
	}

	fclose(fp);

	return 1;
}

