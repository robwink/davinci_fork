#include <stdlib.h>
#include "parser.h"
#include "dvio.h"
#include "dvio_specpr.h"


/**
 ** return string telling file type, suitable for using as type=$N in write()
 **/

Var *
ff_filetype(vfuncptr func, Var * arg)
{
    Var *v, *s = NULL, *e;
    char *filename = NULL, *fname;
    FILE *fp;
    char *p;
    char *ostring = NULL;
	char format[256];

    Alist alist[2];
    alist[0] = make_alist( "filename",    ID_STRING,    NULL,    &filename);
    alist[1].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);
    if (filename == NULL) {
        parse_error("No filename specified: %s()", func->name);
        return (NULL);
    }

    /** 
     ** if open file fails, check for record suffix
     **/

    if ((fname = dv_locate_file(filename)) == NULL) {
        if ((p = strchr(filename, SPECPR_SUFFIX)) != NULL) {
            *p = '\0';
            fname = dv_locate_file(filename);
        }
        if (fname == NULL) {
            sprintf(error_buf, "Cannot find file: %s", filename);
            parse_error(NULL);
            return (NULL);
        }
    }
    if (fname && (fp = fopen(fname, "rb")) != NULL) {
        if (iom_is_compressed(fp))
            fp = iom_uncompress(fp, fname);

        if (ostring == NULL && is_specpr(fp)) ostring = strdup("SPECPR");
        if (ostring == NULL && iom_isVicar(fp)) ostring = strdup("VICAR");
        if (ostring == NULL && iom_isISIS(fp)) ostring = strdup("ISIS");
        if (ostring == NULL && iom_isGRD(fp)) ostring = strdup("GRD");
        if (ostring == NULL && iom_isAVIRIS(fp)) ostring = strdup("AVIRIS");
        if (ostring == NULL && iom_isPNM(fp)) ostring = strdup("PNM");
        if (ostring == NULL && iom_isIMath(fp)) ostring = strdup("IMATH");
		if (ostring == NULL && iom_isENVI(fp)) ostring = strdup("ENVI");
        if (ostring == NULL) {
            sprintf(error_buf, "Unable to determine file type: %s", filename);
            parse_error(NULL);
        }
        fclose(fp);

        if (ostring != NULL) {
            s = newVar();
            V_TYPE(s) = ID_STRING;
            V_STRING(s) = ostring;
        }
    }

    if (fname) free(fname);

    return (s);
}
