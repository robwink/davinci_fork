#include "parser.h"
#include "iomedley.h"
#include "dvio.h"
#include "dvio_specpr.h"


Var *
ff_load_many(vfuncptr func, Var * arg)
{
	int i;
	char *filename;
	Var *s, *t;
	if (arg == NULL || V_TYPE(arg) != ID_TEXT) {
        parse_error("No filenames specified to %s()", func->name);
        return (NULL);
	}

	s = new_struct(V_TEXT(arg).Row);
	for (i = 0 ; i < V_TEXT(arg).Row ; i++) {
		filename = strdup(V_TEXT(arg).text[i]);
		t = ff_load(func, newString(filename));
		if (t) add_struct(s, filename, t);
	}
	if (get_struct_count(s)) {
		return(s);
	}  else {
		return(NULL);
	}
}


Var *
ff_load(vfuncptr func, Var * arg)
{
    int record = -1;
    FILE *fp = NULL;
    Var  *input = NULL;
    char *filename = NULL;
    char *p, *fname;
	struct iom_iheader h;
	Var *fvar = NULL;

	Alist alist[12];
	alist[0] = make_alist( "filename",  ID_UNK,  NULL,     &fvar);
	alist[1] = make_alist( "record",    INT,    	NULL,     &record);
	alist[2] = make_alist( "xlow",      INT,    	NULL,     &h.s_lo[0]);
	alist[3] = make_alist( "xhigh",     INT,    	NULL,     &h.s_hi[0]);
	alist[4] = make_alist( "xskip",     INT,    	NULL,     &h.s_skip[0]);
	alist[5] = make_alist( "ylow",      INT,    	NULL,     &h.s_lo[1]);
	alist[6] = make_alist( "yhigh",     INT,    	NULL,     &h.s_hi[1]);
	alist[7] = make_alist( "yskip",     INT,    	NULL,     &h.s_skip[1]);
	alist[8] = make_alist( "zlow",      INT,    	NULL,     &h.s_lo[2]);
	alist[9] = make_alist( "zhigh",     INT,    	NULL,     &h.s_hi[2]);
	alist[10] = make_alist( "zskip",     INT,    	NULL,     &h.s_skip[2]);
	alist[11].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (fvar == NULL) {
        parse_error("No filename specified to load()");
        return (NULL);
	}
	if (V_TYPE(fvar) == ID_TEXT) {
		return(ff_load_many(func, fvar));
	} else if (V_TYPE(fvar) == ID_STRING) {
		filename = V_STRING(fvar);
	} else {
        parse_error("Illegal argument to function %s(%s), expected STRING", 
			func->name, "filename");
        return (NULL);
	}

    /** 
     ** if open file fails, check for record suffix
     **/
    if ((fname = dv_locate_file(filename)) == NULL) {
        if ((p = strchr(filename, SPECPR_SUFFIX)) != NULL) {
            *p = '\0';
            record = atoi(p + 1);
            fname = dv_locate_file(filename);
        }
        if (fname == NULL) {
            sprintf(error_buf, "Cannot find file: %s", filename);
            parse_error(NULL);
            return (NULL);
        }
    }

	iom_init_iheader(&h);
	if (record != -1) h.s_lo[2] = h.s_hi[2] = record;

    if (fname && (fp = fopen(fname, "rb")) != NULL) {
        if (iom_is_compressed(fp))
            fp = iom_uncompress(fp, fname);

        if (input == NULL)    input = LoadSpecpr(fp, fname, record);
        if (input == NULL)    input = dv_LoadVicar(fp, fname, &h);
        if (input == NULL)    input = dv_LoadISIS(fp, fname, &h);
        if (input == NULL)    input = dv_LoadGRD(fp, fname, &h);
        if (input == NULL)    input = dv_LoadPNM(fp, fname, &h);
        if (input == NULL)    input = dv_LoadIMath(fp, fname, &h);
        if (input == NULL)    input = dv_LoadGOES(fp, fname, &h);
        if (input == NULL)    input = dv_LoadAVIRIS(fp, fname, &h);

#ifdef HAVE_LIBHDF5
        if (input == NULL)    input = LoadHDF5(fname);
#endif

		/* Libmagic should always be the last chance */
#ifdef HAVE_LIBMAGICK
        if (input == NULL)
	         input = dvReadImage(func, arg); // last gasp attempt.
#endif /* HAVE_LIBMAGICK */

        fclose(fp);
		

        if (input == NULL) {
            sprintf(error_buf, "Unable to determine file type: %s", filename);
            parse_error(NULL);
        }
    }
    if (fname)
        free(fname);
    
    return (input);
}
