#include "parser.h"
#include "dvio_specpr.h"

/**
 ** header() - extract information from a files header.
 **
 ** This routine will determine a file's type, and pass everything to the
 ** appropriate header searching routine.
 **
 **/

Var *
ff_header(vfuncptr func, Var *arg)
{
	Var *input = NULL, *v, *e;
	char *filename, *fname, *element;
	int frec;
	FILE *fp;
	char *p;

    struct keywords kw[] = {
		{"filename", NULL},
		{"element", NULL},
		{"record", NULL},
		{NULL, NULL}
	};

    if (evaluate_keywords(func, arg, kw)) {
        return (NULL);
    }
    /**
     ** get filename keyword.  Verify type
     **/
	if ((v = RequireKeyword("filename", kw, ID_STRING, 0, func)) == NULL) 
		return(NULL);
    filename = V_STRING(v);

	if ((v = RequireKeyword("element", kw, ID_STRING, 0, func)) == NULL) 
		return(NULL);
    element = V_STRING(v);

    /** 
     ** If a record was specified as a keyword, get it.
	 ** By the way, this looks at strings, to allow $N shell variables.
     **/
    if ((v = get_kw("record", kw)) != NULL) {
        if (V_TYPE(v) != ID_VAL) {
            e = eval(v);
            if (e == NULL) {
                parse_error("Illegal argument to load(...record=...)");
            }
            v = e;
        }
        if (V_TYPE(v) == ID_STRING) {
            frec = atoi(V_STRING(v));
        } else {
            if (V_TYPE(v) != ID_VAL || V_FORMAT(v) != INT || V_DSIZE(v) != 1) {
                parse_error("Illegal argument to load(...record=...)");
                return (NULL);
            }
            frec = V_INT(v);
        }
    }
    /** 
     ** if open file fails, check for record suffix
     **/
    if ((fname = dv_locate_file(filename)) == NULL) {
        if ((p = strchr(filename, SPECPR_SUFFIX)) != NULL) {
            *p = '\0';
            frec = atoi(p + 1);
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

		while(1) {
			if (LoadSpecprHeader(fp, filename, frec, element, &input)) break;
			if (dv_LoadVicarHeader(fp, filename, frec, element, &input)) break;
			if (dv_LoadISISHeader(fp, filename, frec, element, &input)) break;
			sprintf(error_buf, "Unable to determine file type: %s", filename);
			parse_error(NULL);
			break;
		}
        fclose(fp);
    }
    if (fname)
        free(fname);
    return (input);
}
