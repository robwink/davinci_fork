#include "parser.h"
#include "dvio.h"
#include "iomedley.h"

Var *
dv_LoadIMath(
    FILE *fp,
    char *filename,
    struct iom_iheader *s
    )
{
    char buf[256];
    double *data;
    struct iom_iheader h;
    int i,j, k;
    Var *v;
    int transpose;
    int wt, ht;
    int status;

    /**
    ** Get record size, and verify type
    **/
    if (iom_isIMath(fp) == 0) return(NULL);

    status = iom_GetIMathHeader(fp, filename, &h);
    if (status == 0){ return NULL; }

    if (s){
        iom_MergeHeaderAndSlice(&h, s);
    }

    ht = h.dim[0];
    wt = h.dim[1];
	
    if (VERBOSE > 1) {
        fprintf(stderr, "Imath file: %d x %d doubles\n", wt, ht);
    }

    data = (double *)iom_read_qube_data(fileno(fp), &h);

    if (h.transposed) {
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

    v = iom_iheader2var(&h);
    V_DATA(v) = data;

    iom_cleanup_iheader(&h);

    return(v);
}
