#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include "iomedley.h"

#define BUF_SIZE 256
#define LABEL_SIZE_STRING "Label Size:"

/**
 ** This (hopefully) detects AVIRIS files.
 ** returns:
 **             0: on failure
 **             1: on success
 **/
int iom_isAVIRIS(FILE* fp)
{
	char buf[BUF_SIZE];

	rewind(fp);
	memset(buf, 0, sizeof(char) * BUF_SIZE);
	fgets(buf, BUF_SIZE, fp);
	if (buf[24] != '\n') {
		return (0);
	}

	memset(buf, 0, sizeof(char) * BUF_SIZE);
	fgets(buf, BUF_SIZE, fp);
	if (strncmp(buf, LABEL_SIZE_STRING, strlen(LABEL_SIZE_STRING))) {
		return (0);
	}
	return (1);
}

/**
 ** GetAVIRISHeader() - read and parse an aviris header
 **
 ** This routine returns
 **         0 if the specified file is not an AVIRIS file.
 **         1 on success
 **/
int iom_GetAVIRISHeader(FILE* fp, char* fname, struct iom_iheader* h)
{
	int i;

	int org = -1, format = -1, label = 0;
	int size[3], suffix[3];
	char buf[BUF_SIZE];

	/**
	** Read enough to get identifying label and total label size.
	** Get total label size and mallocate enough space to hold it.
	**/

	rewind(fp);

	memset(buf, 0, sizeof(char) * BUF_SIZE);
	fgets(buf, BUF_SIZE, fp);
	if (buf[24] != '\n') {
		return (0);
	}

/**
** assume that this is really an AVIRIS file, in which case, decode
** the buffer.
**/

#ifndef WORDS_BIGENDIAN
	/* it is convenient to byte-swap the data before it is assigned */
	for (i = 0; i < 6; i++) {
		iom_MSB4((char*)&((int*)buf)[i]);
	}
#endif /* WORDS_BIGENDIAN */

	label     = ((int*)buf)[0];
	format    = ((int*)buf)[1];
	size[1]   = ((int*)buf)[2];
	size[2]   = ((int*)buf)[3];
	size[0]   = ((int*)buf)[4];
	org       = ((int*)buf)[5];
	suffix[0] = suffix[1] = suffix[2] = 0;

	memset(buf, 0, sizeof(char) * BUF_SIZE);
	fgets(buf, BUF_SIZE, fp);
	if (strncmp(buf, LABEL_SIZE_STRING, strlen(LABEL_SIZE_STRING))) {
		return (0);
	}

	/**
	** Right now, the only files we've seen indicate BIL=1.
	** That matches davinci, so go with it.
	**/
	if (org != 1 /* iom_BIL */) {
		if (iom_is_ok2print_errors()) {
			fprintf(stderr,
			        "io_aviris(): Unrecognized org.\n"
			        "please mail this whole file to gorelick@asu.edu\n");
		}
		return (0);
	}

	/**
	** Right now, the only files we've seen indicate SHORT=2.
	** That matches davinci, so go with it.
	**/
	if (format != 2 /* iom_SHORT */) {
		if (iom_is_ok2print_errors()) {
			fprintf(stderr,
			        "io_aviris(): Unrecognized format.\n"
			        "please mail this whole file to gorelick@asu.edu\n");
		}
		return (0);
	}

	/**
	** Put together a proper structure from the read values.
	**/

	/* memset(h, '\0', sizeof(struct _iheader)); */
	iom_init_iheader(h);

	h->dptr       = label;
	h->byte_order = 4321; /* can this be reliably determined? */
	h->eformat    = iom_MSB_INT_2;
	h->format     = format; /* SHORT */
	h->org        = org;
	h->gain       = 1.0;
	h->offset     = 0.0;
	for (i = 0; i < 3; i++) {
		h->size[iom_orders[org][i]]   = size[i];
		h->suffix[iom_orders[org][i]] = suffix[i];
	}

	return (1);
}
