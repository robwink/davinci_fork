#include "parser.h"

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
is_GRD(FILE *fp)
{
    char buf[256];
    int size,size2;

    /**
     ** Get record size.
     **/

    rewind(fp);
    fread(&size, sizeof(int), 1, fp);
    if (size == 104 || size == 92) {
        /**
         ** read record, and get trailing record size
         **/
        fread(buf, size, 1, fp);
        fread(&size2, sizeof(int), 1, fp);
        if (size == size2) {
            return 1;
        }
    }
    return 0;
}

int
LoadGRDHeader(FILE *fp, char *filename, struct _iheader *h)
{
    struct GRD *grd;
    char buf[256];
    int size,size2;

    /**
     ** Get record size, and verify type
     **/
    rewind(fp);
    fread(&size, sizeof(int), 1, fp);
    if (size != 104 && size != 92) {
        return(0);
    }

    /**
     ** read record, and verify that it is correct length.
     **/
    fread(buf, size, 1, fp);
    fread(&size2, sizeof(int), 1, fp);

    if (size != size2) {
        sprintf(error_buf, "Error in GRD file: %s", filename);
        parse_error(NULL);
        return(0);
    }

    grd = (struct GRD *)buf;
    if (VERBOSE > 1) {
        fprintf(stderr, "GRD file -");
        fprintf(stderr, "Title: %56.56s, program: %8.8s\n", grd->id, grd->pgm);
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
    h->format = FLOAT;
    h->org = BSQ;

	return(1);
}



Var *
LoadGRD(FILE *fp, char *filename, struct _iheader *s)
{
    void *data;
    Var *v;
	struct _iheader h;
    struct GRD *grd;
    char buf[256];
    int size,size2;

    /**
     ** Get record size, and verify type
     **/
    rewind(fp);
    fread(&size, sizeof(int), 1, fp);
    if (size != 104 && size != 92) {
        return(NULL);
    }

    /**
     ** read record, and verify that it is correct length.
     **/
    fread(buf, size, 1, fp);
    fread(&size2, sizeof(int), 1, fp);

    if (size != size2) {
        sprintf(error_buf, "Error in GRD file: %s", filename);
        parse_error(NULL);
        return(NULL);
    }

    grd = (struct GRD *)buf;
    if (VERBOSE > 1) {
        fprintf(stderr, "GRD file -");
        fprintf(stderr, "Title: %56.56s, program: %8.8s\n", grd->id, grd->pgm);
        fprintf(stderr, "Size: %dx%d [%d]\n", grd->ncol, grd->nrow, grd->nz);
        fprintf(stderr, "X,Y: %f,%f\tdx,dy: %f,%f\n", 
                grd->xo,grd->yo,grd->dx,grd->dy);
    }

    h.dptr = size+8;
    h.prefix[0]= 8;
    h.suffix[0]= 4;
    h.size[0] = grd->ncol;
    h.size[1] = grd->nrow;
    h.size[2] = grd->nz;
    h.format = FLOAT;
    h.org = BSQ;

    data = read_qube_data(fileno(fp), &h);
    v = iheader2var(&h);

    V_DATA(v) = data;

    V_TITLE(v) = strdup(grd->id);

    return(v);
}

int
WriteGRD(Var *s, FILE *fp, char *filename)
{
    /**
     ** can only write float values
     **/
    struct GRD *grd;
    char buf[256];
    int size;
    int x,y,z;
    float *fptr, f;
    int i,j;


    grd = (struct GRD *)buf;

    strcpy(grd->id, V_TITLE(s));
    strncpy(grd->pgm, "dv           ",8);
    x = grd->ncol = GetSamples(V_SIZE(s), V_ORG(s));
    y = grd->nrow = GetLines(V_SIZE(s), V_ORG(s));
    z = grd->nz = GetBands(V_SIZE(s), V_ORG(s));
    grd->xo = grd->yo = 0.0;
    grd->dx = grd->dy = 1.0;

    if (V_TYPE(s) != ID_VAL) {
        sprintf(error_buf, "Var is not a value: %s", V_NAME(s));
        parse_error(NULL);
        return 0;
    }

    if (grd->nz != 1) {
        parse_error("Cannot write GRD files with more than 1 band");
        return 0;
    }

    if (fp == NULL) {
        if ((fp = fopen(filename, "w")) == NULL) {
            return 0;
        }
    }

    size = sizeof(struct GRD);
    fwrite(&size, 1, sizeof(int), fp);
    fwrite(grd, 1, size, fp);
    fwrite(&size, 1, sizeof(int), fp);

    size = (x+1)*4;

    if (V_FORMAT(s) == FLOAT) {
        fptr = (float *)V_DATA(s);
        for (i = 0 ; i < y ; i++) {
            fwrite(&size, 1, sizeof(int), fp);
            fwrite("\0\0\0\0", 1, 4, fp);
            fwrite(fptr+i*x, sizeof(float), x, fp);
            fwrite(&size, 1, sizeof(int), fp);
        }
    } else {
        for (i = 0 ; i < y ; i++) {
            fwrite(&size, 1, sizeof(int), fp);
            fwrite("\0\0\0\0", 1, 4, fp);
            for (j = 0 ; j < x ; j++) {
                f = extract_float(s, j+i*x);
                fwrite(&f, 1, sizeof(float), fp);
            }
            fwrite(&size, 1, sizeof(int), fp);
        }
    }
    fclose(fp);
    return (0);
}
