#include "parser.h"

/**
 ** These are fortran sequential unformatted records.
 **
 ** Each line is prefixed and suffixed with an int containing the
 ** size of the record, not including the prefix and suffix values.
 **/

#define BINARY_LABEL "vector_file"

int
is_imath(FILE *fp)
{
    char buf[256];

    /**
    ** Get record size.
    **/

    rewind(fp);
    fread(buf, strlen(BINARY_LABEL), 1, fp);
	
    if (!strncasecmp(buf, BINARY_LABEL, strlen(BINARY_LABEL))) {
        return(1);
    }
    return 0;
}

Var *
Load_imath(FILE *fp, char *filename, struct _iheader *s)
{
    char buf[256];
    double *data;
    struct _iheader h;
    int i,j, k;
    Var *v;
    int transpose;
    int wt, ht;

    /**
    ** Get record size, and verify type
    **/
    if (is_imath(fp) == 0) return(NULL);
	
    rewind(fp);
    fread(buf, strlen(BINARY_LABEL)+1, 1, fp);
    fread(&wt, sizeof(int), 1, fp);
    fread(&ht, sizeof(int), 1, fp);

    transpose = (buf[0] == 'V');

    if (VERBOSE > 1) {
        fprintf(stderr, "Imath file: %d x %d doubles\n", wt, ht);
    }

    memset(&h, 0, sizeof(h));
    h.dptr = strlen(BINARY_LABEL)+9;
    h.size[0] = ht;
    h.size[1] = wt;
    h.size[2] = 1;
    h.format = DOUBLE;
    h.org = BSQ;
    h.prefix[0] = h.prefix[1] = h.prefix[2] = 0;
    h.suffix[0] = h.suffix[1] = h.suffix[2] = 0;

    data = (double *)read_qube_data(fileno(fp), &h);

    if (transpose) {
        double *data2 = (double *)calloc(wt * ht, sizeof(double));
        if (VERBOSE > 1) fprintf(stderr, "transposed\n");

        k = 0;
        for (i = 0 ; i < ht ; i++) {				/* 13 */
            for (j = 0 ; j < wt ; j++) {			/* 8 */
                data2[ ((k%ht) * wt)  + k/ht ] = data[k];
                k++;
            }
        }
        i = h.dim[0];
        h.dim[0] = h.dim[1];
        h.dim[1] = i;
        free(data);
        data  = data2;
    }

    v = iheader2var(&h);
    V_DATA(v) = data;

    return(v);
}

int
WriteIMath(Var *s, FILE *fp, char *filename)
{
    int w,h;
	int i,j;
	double d;

    if (VERBOSE > 1) {
        fprintf(stderr, "Writing %s: Imath file %dx%d\n",
				filename,
                GetSamples(V_SIZE(s), V_ORG(s)),
                GetLines(V_SIZE(s), V_ORG(s)));
        fflush(stderr);
    }
	w = GetSamples(V_SIZE(s), V_ORG(s));
	h = GetLines(V_SIZE(s), V_ORG(s));

    if (fp == NULL) {
        if ((fp = fopen(filename, "w")) == NULL) {
			return(0);
		}
    }

	fwrite(BINARY_LABEL, strlen(BINARY_LABEL)+1, 1, fp);
	fwrite(&w, 4, 1, fp);
	fwrite(&h, 4, 1, fp);
    /**
     ** write data
     **/
	for (i = 0 ; i < w ; i++) {
		for (j = 0 ; j < h ; j++) {
			d = extract_double(s, j*w+i);
			fwrite(&d, 8, 1, fp);
		}
	}
    fclose(fp);

    return(1);
}
