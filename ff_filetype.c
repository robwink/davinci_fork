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
    char *filename, *fname;
    FILE *fp;
    char *p;
    char *ostring = NULL;
	char format[256];

    struct keywords kw[] =
            {
                {"filename", NULL},
                {NULL, NULL}
            };

    if (evaluate_keywords(func, arg, kw)) {
        return (NULL);
    }
    if ((v = get_kw("filename", kw)) == NULL) {
        sprintf(error_buf, "No filename specified: %s()", func->name);
        parse_error(NULL);
        return (NULL);
    }
    if (V_TYPE(v) != ID_STRING) {
        e = eval(v);
        if (e == NULL || V_TYPE(e) != ID_STRING) {
            sprintf(error_buf, "Illegal argument: %s(... filename=...)",
                    func->name);
            parse_error(NULL);
            return (NULL);
        }
        v = e;
    }
    filename = V_STRING(v);

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
    if (fname && (fp = fopen(fname, "r")) != NULL) {
        if (iom_is_compressed(fp))
            fp = iom_uncompress(fp, fname);

        if (ostring == NULL && is_specpr(fp)) ostring = strdup("SPECPR");
        if (ostring == NULL && iom_isVicar(fp)) ostring = strdup("VICAR");
        if (ostring == NULL && iom_isISIS(fp)) ostring = strdup("VICAR");
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
