/*********************************** envi.c **********************************/
#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "iomedley.h"

#ifdef _WIN32
#undef strncasecmp
#define strncasecmp strnicmp
#else
#include <unistd.h>
#include <pwd.h>
#endif /* _WIN32 */


#define ENVI_MAGIC		"ENVI"

/**
 ** ENVI I/O routines
 **
 ** is_ENVI()  - detect ENVI magic cookie
 ** GetENVIHeader() - read and parse a ENVI header
 ** LoadENVI() - Load ENVI data file
 **/

/**
 ** This detects the magic cookie for envi files.
 ** returns:
 **             0: on failure
 **             1: on success
 **/
int
iom_isENVI(FILE *fp)
{
    int len;
    char buf[16];

    rewind(fp);
    len = fread(buf, 1, strlen(ENVI_MAGIC), fp);
    return (len == strlen(ENVI_MAGIC) && !strncmp(buf, ENVI_MAGIC, len));
}


/*
** Retruns VALUE from a string of the form:
**
**       KEYWORD=VALUE
**
** where:
**   s1 is of the form "KEYWORD="
**   s2 is the string
*/
static char *
envi_get_value(const char *s1, const char *s2)
{
    char *p, *q;

	p = strstr(s1, s2);
	if (p) {
		q = p+strlen(s2);
		while (*q == '=' || *q == ' ') q++;
		return(q);
	}

    return(NULL);
}


/**
 ** GetENVIHeader() - read and parse a envi header
 **
 ** This routine returns
 **         0 if the specified file is not a ENVI file.
 **         1 on success
 **/

int
iom_GetENVIHeader(FILE *fp, char *fname, struct iom_iheader *h)
{
    char *p, *q;
    int i;

    int org=-1;
    int format=-1;
	int offset = 0;

    int size[3], suffix[3], prefix[3];
	char buf[1024];


    /**
     **/

    rewind(fp);

    fread(buf, 1, 1024, fp);
    buf[1023] = '\0';
	if (strncmp(buf, "ENVI",4)) {
        return(0);
    }
	p = buf;

    size[0] =   atoi(envi_get_value(p, "samples")); /* width */
    size[1] =   atoi(envi_get_value(p, "lines")); /* height */
    size[2] =   atoi(envi_get_value(p, "bands")); /* depth */

    suffix[0] = suffix[1] = suffix[2] = 0;
    prefix[0] = prefix[1] = prefix[2] = 0;

    if ((q = envi_get_value(p, "header offset")) != NULL) {
		offset = atoi(q);
	}

    if ((q = envi_get_value(p, "interleave")) != NULL) {
        if (!strncasecmp(q, "bil", 3)) org = iom_BIL;
        if (!strncasecmp(q, "bsq", 3)) org = iom_BSQ;
        if (!strncasecmp(q, "bip", 3)) org = iom_BIP;
    }
    if (org == -1) {
		if (iom_is_ok2print_unsupp_errors()){
			fprintf(stderr, "%s has no org.", fname);
		}
        return(0);
    }


    if ((q = envi_get_value(p, "data type")) != NULL) {
		switch(atoi(q)) {
			case 1:	/* byte */	format = iom_MSB_INT_1; break;
			case 2:	/* short */	format = iom_MSB_INT_2; break;
			case 3: /* int */	format = iom_MSB_INT_4; break;
			case 4:	/* float */ format = iom_MSB_IEEE_REAL_4; break;
			case 5: /* double */ format = iom_MSB_IEEE_REAL_8; break;
			case 6:	/* complex, 2x32 */
			case 9: /* complex 2x64 */
			case 12: /* 16-bit unsigned */
			case 13: /* 32-bit unsigned */
			case 14: /* 64-bit unsigned */
			case 15: /* 64-bit unsigned long integer */
				fprintf(stderr, "Envi format %d not supported\n", atoi(q));
				return(0);
		}
	}

    if ((q = envi_get_value(p, "byte order")) != NULL) {
		if (atoi(q) == 0) {
			/* format assumes MSB */
			switch (format) {
				case iom_MSB_INT_2:       format = iom_LSB_INT_2; break;
				case iom_MSB_INT_4:       format = iom_LSB_INT_4; break;
				case iom_MSB_IEEE_REAL_4: format = iom_LSB_IEEE_REAL_4; break;
				case iom_MSB_IEEE_REAL_8: format = iom_LSB_IEEE_REAL_8; break;
			}
		}
	}

    if (format == iom_EDF_INVALID) {
		if (iom_is_ok2print_unsupp_errors()){
			fprintf(stderr, "%s has unsupported/invalid format.", fname);
		}
        return(0);
    }

    /**
     ** Put together a proper structure from the read values.
     **/

    iom_init_iheader(h);

    h->dptr = offset;
    h->byte_order = 4321;       /* can this be reliably determined? */
    h->eformat = format;
    h->org = org;
    h->gain = 1.0;
    h->offset = 0.0;
    for (i = 0 ; i < 3 ; i++) {
        h->size[iom_orders[org][i]] = size[i];
        h->suffix[iom_orders[org][i]] = suffix[i];
        h->prefix[iom_orders[org][i]] = prefix[i];
    }

    return(1);
}

