#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#include <string.h>
#include "iomedley.h"

/**
 ** These are fortran sequential unformatted records.
 **
 ** Each line is prefixed and suffixed with an int containing the
 ** size of the record, not including the prefix and suffix values.
 **/

struct GRD {
    char id[56];                /* Title */
    char pgm[8];                /* program that generated this file */
    int ncol, nrow, nz;         /* width, height, depth */
    float xo, dx, yo, dy;       /* cartesian location and scaling factors */
};

int
iom_isGRD(FILE *fp)
{
    char buf[256];
    int size,size2;

    /**
     ** Get record size.
     **/

    rewind(fp);
    fread(&size, sizeof(int), 1, fp);
#ifndef WORDS_BIGENDIAN
    iom_MSB4((char *)&size);
#endif /* WORDS_BIGENDIAN */
    
    if (size == 104 || size == 92) {
        /**
         ** read record, and get trailing record size
         **/
        fread(buf, size, 1, fp);
        fread(&size2, sizeof(int), 1, fp);
#ifndef WORDS_BIGENDIAN
        iom_MSB4((char *)&size2);
#endif /* WORDS_BIGENDIAN */
        
        if (size == size2) {
            return 1;
        }
    }
    return 0;
}

int
iom_GetGRDHeader(
    FILE *fp,
    char *filename,
    struct iom_iheader *h
    )
{
    struct GRD *grd;
    char buf[256];
    int size;


    iom_init_iheader(h);

    if (!iom_isGRD(fp)){
      return 0;
    }
    
    rewind(fp);
    
    /* Get record size */
    fread(&size, sizeof(int), 1, fp);
#ifndef WORDS_BIGENDIAN
    iom_MSB4((char *)&size);
#endif /* WORDS_BIGENDIAN */
    
    fread(buf, size, 1, fp);
    
    grd = (struct GRD *)buf;
#ifndef WORDS_BIGENDIAN
    /*
    ** byte-swap the integers in the header
    ** ieee floats have the same format though
    */
    iom_MSB4((char *)&grd->ncol);
    iom_MSB4((char *)&grd->nrow);
    iom_MSB4((char *)&grd->nz);
#endif /* WORDS_BIGENDIAN */
    
	if (iom_is_ok2print_details()){
		fprintf(stderr, "GRD file - ");
		fprintf(stderr, "Title: %s, program: %s\n", grd->id, grd->pgm);
		fprintf(stderr, "Size: %dx%d [%d]\n", grd->ncol, grd->nrow, grd->nz);
		fprintf(stderr, "X,Y: %f,%f\tdx,dy: %f,%f\n", 
				grd->xo,grd->yo,grd->dx,grd->dy);
	}

    h->dptr = size+8;
    h->prefix[0]= 8;
    h->suffix[0]= 4;
    h->size[0] = grd->ncol;
    h->size[1] = grd->nrow;
    h->size[2] = grd->nz;
    h->eformat = iom_MSB_IEEE_REAL_4;
    h->format = iom_FLOAT;
    h->org = iom_BSQ;
    
    /* iom_iheader does not have h->title */
    /* h->title = strdup(grd->id); */

	return(1);
}


/*
** WriteGRD()
**
** Given data in BSQ format; write it to a GRD file in float format.
*/

int
iom_WriteGRD(
    char *fname,
    void *data,
    struct iom_iheader *h,
    int force_write,
    char *title,
    char *pgm
    )
{
    struct GRD *grd = NULL;
    char buf[256];
    int size;
    int x,y,z;
    float *fptr = (float *)data;
    int   *iptr = (int *)data;
    short *sptr = (short *)data;
    char  *bptr = (char *)data;
    double *dptr = (double *)data;
    int i, j;
    float f;
    FILE *fp = NULL;

    if (!force_write && access(fname, F_OK) == 0){
		if (iom_is_ok2print_errors()){
			fprintf(stderr, "File %s already exists.\n", fname);
		}
        return 0;
    }

    if ((fp = fopen(fname, "wb")) == NULL){
		if (iom_is_ok2print_sys_errors()){
			fprintf(stderr, "Unable to write file %s. Reason: %s.\n",
					fname, strerror(errno));
		}
        return 0;
    }
    
    grd = (struct GRD *)buf;

    strcpy(grd->id, (title ? title : "default"));
    sprintf(grd->pgm, "%-8s", pgm);
    
    x = grd->ncol = iom_GetSamples(h->size, h->org);
    y = grd->nrow = iom_GetLines(h->size, h->org);
    z = grd->nz = iom_GetBands(h->size, h->org);
    
    grd->xo = grd->yo = 0.0;
    grd->dx = grd->dy = 1.0;

    if (z != 1) {
		if (iom_is_ok2print_errors()){
			fprintf(stderr, "Cannot write GRD files with more than 1 band.\n");
		}
        fclose(fp);
        unlink(fname);
        return 0;
    }
    
    size = sizeof(struct GRD);
    
#ifndef WORDS_BIGENDIAN
    iom_LSB4((char *)&size);
    iom_LSB4((char *)&grd->ncol);
    iom_LSB4((char *)&grd->nrow);
    iom_LSB4((char *)&grd->nz);
#endif /* WORDS_BIGENDIAN */
    
    fwrite(&size, 1, sizeof(int), fp);
    fwrite(grd, 1, size, fp);
    fwrite(&size, 1, sizeof(int), fp);

    size = (x+1)*4;
    
#ifndef WORDS_BIGENDIAN
    iom_LSB4((char *)&size);
#endif /* WORDS_BIGENDIAN */


    for (i = 0 ; i < y ; i++) {
        fwrite(&size, 1, sizeof(int), fp);
        fwrite("\0\0\0\0", 1, 4, fp);
        for (j = 0 ; j < x ; j++) {
            switch(h->format){
            case iom_BYTE:   f = (float)bptr[j+i*x]; break;
            case iom_SHORT:  f = (float)sptr[j+i*x]; break;
            case iom_INT:    f = (float)iptr[j+i*x]; break;
            case iom_FLOAT:  f =        fptr[j+i*x]; break;
            case iom_DOUBLE: f = (float)dptr[j+i*x]; break;
            default:
				if (iom_is_ok2print_unsupp_errors()){
					fprintf(stderr, "Unsupported internal file format for GRD file.\n");
					fprintf(stderr, "See file: %s  line: %d.\n", __FILE__, __LINE__);
				}
				fclose(fp);
                return 0;
            }
#ifndef WORDS_BIGENDIAN
            /* LSB -> MSB */
            iom_MSB4((char *)&f);
#endif /* WORDS_BIGENDIAN */
            fwrite(&f, 1, sizeof(float), fp);
        }
        fwrite(&size, 1, sizeof(int), fp);
    }

	if (ferror(fp)){
		if (iom_is_ok2print_sys_errors()){
			fprintf(stderr, "Unable to write to %s. Reason: %s.\n",
				fname, strerror(errno));
		}
		fclose(fp);
		unlink(fname);
		return 0;
	}

	fclose(fp);
    return 1;
}
