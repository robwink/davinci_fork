#include "parser.h"

/**
 ** read a text file into BYTE data
 **/

Var *
ff_text(vfuncptr func, Var *arg)
{
    char    *filename;
    Var     *v, *e, *s;
    char    *fname;
    FILE *fp;
    char *ptr;
    int rlen;

    unsigned char *cdata;
    int count=0;

    int i,j,k;

    int dsize;
    int x=0;
    int y=0;

    struct keywords kw[] = {
        { "filename", NULL },   /* filename to read */
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    if ((v = get_kw("filename", kw)) == NULL) {
        sprintf(error_buf, "No filename specified: %s()", func->name);
        parse_error(NULL);
        return(NULL);
    }
    if (V_TYPE(v) != ID_STRING) {
        e = eval(v);
        if (e == NULL || V_TYPE(e) != ID_STRING) {
            sprintf(error_buf, "Illegal argument: %s(... filename=...)", 
                    func->name);
            parse_error(NULL);
            return(NULL);
        }
        v = e;
    }
    filename = V_STRING(v);

    if ((fname = locate_file(filename)) == NULL) {
        sprintf(error_buf, "Cannot find file: %s\n", filename);
        parse_error(NULL);
        return(NULL);
    }
    if ((fp = fopen(fname, "r")) == NULL) {
        sprintf(error_buf, "Cannot open file: %s\n", fname);
        parse_error(NULL);
        return(NULL);
    }
        /**
         ** Determine the file size.  X is max of all line lengths.
         **/

	x = y = 0;
	count = 0;
	while(getline(&ptr, fp) != EOF) {
		if ((int)strlen(ptr) > x) 
			x = (int)strlen(ptr);
		y++;
		count += (int)strlen(ptr);
	}
	rewind(fp);

    dsize = x*y;
    cdata = (unsigned char *)calloc(1, dsize);

	for (j = 0 ; j < y ; j++) {
		if ((rlen = getline(&ptr, fp)) == -1) break;
		memcpy(cdata+(x*j), ptr, strlen(ptr));
	}

    if (VERBOSE > 1) {
		fprintf(stderr, "Read TEXT file: %dx%d (%d bytes)\n", x,y,count);
    }

    s = new(Var);
    V_TYPE(s) = ID_VAL;
    V_DATA(s) = cdata;
    V_FORMAT(s) = BYTE;
    V_ORG(s) = BSQ;
    V_DSIZE(s) = dsize;
    V_SIZE(s)[0] = x;
    V_SIZE(s)[1] = y;
    V_SIZE(s)[2] = 1;

    return(s);
}

/**
 ** extract from BYTE using delim
 **/

Var *
ff_delim(vfuncptr func, Var *arg)
{
	Var *ob, *cval, *s, *v, *e;
	char *delim;
	char *cdata;
	char *ptr;
	int item, count;

    struct keywords kw[] = {
        { "object", NULL },   	/* object to read */
		{ "delim",    NULL },   /* string of delimters to use */
		{ "count",    NULL },   /* string of delimters to use */
        { NULL, NULL }
    };

    if (evaluate_keywords(func, arg, kw)) {
        return(NULL);
    }

    if ((v = get_kw("object",kw)) == NULL) {
        sprintf(error_buf, "No object specified: %s()\n", func->name);
        parse_error(NULL);
        return(NULL);
    }
    e = eval(v);
    if (e == NULL) {
        parse_error("Illegal argument to read_text(...object=...)");
        return(NULL);
    }
	if (V_TYPE(e) == ID_VAL) {
		if (V_FORMAT(e) != BYTE) {
			parse_error("Illegal argument to read_text(...object=...), must be BYTE");
			return(NULL);
		}
	} else if (V_TYPE(e) != ID_STRING) {
        parse_error("Illegal argument to read_text(...object=...), must be BYTE");
        return(NULL);
	}
    ob = e;
	
	if ((cval = RequireKeyword("count",  kw, ID_VAL, INT,  func)) == NULL) 
		return(NULL);
	item = V_INT(cval);

	if (item <= 0)  {
		sprintf(error_buf, "%s(), count must be greater than 0", func->name);
		parse_error(NULL);
		return(NULL);
	}

    if ((v = get_kw("delim", kw)) != NULL) {
		if ((e = eval(v)) != NULL) v = e;
		if (V_TYPE(v) != ID_STRING) {
			sprintf(error_buf, "%s(), delim must be a STRING", func->name);
		}
    }
	delim = V_STRING(v);

	if (V_TYPE(ob) == ID_VAL) {
		cdata = strdup((char *)V_DATA(ob));
	} else {
		cdata = strdup((char *)V_STRING(ob));
	}

	ptr = cdata;
	count = 0;
	while((ptr = strtok(ptr, delim)) != NULL) {
		count++;
		if (count == item) {
			s = new(Var);
			V_TYPE(s) = ID_STRING;
			V_STRING(s) = strdup(ptr);
			free(cdata);
			return(s);
		}
		ptr = NULL;
	}

	if (VERBOSE) {
		fprintf(stderr, "%s(), Unable to find delimiter %d\n", func->name, item);
	}
	return(NULL);
}
