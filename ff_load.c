#include "parser.h"
#include "io_specpr.h"

#ifndef __MSDOS__
#include <pwd.h>
#endif

#ifdef LITTLE_E
extern Swap_Big_and_Little(Var *);
#endif

void
init_iheader(struct _iheader *h)
{
	int i;
	memset(h, '\0', sizeof(*h));
	for (i = 0 ; i < 3 ; i++) {
		h->s_lo[i] = -1;
		h->s_hi[i] = -1;
		h->s_skip[i] = -1;
		h->prefix[i] = -1;
		h->suffix[i] = -1;
	}
}


/**
 ** locate_file() - check path for filename.
 **/

char *
locate_file(char *fname)
{
    char *p, *q;
    char *path = NULL;
    FILE *fp = NULL;
    char buf[1024];
    Var *v;

    strcpy(buf, fname);
    fname = expand_filename(buf);

    if (fname != NULL && (fp = fopen(fname, "r")) == NULL) {
        /**
         ** Get data path
         **/
        v = get_sym("$datapath");
        if (v != NULL) {
            path = strdup(V_STRING(v)); /* dupd, so we can cut on it */
        }
        for (p = path; fp == NULL && p != NULL; p = q) {
            if ((q = strchr(p, PATH_SEP)) != NULL) {
                while (*q && *q == PATH_SEP)
                    *q++ = '\0';
            }
            sprintf(buf, "%s/%s", p, fname);
            fp = fopen(buf, "r");
        }
    }
    if (path)
        free(path);
    if (fp != NULL) {
        fclose(fp);
        return (strdup(buf));
    }
    return (NULL);
}

Var *
ff_raw(vfuncptr func, Var * arg)
{
	unsigned char *buf;
	char *filename=NULL;
	int Row=0;
	int Col=0;
	int size;
	FILE *fp;
	int ac;
	Var **av;
	Alist alist[4];
	alist[0] = make_alist( "filename",  ID_STRING,  NULL,     &filename);
	alist[1] = make_alist( "row", INT,NULL,&Row);
	alist[2] = make_alist( "col", INT,NULL,&Col);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

        if (filename == NULL) {
        	parse_error("No filename specified to load()");
        	return (NULL);
        }

	if ((fp=fopen(filename,"r"))==NULL){
		parse_error("Can't find file: %s",filename);
		return(NULL);
	}

	if (!(Row) || !(Col)) {
		parse_error("Must specify row and column size!");
		return(NULL);
	}

	buf=(unsigned char *)calloc((Row*Col),sizeof(char));

	size=fread(buf,sizeof(char),(Row*Col),fp);
	if (size < (Row*Col)){
		parse_error("Incorrect Row/Col size, sorry...aborting");
		return(NULL);
	}

	return(newVal(BSQ,Col,Row,1,BYTE,buf));
}





Var *
ff_load(vfuncptr func, Var * arg)
{
    int record = -1;
    FILE *fp = NULL;
    Var  *input = NULL;
    char *filename = NULL;
    char *p, *fname;
	struct _iheader h;


	int ac;
	Var **av;
	Alist alist[12];
	alist[0] = make_alist( "filename",  ID_STRING,  NULL,     &filename);
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

	init_iheader(&h);

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (filename == NULL) {
        parse_error("No filename specified to load()");
        return (NULL);
	}

    /** 
     ** if open file fails, check for record suffix
     **/
    if ((fname = locate_file(filename)) == NULL) {
        if ((p = strchr(filename, SPECPR_SUFFIX)) != NULL) {
            *p = '\0';
            record = atoi(p + 1);
            fname = locate_file(filename);
        }
        if (fname == NULL) {
            sprintf(error_buf, "Cannot find file: %s", filename);
            parse_error(NULL);
            return (NULL);
        }
    }

	if (record != -1) h.s_lo[2] = h.s_hi[2] = record;

    if (fname && (fp = fopen(fname, "rb")) != NULL) {
        if (is_compressed(fp))
            fp = uncompress(fp, fname);

        if (input == NULL)    input = LoadSpecpr(fp, filename, record);
        if (input == NULL)    input = LoadVicar(fp, filename, &h);
        if (input == NULL)    input = LoadISIS(fp, filename, &h);
        if (input == NULL)    input = LoadGRD(fp, filename, &h);
        if (input == NULL)    input = LoadPNM(fp, filename, &h);
        if (input == NULL)    input = Load_imath(fp, filename, &h);
        if (input == NULL)    input = LoadGOES(fp, filename, &h);
        if (input == NULL)    input = LoadAVIRIS(fp, filename, &h);

        fclose(fp);	/* These others open their own files */
		
#ifdef HAVE_LIBHDF5
	  	  if (input == NULL)    input = LoadHDF5(filename);
#endif

#ifdef LITTLE_E
        if (input!=NULL)
                return((Var *)Swap_Big_and_Little(input));
#endif

#ifdef HAVE_LIBMAGICK
		if (input == NULL)    input = LoadGFX_Image(filename);
#endif

        if (input == NULL) {
            sprintf(error_buf, "Unable to determine file type: %s", filename);
            parse_error(NULL);
        }
    }
    if (fname)
        free(fname);
    return (input);
}

