#include "parser.h"
#include "dvio.h"
#include "iomedley.h"

Var *
dv_LoadGRD(FILE *fp, char *filename, struct iom_iheader *s)
{
    void *data;
    Var *v;
	struct iom_iheader h;
    struct GRD *grd;
    char buf[256];
    int size,size2;
    int status;

    if (!iom_isGRD(fp)){ return NULL; }

    /**
     ** Get record size, and verify type
     **/
    status = iom_GetGRDHeader(fp, filename, &h);
    if (status == 0){ return NULL; }

    if (s){
        iom_MergeHeaderAndSlice(&h, s);
    }

    data = iom_read_qube_data(fileno(fp), &h);
    v = iom_iheader2var(&h);

    V_DATA(v) = data;
    V_TITLE(v) = strdup(h.title);

    iom_cleanup_iheader(&h);
    
    return(v);
}

int
dv_WriteGRD(Var *s, FILE *fp, char *filename)
{
    struct iom_iheader h;
    int status;

    if (V_TYPE(s) != ID_VAL) {
        sprintf(error_buf, "Var is not a value: %s", V_NAME(s));
        parse_error(NULL);
        return 0;
    }

    if (GetBands(V_SIZE(s), V_ORG(s)) != 1) {
        parse_error("Cannot write GRD files with more than 1 band");
        return 0;
    }
    
    iom_var2iheader(s, &h);
    
    status = iom_WriteGRD(fp, filename, V_DATA(s), &h, V_TITLE(s), "dv           ");
    if (status == 0){
        parse_error("Writing %s failed.\n", filename);
    }
    
    return (status == 1);
}
