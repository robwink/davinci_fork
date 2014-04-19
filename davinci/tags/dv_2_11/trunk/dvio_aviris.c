#include "parser.h"
#include "dvio.h"
#include "iomedley.h"

Var *
dv_LoadAVIRIS(FILE *fp, char *filename, struct iom_iheader *s)
{
    struct iom_iheader h;
    Var *v;
    void *data;
    char hbuf[HBUFSIZE];

    if (iom_isAVIRIS(fp) == 0){
        return NULL;
    }
    
    if (iom_GetAVIRISHeader(fp, filename, &h) == 0) {
        return(NULL);
    }

    if (s != NULL)  {
        /** 
        ** Set subsets.  The s struct is assumed to be in BSQ order
        **/
        iom_MergeHeaderAndSlice(&h, s);
    }

    /**
    ** Put all this into a Var struct.
    **/
        
    data = iom_read_qube_data(fileno(fp), &h);
    
    v = iom_iheader2var(&h);
    V_DATA(v) = data;

    sprintf(hbuf, "%s: AVIRIS %s image: %dx%dx%d, %d bits",
            filename, iom_Org2Str(h.org),
            iom_GetSamples(h.dim, h.org), 
            iom_GetLines(h.dim, h.org), 
            iom_GetBands(h.dim, h.org), 
            iom_NBYTESI(h.format)*8);
    
    if (VERBOSE > 1) {
        parse_error(hbuf);
    }

    return(v);
}