/**
 ** read_qube_data() - generalized cube input routines
 **/

void *
read_qube_data(int fd, struct _iheader *h)
{
    void *data;
    void *p_data;
    int dsize;
    int i, x, y, z;
    int nbytes;
    int dim[3];                 /* dimension of output data */
    int d[3];                   /* total dimension of file */
    int plane;
    int count;
    int err;

    /**
     ** data name definitions:
     **
     ** size: size of cube in file
     ** dim:  size of output cube
     ** d:    size of physical file (includes prefix and suffix values)
     **/

    /**
     ** WARNING!!!
     **  if (prefix+suffix)%nbytes != 0, bad things could happen.
     **/

    /**
     ** Touch up some default values
     **/
    nbytes = NBYTES(h->format);
    dsize = 1;
    for (i = 0; i < 3; i++) {
        if (h->size[i] == 0) h->size[i] = 1;
        if (h->s_lo[i] == 0) h->s_lo[i] = 1;
        if (h->s_hi[i] == 0) h->s_hi[i] = h->size[i];
        if (h->s_skip[i] == 0) h->s_skip[i] = 1;

        h->s_lo[i]--;           /* value is 1-N.  Switch to 0-(N-1) */
        h->s_hi[i]--;

        dim[i] = h->s_hi[i] - h->s_lo[i] + 1;
        h->dim[i] = dim[i];

        d[i] = h->size[i] * nbytes + h->suffix[i] + h->prefix[i];
        if (i && (h->suffix[i] + h->prefix[i]) % nbytes != 0) {
            fprintf(stderr, "Warning!  Prefix+suffix not divisible by pixel size\n");
        }
        dsize *= (dim[i] - 1) / h->s_skip[i] + 1;
    }
    if (h->gain == 0.0)
        h->gain = 1.0;

    /**
     ** compute output size, allocate memory.
     **/

    if ((data = malloc(dsize * nbytes)) == NULL) {
        fprintf(stderr, "Unable to allocate memory.\n");
        return (NULL);
    }
    /**
     ** Allocate some temporary storage space
     **/

    plane = d[0] * dim[1];      /* line * N-samples */

    if ((p_data = malloc(plane)) == NULL) {
        free(data);
        return (NULL);
    }
    /**
     ** loop, doesn't do skips yet.
     **/

    count = 0;
    for (z = 0; z < dim[2]; z += h->s_skip[2]) {

        /**
         ** read an entire plane 
         **/

        /*........label.....plane.....offset............. */

        lseek(fd, h->dptr +
              (z + h->s_lo[2]) * 
		  		(d[0] * h->size[1] + 
				 h->size[0] * (h->prefix[1]+h->suffix[1]) + 
				 h->corner) + 
			  d[0] * h->s_lo[1], 0);

        if ((err = read(fd, p_data, plane)) != plane) {
            parse_error("Early EOF");
            break;
        }
        for (y = 0; y < dim[1]; y += h->s_skip[1]) {

            /**
             ** find the line we are interested in 
             **/

            if (h->s_skip[0] == 1) {
                memcpy((char *) data + count,
                       (char *) p_data + (h->prefix[0] + h->s_lo[0] * nbytes + y * d[0]),
                       dim[0] * nbytes);
                count += dim[0] * nbytes;
            } else {
                for (x = 0; x < dim[0]; x += h->s_skip[0]) {
                    memcpy((char *) data + count,
                           (char *) p_data + (h->prefix[0] + (h->s_lo[0] + x) * nbytes + y * d[0]),
                           nbytes);
                    count += nbytes;
                }
            }
        }
        if (VERBOSE > 1)
            fprintf(stderr, ".");
    }

    /**
     ** do byte swap,
     ** find data limits, 
     ** apply multiplier.
     **/

    /**
    ** convert VAX_FLOAT if necessary.
    **/
    if (h->format == VAX_FLOAT) {
        int i;
        for (i = 0 ; i < dsize ; i++) {
            vax_ieee_r(&((float *)data)[i],&((float *)data)[i]);
        }
    } else if (h->format == VAX_INTEGER) {
        /**
        ** byte swap 'em.
        **/
        swab((const char *)data, (char *)data, dsize * NBYTES(h->format));
    }
    /**
     **/

    free(p_data);
    return (data);
}

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

    if ((fname = locate_file(filename)) == NULL) {
        if ((p = strchr(filename, SPECPR_SUFFIX)) != NULL) {
            *p = '\0';
            fname = locate_file(filename);
        }
        if (fname == NULL) {
            sprintf(error_buf, "Cannot find file: %s", filename);
            parse_error(NULL);
            return (NULL);
        }
    }
    if (fname && (fp = fopen(fname, "r")) != NULL) {
        if (is_compressed(fp))
            fp = uncompress(fp, fname);

        if (ostring == NULL && is_specpr(fp)) ostring = strdup("SPECPR");
        if (ostring == NULL && is_Vicar(fp)) ostring = strdup("VICAR");
        if (ostring == NULL && is_ISIS(fp)) ostring = strdup("VICAR");
        if (ostring == NULL && is_GRD(fp)) ostring = strdup("GRD");
        if (ostring == NULL && is_AVIRIS(fp)) ostring = strdup("AVIRIS");
        if (ostring == NULL && is_PNM(fp, format) != NULL)
			ostring = strdup(format);
        if (ostring == NULL && is_imath(fp)) ostring = strdup("imath");
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
    if (fname)
        free(fname);
    return (s);
}

