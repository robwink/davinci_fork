#include "parser.h"

int 
getline(char **ptr, FILE *fp)
{
    static char *line=NULL;
    static int len=0;

    if (fp == NULL && line != NULL) {
        free(line);
        line = NULL;
        return 0;
    }

    if (line == NULL) {
        len = 8192;
        line = (char *)malloc(len+1);
    }

    if (fgets(line, len, fp) == NULL) {
        *ptr = NULL;
        return(-1);
    }
    while (strchr(line, '\n') == NULL) {
        line = (char *)my_realloc(line, len*2+1);
        if ((fgets(line+len-1, len, fp)) == NULL) break;
        len = len*2-1;
        if (len > 1000000) {
			fprintf(stderr, "Line is at 1000000\n");
			exit(1);
		}
    }
    *ptr = line;
    return(strlen(line));
}

Var *
ff_ascii(vfuncptr func, Var *arg)
{
    char    *filename;
    Var         *v, *e, *s;
    char    *fname;
    FILE *fp;
    char *ptr;
    int rlen;

    void *data;
    unsigned char *cdata;
    short *sdata;
    int *idata;
    float *fdata;
    double *ddata;
    int count=0;
	char delim[256] = " \t";

    int i,j,k;

    int dsize;
    int x=0;
    int y=0;
    int z=0;
    int format=0;
    int column=0;
    int row=0;


    struct keywords kw[] = {
        { "filename", NULL },   /* filename to read */
        { "x",        NULL },   /* size in x */
        { "y",        NULL },   /* size in y */
        { "z",        NULL },   /* size in z */
        { "format",   NULL },   /* data format */
        { "column",   NULL },   /* start column (number to skip) */
        { "row",      NULL },   /* start row (number to skip) */
		{ "delim",    NULL },   /* string of delimters to use */
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

    if (KwToInt("x", kw, &x) == -1 ||
        KwToInt("y", kw, &y) == -1 ||
        KwToInt("z", kw, &z) == -1 ||
        KwToInt("column", kw, &column) == -1 ||
        KwToInt("row", kw, &row) == -1) 
        return(NULL);

    if ((v = get_kw("format", kw)) != NULL) {
        ptr = V_NAME(v);
        if (ptr == NULL) 
            ptr = V_STRING(v);
        while (1) {
            if (!strcasecmp(ptr, "byte")) format = BYTE;
            else if (!strcasecmp(ptr, "short")) format = SHORT;
            else if (!strcasecmp(ptr, "int")) format = INT;
            else if (!strcasecmp(ptr, "float")) format = FLOAT;
            else if (!strcasecmp(ptr, "double")) format = DOUBLE;
            else {
                if ((e = eval(v)) == NULL) {
                    sprintf(error_buf, "Unrecognized value for keyword: %s=%s", 
                            "format", ptr);
                    parse_error(NULL);
                    return(NULL);
                }
                ptr = V_STRING(e);
                v = NULL;
                continue;
            }
            break;
        }
    }

    if ((v = get_kw("delim", kw)) != NULL) {
		if ((e = eval(v)) != NULL) v = e;
		if (V_TYPE(v) != ID_STRING) {
			parse_error("Expected string for keyword 'delim'");
			return(NULL);
		}
		strcpy(delim, V_STRING(v));
	}

    /**
     ** Got all the values.  Do something with 'em.
     **/

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
    if (x == 0 && y == 0 && z == 0) {
        /**
         ** User wants us to determine the file size.
         **/
        z = 1;

        /* skip some rows */
        for (i = 0 ; i < row ; i ++) {
            getline(&ptr, fp);
            if (ptr == NULL) {
                fprintf(stderr, "Early EOF, aborting.\n");
                return(NULL);
            }
        }

        while(getline(&ptr, fp) != EOF) {
            if (ptr[0] == '\n') {
                z++;
                continue;
            }

            while ((ptr = strtok(ptr, delim)) != NULL)  {
                count++;
                ptr = NULL;
            }
            y++;
        }

        if (z == 0) z = 1;
        if (y == 0) y = 1;
        y = y/z;
        x = count/y/z;

        if (x*y*z != count) {
            fprintf(stderr, "Unable to determine file size.\n");
            return(NULL);
        }
        if (VERBOSE) fprintf(stderr, "Apparent file size: %dx%dx%d\n", x,y,z);
        rewind(fp);
    }

    /**
     ** Decode file with specified X, Y, Z.
     **/
    if (x == 0) x = 1;
    if (y == 0) y = 1;
    if (z == 0) z = 1;
    if (format == 0) format = INT;

    dsize = x*y*z;
    data = calloc(NBYTES(format), dsize);

    cdata = (unsigned char *)data;
    sdata = (short *)data;
    idata = (int *)data;
    fdata = (float *)data;
    ddata = (double *)data;

    count = 0;


    /**
     ** Skip N rows.
     **/
    for (i = 0 ; i < row ; i ++) {
        getline(&ptr, fp);
        if (ptr == NULL) {
            fprintf(stderr, "Early EOF, aborting.\n");
            return(NULL);
        }
    }
    for (k = 0 ; k < z ; k++) {
        if (k) {
            /**
             ** skip to end of block
             **/
            while (getline(&ptr, fp) > 1)
                ;       
        }

        for (j = 0 ; j < y ; j++) {
            if ((rlen = getline(&ptr, fp)) == -1) break;

            /**
             ** skip columns
             **/

            for (i = 0 ; i < column ; i++) {
                ptr = strtok(ptr, delim);
                ptr = NULL;
            }
            /**
             ** read X values from this line
             **/

            for (i = 0 ; i < x ; i++) {
                ptr = strtok(ptr, delim);
                if (ptr == NULL) {
                    fprintf(stderr, "Line too short\n");
                    count += x-i;
                    break;
                }
                switch (format) {
                  case BYTE:
                    cdata[count++] = saturate_byte(strtol(ptr, NULL, 10));
                    break;
                  case SHORT:
                    sdata[count++] = saturate_short(strtol(ptr, NULL, 10));
                    break;
                  case INT:
                    idata[count++] = saturate_int(strtol(ptr, NULL, 10));
                    break;
                  case FLOAT:
                    fdata[count++] = strtod(ptr, NULL);
                    break;
                  case DOUBLE:
                    ddata[count++] = strtod(ptr, NULL);
                    break;
                }
                ptr = NULL;
            }
        }
        if (rlen == -1) {
            fprintf(stderr, "Early EOF\n");
            break;
        }
    }

    if (VERBOSE > 1) {
        fprintf(stderr, "Read ASCII file: %dx%dx%d\n", x,y,z);
    }

    s = newVar();
    V_TYPE(s) = ID_VAL;
    V_DATA(s) = data;
    V_FORMAT(s) = format;
    V_ORG(s) = BSQ;
    V_DSIZE(s) = dsize;
    V_SIZE(s)[0] = x;
    V_SIZE(s)[1] = y;
    V_SIZE(s)[2] = z;

    return(s);
}
