#include "parser.h"
#include "dvio.h"
#include "iomedley.h"


Var *
dv_LoadVicar(
    FILE *fp,
    char *filename,
    struct iom_iheader *s
    )
{
    struct iom_iheader h;
    int org;
    Var *v;
    void *data;
    char hbuf[HBUFSIZE];
	int i, j;

    if (iom_isVicar(fp) == 0){ return NULL; }
    
    if (iom_GetVicarHeader(fp, filename, &h) == 0) { return(NULL); }

    /**
     ** If user specified a record, subset out a specific band
     **/

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

    sprintf(hbuf, "%s: VICAR %s image: %dx%dx%d, %d bits",
            filename, iom_Org2Str(h.org),
            iom_GetSamples(h.dim, h.org), 
            iom_GetLines(h.dim, h.org), 
            iom_GetBands(h.dim, h.org), 
            iom_NBYTESI(h.format)*8);
    if (VERBOSE > 1) {
        parse_error(hbuf);
    }
    iom_cleanup_iheader(&h);

    return(v);
}

int
dv_WriteVicar(Var *ob, char *filename, int force)
{
    struct iom_iheader h;
    int status;

    var2iom_iheader(ob, &h);
    status = iom_WriteVicar(filename, V_DATA(ob), &h, force);
    iom_cleanup_iheader(&h);

    if (status == 0){
        parse_error("Writing VICAR file %s failed.\n", filename);
        return 0;
    }
    
    return  1;
}
