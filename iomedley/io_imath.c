#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#undef strncasecmp
#define strncasecmp strnicmp
#else
#include <unistd.h>
#endif /* _WIN32 */
#include "iomedley.h"


/**
 ** These are fortran sequential unformatted records.
 **
 ** Each line is prefixed and suffixed with an int containing the
 ** size of the record, not including the prefix and suffix values.
 **/

#define BINARY_LABEL "vector_file"

int
iom_isIMath(FILE *fp)
{
    char buf[256];

    /**
    ** Get record size.
    **/

    rewind(fp);
    fread(buf, strlen(BINARY_LABEL), 1, fp);

    if (ferror(fp)){
        perror(NULL);
        return 0;
    }
	
    if (!strncasecmp(buf, BINARY_LABEL, strlen(BINARY_LABEL))) {
        return(1);
    }
    return 0;
}


int
iom_GetIMathHeader(
    FILE *fp,
    char *filename,
    struct iom_iheader *h
)
{
    char buf[256];
    int  wt, ht;
	int  transpose;

    if (iom_isIMath(fp) == 0){ return 0; }
    
    rewind(fp);
    fread(buf, strlen(BINARY_LABEL)+1, 1, fp);
    fread(&wt, sizeof(int), 1, fp);
    fread(&ht, sizeof(int), 1, fp);

#ifndef WORDS_BIGENDIAN
    /* convert MSB -> LSB */

    iom_MSB4((char *)&wt);
    iom_MSB4((char *)&ht);
#endif

    transpose = (buf[0] == 'V');


    iom_init_iheader(h);

    h->dptr = strlen(BINARY_LABEL)+9;
    h->size[0] = ht;
    h->size[1] = wt;
    h->size[2] = 1;
    h->eformat = iom_MSB_IEEE_REAL_8;
    h->format = iom_DOUBLE;
    h->org = iom_BSQ;
    h->transposed = transpose;
    h->prefix[0] = h->prefix[1] = h->prefix[2] = 0;
    h->suffix[0] = h->suffix[1] = h->suffix[2] = 0;

    return 1;
}


int
iom_WriteIMath(
    char *filename,
    void *data,
    struct iom_iheader *h,
    int force_write
    )
{
    int i, j;
    double d;
	int height, width;
    FILE *fp = NULL;

    if (!force_write && access(filename, F_OK)){
		if (iom_is_ok2print_errors()){
			fprintf(stderr, "File %s already exists.\n", filename);
		}
        return 0;
    }

    if ((fp = fopen(filename, "wb")) == NULL){
		if (iom_is_ok2print_sys_errors()){
			fprintf(stderr, "Unable to write file %s. Reason: %s.\n",
					filename, strerror(errno));
		}
        return 0;
    }

    errno = 0; /* Avoid checking for errno at every instance */
    
    /* Write header */
    fwrite(BINARY_LABEL, strlen(BINARY_LABEL)+1, 1, fp);

    /* Get image width and height */
    height = iom_GetLines(h->size, h->org);
    width = iom_GetSamples(h->size, h->org);

#ifndef WORDS_BIGENDIAN
    /* Convert LSB -> MSB */
    iom_LSB4((char *)&width);
    iom_LSB4((char *)&height);
#endif

    fwrite(&width, 4, 1, fp);
    fwrite(&height, 4, 1, fp);

    /* Write data. */
    for (i = 0 ; i < width ; i++) {
        for (j = 0 ; j < height ; j++) {
            d = ((double *)data)[j*width+i];
            
#ifndef WORDS_BIGENDIAN
            /* Convert LSB -> MSB */
            iom_LSB4((char *)&d);
#endif /* WORDS_BIGENDIAN */
            
            fwrite(&d, 8, 1, fp);
        }
    }

    if (ferror(fp)){ /* Catch all write errors here. */
		if (iom_is_ok2print_sys_errors()){
			fprintf(stderr, "Error writing file %s. Reason: %s.\n",
					filename, strerror(errno));
		}
        fclose(fp);
        unlink(filename);
        
        return 0;
    }
    
    fclose(fp);

    return 1;
}