void
stradd(char *ptr, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void) vsprintf(ptr+strlen(ptr), fmt, ap);
	va_end(ap);
}


/*
** WriteENVI()
**
** Writes the given data into a envi file. The data should
** be in the current machine's native-internal format.
**
** This operation is not desctructive on the input data.
*/

char envi_header_init[] =  "ENVI\n"
                           "description = { davinci product }\n"
						   "file type = ENVI standard\n"
						   "header offset = 0     \n";

int
iom_WriteENVI(
    char *filename,        /* File name for reference purpose */
    void *data,            /* The data to the written out - "native" format */
    struct iom_iheader *h, /* A header describing the data */
    int force_write        /* Overwrite existing file */
    )
{
    char ptr[4096], tbuf[64];
    FILE *fp = NULL;
	int org;
	
	int x, y, z;

    if (!force_write && access(filename, F_OK) == 0) {
        fprintf(stderr, "File %s already exits.\n", filename);
        return 0;
    }

    if ((fp = fopen(filename, "wb")) == NULL){
        fprintf(stderr, "Unable to write file %s. Reason: %s.\n",
                filename, strerror(errno));
        return 0;
    }

    memset(ptr, 0, sizeof(ptr));
	stradd(ptr, envi_header_init);

    org = h->org;
	x = iom_GetSamples(h->size, org);
	y = iom_GetLines(h->size, org);
	z = iom_GetBands(h->size, org);

	stradd(ptr, "samples = %d\n", x);
	stradd(ptr, "lines = %d\n", y);
	stradd(ptr, "bands = %d\n", z);

    switch(h->format) {
		case iom_BYTE:  stradd(ptr,  "data type = 1\n"); break;
		case iom_SHORT: stradd(ptr,  "data type = 2\n"); break;
		case iom_INT:   stradd(ptr,  "data type = 3\n"); break;
		case iom_FLOAT: stradd(ptr,  "data type = 4\n"); break;
		case iom_DOUBLE: stradd(ptr,  "data type = 5\n"); break;
	}

#ifdef WORDS_BIGENDIAN
	stradd(ptr, "byte order = 1\n");
#else /* little endian */
	stradd(ptr, "byte order = 0\n");
#endif /* WORDS_BIGENDIAN */

    switch(h->org) {
		case iom_BIL: stradd(ptr, "interleave = bil\n"); break;
		case iom_BIP: stradd(ptr, "interleave = bip\n"); break;
		case iom_BSQ: stradd(ptr, "interleave = bsq\n"); break;
    }


	sprintf(tbuf, "%lu", strlen(ptr));
/*	                     01234567890123456 */
	strncpy(strstr(ptr, "header offset = 0")+16, tbuf, strlen(tbuf));
    fwrite(ptr, strlen(ptr), 1, fp);

    /*
    ** Write Data
    **
    ** This works here because we never write non-native data.
    ** i.e. we never write big-endian data on a little machine
    ** and vice-versa.
    **
    */
    fwrite(data, iom_iheaderItemBytesI(h), iom_iheaderDataSize(h), fp);

    if (ferror(fp)){
		if (iom_is_ok2print_sys_errors()){
			fprintf(stderr, "Unable to write to file %s. Reason: %s.\n",
					filename, strerror(errno));
		}
        fclose(fp);
        unlink(filename);
        return 0;
    }

    fclose(fp);
    return(1);
}