#define PACK_MAGIC     "\037\036"       /* Magic header for packed files */
#define GZIP_MAGIC     "\037\213"       /* Magic header for gzip files, 1F 8B */
#define OLD_GZIP_MAGIC "\037\236"       /* Magic header for gzip 0.5 = freeze 1.x */
#define LZH_MAGIC      "\037\240"       /* Magic header for SCO LZH Compress files */
#define LZW_MAGIC      "\037\235"       /* Magic header for lzw files, 1F 9D */

int
is_compressed(FILE * fp)
{
    char buf[8];

    fread(buf, 1, 2, fp);
    buf[2] = '\0';
    rewind(fp);

    return (!strcmp(buf, LZW_MAGIC) ||
            !strcmp(buf, GZIP_MAGIC) ||
            !strcmp(buf, LZH_MAGIC) ||
            !strcmp(buf, PACK_MAGIC) ||
            !strcmp(buf, OLD_GZIP_MAGIC));
}

FILE *
uncompress(FILE * fp, char *fname)
{
    /**
     ** Try gzip first, then compress
     **/
    char buf[256];
    FILE *pfp;
    char *tptr;

    tptr = tempnam(NULL, NULL);
    sprintf(buf, "gzip -d < %s > %s", fname, tptr);

    if (VERBOSE > 1)
        fprintf(stderr, "Uncompressing %s\n", fname);

    if ((pfp = popen(buf, "r")) == NULL) {
        sprintf(buf, "compress -d < %s > %s", fname, tptr);
        if ((pfp = popen(buf, "r")) == NULL) {
            free(tptr);
            return (NULL);
        }
    }
    /** this should wait for the pfp to finish. **/
    fgets(buf, 256, pfp);

    fclose(fp);
    fp = fopen(tptr, "r");

    unlink(tptr);
    free(tptr);

    return (fp);
}

/**
 ** Try to expand environment variables and ~
 ** puts answer back into argument.  Make sure its big enough...
 **/

char *
expand_filename(char *s)
{
    char buf[1024];
    char ebuf[256];
    char *p, *q, *e;
    struct passwd *pwent;

    buf[0] = '\0';

    p = s;
    while (p && *p) {
        if (*p == '$') {        /* environment variable expansion */
            q = p + 1;
            while (*q && (isalnum(*q) || *q == '_')) {
                q++;
            }
            strncpy(ebuf, p + 1, q - p - 1);
            if ((e = get_env_var(ebuf)) == NULL) {
                fprintf(stderr, "error: unknown environment variable: %s\n",
                        ebuf);
                return (NULL);
            } else {
                strcat(buf, e);
            }
            p = q;
	
        }
#ifndef __MSDOS__ 
	    else if (*p == '~' && p == s) { /* home directory expansion */
            q = p + 1;
            while (*q && (isalnum(*q) || *q == '_')) {
                q++;
            }
            if (q == p + 1) {   /* no username specified, use $HOME */
                strcat(buf, getenv("HOME"));
            } else {
                strncpy(ebuf, p + 1, q - p - 1);
				ebuf[q-p-1] = 0;
                if ((pwent = getpwnam(ebuf)) == NULL) {
                    fprintf(stderr, "error: unknown user: %s\n", ebuf);
                    return (NULL);
                } else {
                    strcat(buf, pwent->pw_dir);
                }
            }
            p = q;
        } 
#endif
	    else {
            strncat(buf, p, 1);
            p++;
        }
    }
    strcpy(s, buf);
    return (s);
}

char *
get_env_var(char *name)
{
    char *value = NULL;

    /**
     ** Shell command line variables still have the '$' in front of them.
     ** Environment variables shouldn't.
     **/
    if ((value = getenv(name)) == NULL) {
        sprintf(error_buf, "Environment variable not found: %s", name);
        parse_error(NULL);
        return (NULL);
    }
    return (value);
}

