#include "parser.h"
#include "dvio_specpr.h"
#include "iomedley.h"

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
	char *filename= NULL, *fname, *element= NULL;
	int frec = 0;
	FILE *fp;
	char *p;

	Alist alist[4];
	alist[0] = make_alist( "filename",  ID_STRING,    NULL,    &filename);
	alist[1] = make_alist( "element",   ID_STRING,    NULL,    &element);
	alist[2] = make_alist( "record",    INT,          NULL,    &frec);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);
	if (filename == NULL) {
		parse_error("%s: No filename specified", func->name);
		return(NULL);
	}
	if (element == NULL) {
		parse_error("%s: No element specified", func->name);
		return(NULL);
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
    if (fname && (fp = fopen(fname, "rb")) != NULL) {

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
