#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include "iomedley.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void print_data(void* data, int x, int y, int z, iom_idf t, iom_order o);

int main(int ac, char* av[])
{
	struct iom_iheader h;
	FILE* fp;
	char* fname;
	void* data;
	int x, y, z;
	iom_idf t;
	iom_order o;

	if (ac != 2) {
		fprintf(stderr, "Usage: %s file\n", av[0]);
		return 1;
	}

	fname = av[1];

	/* open file */
	if ((fp = fopen(fname, "rb")) == NULL) {
		fprintf(stderr, "%s: File: %s: %s\n", av[0], fname, strerror(errno));
	}

	/* load header */
	if (iom_LoadHeader(fp, fname, &h)) {

		/* print header */
		iom_PrintImageHeader(stdout, fname, &h);
		printf("\n");

		/* load data */
		data = (char*)iom_read_qube_data(fileno(fp), &h);

		x = iom_GetSamples(h.dim, h.org);
		y = iom_GetLines(h.dim, h.org);
		z = iom_GetBands(h.dim, h.org);
		t = h.format;
		o = h.org;

		/* print data */
		print_data(data, x, y, z, t, o);

		/* cleanup */
		iom_cleanup_iheader(&h);
	} else {
		fprintf(stderr, "%s: File: %s cannot be opened using iomedley.\n", av[0], fname);
		fclose(fp);
		return 1;
	}

	fclose(fp);

	return 0;
}

void print_data(void* data, int x, int y, int z, iom_idf t, iom_order o)
{
	int i, j, k;
	int c;

	/* let's print just one band */

	k = 0;
	for (j = 0; j < y; j++) {
		for (i = 0; i < x; i++) {
			switch (o) {
			case iom_BSQ: c = i + j * x + k * y * x; break;
			case iom_BIL: c = i + j * x * z + k * x; break;
			case iom_BIP: c = k + i * z + j * x * z; break;
			default: fprintf(stderr, "Unknown org %d\n", o); break;
			}

			switch (t) {
			case iom_BYTE: printf("%d ", ((unsigned char*)data)[c]); break;
			case iom_SHORT: printf("%d ", ((short*)data)[c]); break;
			case iom_INT: printf("%d ", ((long*)data)[c]); break;
			case iom_FLOAT: printf("%g ", ((float*)data)[c]); break;
			case iom_DOUBLE: printf("%g ", ((double*)data)[c]); break;
			default: fprintf(stderr, "Unknown format %d\n", t); break;
			}
		}
		printf("\n");
	}
}

