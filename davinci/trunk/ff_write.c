#include "parser.h"

Var *
ff_write(vfuncptr func, Var *arg)
{
    Var *v, *e, *ob;
    char *filename;
    char *title;
    char *type=NULL;
	int force=0;

    struct keywords kw[] = {
        { "object", NULL },
        { "filename", NULL },
        { "type", NULL },
        { "title", NULL },
		{ "force", NULL},
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    /**
     ** Make sure user specified an object
     **/
    if ((v = get_kw("object", kw)) == NULL) {
        parse_error("write what?  Specify data object");
        return(NULL);
    }

    e = eval(v);
    if (e == NULL) {
        sprintf(error_buf, "Variable not found: %s", V_NAME(v));
        parse_error(NULL);
        return(NULL);
    }
    ob = e;

    /**
     ** get filename keyword.  Verify type
     **/

    if ((v = get_kw("filename", kw)) == NULL) {
        parse_error("No filename specified.");
        return(NULL);
    }

    if (V_TYPE(v) != ID_STRING) {
        e = eval(v);
        if (e == NULL || V_TYPE(e) != ID_STRING) {
            parse_error("Illegal argument (... filename=... )");
            return(NULL);
        }
        v = e;
    }

    filename = V_STRING(v);

    /**
     ** Figure out what output type.
     **/

    if ((v = get_kw("type", kw)) == NULL) {
        parse_error("No type specified.");
        return(NULL);
    }

    /**
     ** check for variable dereference
     **/
    e = eval(v);
    if (e != NULL) {
        v = e;
    }
    if (V_TYPE(v) == ID_STRING) {
        type = V_STRING(v);
    }  else {
        type = V_NAME(v);
    }

	if ((v = get_kw("force", kw)) != NULL) force=1;

	if ((access(filename, F_OK) == 0) && strcasecmp(type, "specpr") && !force) {
		fprintf(stderr, "File exists.\n");
		return(NULL);
	}
	/**
	 ** Get title string
	 **/
	if ((v = get_kw("title", kw)) == NULL) {
		title = "DV data product";
	} else {
		if (V_TYPE(v) != ID_STRING) {
			e = eval(v);
			if (e == NULL || V_TYPE(e) != ID_STRING) {
				parse_error("Illegal argument (... title=... )");
				return(NULL);
			}
			v = e;
		}
		title = V_STRING(v);
	}

    if (!strcasecmp(type, "vicar")) WriteVicar(ob, NULL, filename); 
    else if (!strcasecmp(type, "grd"))   WriteGRD(ob, NULL, filename);
    else if (!strcasecmp(type, "pgm"))   WritePGM(ob, NULL, filename);
    else if (!strcasecmp(type, "ppm"))   WritePPM(ob, NULL, filename);
    else if (!strcasecmp(type, "ascii"))  WriteAscii(ob, NULL, filename);
    else if (!strcasecmp(type, "ers"))  WriteERS(ob, NULL, filename);
    else if (!strcasecmp(type, "imath"))  WriteIMath(ob, NULL, filename);
    else if (!strcasecmp(type, "isis"))  {
		WriteISIS(ob, NULL, filename, title);
    } else if (!strcasecmp(type, "specpr")) {
        WriteSpecpr(ob, filename, title);
    } else {
        sprintf(error_buf, "Unrecognized type: %s", type);
        parse_error(NULL);
        return(NULL);
    }
    return(NULL);
}
