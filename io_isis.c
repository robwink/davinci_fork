#include "parser.h"
#include "io_lablib3.h"

/**
 **/

/****Addition******  Modified 9/21/99****/
typedef struct _sheader
{
	int	s_bytes;
	int	*s_item_type;
	int	s_quantity;
	int	s_plane_number;
	int	*s_item_byte;
}sheader;


int GetISISHeader(FILE *, char *filename, struct _iheader *, OBJDESC **ob);
void vax_ieee_r(float *from, float *to);

int
is_ISIS(FILE *fp)
{
    char        buf[256];

    rewind(fp);

    if (fgets(buf, 256, fp) == NULL) {
        return(0);
    }
    if (strncmp(buf, "CCSD", 4) && strncmp(buf, "NJPL", 4)) {
        return(0);
    }

    /**
    ** Best guess is yes, it is.
    **/
    return(1);
}


int
GetISISHeader(FILE *fp, char *filename, struct _iheader *h, OBJDESC **r_obj)
{
    OBJDESC * ob, *qube, *image;
    KEYWORD * key;
    int rlen;
    char        buf[256];
    int scope = ODL_THIS_OBJECT;
    char        *ptr, p1[16], p2[16],p3[16];
    int org, format, size[3];
    int suffix[3], suffix_size[3];
    int i;
    float       offset, gain;
    unsigned short start_type;
    unsigned long start;
    int suffix_bytes = 0;
    char *err_file = NULL;

    suffix[0] = suffix[1] = suffix[2] = 0;
    suffix_size[0] = suffix_size[1] = suffix_size[2] = 0;
    /**
    ** Parse the label
    **/

    rewind(fp);
    memset(h, '\0', sizeof(struct _iheader));

    if (VERBOSE > 0) err_file = NULL;
    else err_file = "/dev/null";

    ob = (OBJDESC *)OdlParseLabelFptr(fp, err_file,
                                      ODL_EXPAND_STRUCTURE, 0);

    if (!ob || (key = OdlFindKwd(ob, "RECORD_BYTES", NULL, 0, 0)) == NULL) {
        OdlFreeTree(ob);
        parse_error("Not a PDS file.");
        return(0);
    } else {
        rlen = atoi(key->value);
    }

    /**
    ** Check if this is an ISIS QUBE first.  If not, check for an IMAGE
    **/

    if ((qube = OdlFindObjDesc(ob, "QUBE", NULL, 0, 0, 0)) != NULL) {

        /**
        ** Get data organization
        **/

        org = -1;
        if ((key = OdlFindKwd(qube, "AXES_NAME", NULL, 0, scope)) ||
            (key = OdlFindKwd(qube, "AXIS_NAME", NULL, 0, scope))) {
            ptr = key->value;
            sscanf(ptr, " ( %[^,] , %[^,] , %[^)] ) ", p1, p2, p3);
            if (!strcmp(p1,"SAMPLE") && !strcmp(p2,"LINE") && !strcmp(p3,"BAND")) 
                org = BSQ;
            else if (!strcmp(p1,"SAMPLE") && !strcmp(p2,"BAND") && !strcmp(p3,"LINE")) 
                org = BIL;
            else if (!strcmp(p1,"BAND") && !strcmp(p2,"SAMPLE") && !strcmp(p3,"LINE")) 
                org = BIP;
            else {
                parse_error("Unrecognized data organization: %s = %s",
                        "AXIS_NAME", key->value);
            }
        }

        /**
        ** Size of data
        **/

        if ((key = OdlFindKwd(qube, "CORE_ITEMS", NULL, 0, scope))) {
            sscanf(key->value, "(%d,%d,%d)", &size[0], &size[1], &size[2]);
        }

        /**
        ** Format
        **/

        if ((key = OdlFindKwd(qube, "CORE_ITEM_BYTES", NULL, 0, scope))) {
            switch (atoi(key->value)) {
            case 1:           
                format = BYTE; 
                break;
            case 2:           
                format = SHORT; 
                break;
            case 4:           
                format = INT; 
                break;
            default:
                parse_error( "Unrecognized data format: %s = %s",
                        "CORE_ITEM_BYTES", key->value);
                break;
            }
        }

        /**
        ** This tells us if we happen to be using float vs int
        **/

        if ((key = OdlFindKwd(qube, "CORE_ITEM_TYPE", NULL, 0, scope))) {
            if (format == INT && !strcmp(key->value, "REAL")) {
                format = FLOAT;
            }
            if (format == INT && !strcmp(key->value, "SUN_REAL")) {
                format = FLOAT;
            }
            if (format == INT && !strcmp(key->value, "VAX_REAL")) {
                format = VAX_FLOAT;
            }
        }

        if ((key = OdlFindKwd(qube, "CORE_BASE", NULL, 0, scope))) {
            offset = atof(key->value);
        }

        if ((key = OdlFindKwd(qube, "CORE_MULTIPLIER", NULL, 0, scope))) {
            gain = atof(key->value);
        }


        if ((key = OdlFindKwd(qube, "SUFFIX_ITEMS", NULL, 0, scope))) {
            sscanf(key->value, "(%d,%d,%d)", &suffix[0], &suffix[1], &suffix[2]);
        }
        if ((key = OdlFindKwd(qube, "SUFFIX_BYTES", NULL, 0, scope))) {
            suffix_bytes = atoi(key->value);
        }

#if 0
        /**
        ** In theory, we should be able to simply multiply the suffix_items
        ** times suffix_bytes, to get a suffix_size, but its not that cut and
        ** dry.  Instead, for each suffix_item, we need to loop through 
        ** its *_suffix_bytes, and add them all together, to get a total 
        ** number of bytes to skip.
        **/

        if (suffix[0] != 0) {
            switch (org) {
            case BIL: 
                strcpy(name, "SAMPLE_SUFFIX_ITEM_BYTES"); 
                break;
            case BIP:   
                strcpy(name, "BAND_SUFFIX_ITEM_BYTES"); 
                break;
            case BSQ:   
                strcpy(name, "SAMPLE_SUFFIX_ITEM_BYTES"); 
                break;
            }
            if ((key = OdlFindKwd(qube, name, NULL, 0, scope))) {
                suffix_size[0] = 0;
                if ((ptr = strchr(key->value, '(')) != NULL) {
                    for (i = 0 ; i < suffix[0] ; i++) {
                        suffix_size[0] += strtod(ptr + 1, &ptr);
                    }
                } else {
                    suffix_size[0] = strtod(key->value, NULL);
                }
            }
        }
        if (suffix[1] != 0) {
            switch (org) {
            case BIL: 
                strcpy(name, "BAND_SUFFIX_ITEM_BYTES"); 
                break;
            case BIP:   
                strcpy(name, "SAMPLE_SUFFIX_ITEM_BYTES"); 
                break;
            case BSQ:   
                strcpy(name, "LINE_SUFFIX_ITEM_BYTES"); 
                break;
            }
            if ((key = OdlFindKwd(qube, name, NULL, 0, scope))) {
                suffix_size[1] = 0;
                if ((ptr = strchr(key->value, '(')) != NULL) {
                    for (i = 0 ; i < suffix[1] ; i++) {
                        suffix_size[1] += strtod(ptr + 1, &ptr);
                    }
                } else {
                    suffix_size[1] = strtod(key->value, NULL);
                }
            }
        }
        if (suffix[2] != 0) {
            switch (org) {
            case BIL: 
                strcpy(name, "LINE_SUFFIX_ITEM_BYTES"); 
                break;
            case BIP:   
                strcpy(name, "LINE_SUFFIX_ITEM_BYTES"); 
                break;
            case BSQ:   
                strcpy(name, "BAND_SUFFIX_ITEM_BYTES"); 
                break;
            }
            if ((key = OdlFindKwd(qube, name, NULL, 0, scope))) {
                suffix_size[2] = 0;
                if ((ptr = strchr(key->value, '(')) != NULL) {
                    for (i = 0 ; i < suffix[0] ; i++) {
                        suffix_size[2] += strtod(ptr + 1, &ptr);
                    }
                } else {
                    suffix_size[2] = strtod(key->value, NULL);
                }
            }
        }
#endif

        for (i = 0 ; i < 3 ; i++) {
            if (suffix[i]) {
                suffix_size[i] = suffix[i] * suffix_bytes;
            }
        }
        if (VERBOSE > 2) { 
            fprintf(stderr, "ISIS Qube: ");
            fprintf(stderr, "%dx%dx%d %s %s\t\n",
                    GetSamples(size, org),
                    GetLines(size, org),
                    GetBands(size, org),
                    Org2Str(org), Format2Str(format));

            for (i = 0 ; i < 3 ; i++) {
                if (suffix[i]) {
                    fprintf(stderr, "Suffix %d present, %d bytes\n", 
                            i, suffix_size[i]);
                }
            }
        }

        /**
        ** Cram everything into the appropriate header, and
        ** make sure we have the right data file.
        **/

        h->org = org;           /* data organization */
        h->size[0] = size[0];
        h->size[1] = size[1];
        h->size[2] = size[2];
        h->format = format;
        h->offset = offset;
        h->gain = gain;
        h->prefix[0] = h->prefix[1] = h->prefix[2] = 0;
        h->suffix[0] = suffix_size[0];
        h->suffix[1] = suffix_size[1];
        h->suffix[2] = suffix_size[2];
		h->corner = suffix[0]*suffix[1]*suffix_bytes;

        if ((key = OdlFindKwd(ob, "^QUBE", NULL, 0, 0)) != NULL) {
            OdlGetFileName(key, &start, &start_type);
            if (start_type == ODL_RECORD_LOCATION) {
                start = (start-1)*rlen;
            }
            h->dptr = start;
        }
        if (r_obj != NULL) 
            *r_obj = ob;
        else 
            OdlFreeTree(ob);
        return(1);
    } else {
        if ((image = OdlFindObjDesc(ob, "IMAGE", NULL, 0, 0, 0)) != NULL) {
            
            if ((key = OdlFindKwd(image, "LINES", NULL, 0, scope))) {
                size[1] = atoi(key->value);
            }
            if ((key = OdlFindKwd(image, "LINE_SAMPLES", NULL, 0, scope))) {
                size[0] = atoi(key->value);
            }

            if ((key = OdlFindKwd(image, "SAMPLE_BITS", NULL, 0, scope))) {
                if (atoi(key->value) == 8) {
                    format = BYTE;
                } else if (atoi(key->value) == 16) {
                    format = SHORT;
                } else if (atoi(key->value) == 32) {
                    format = INT;
                }
            }

            if ((key = OdlFindKwd(image, "SAMPLE_TYPE", NULL, 0, scope))) {
                if (!strcmp(key->value, "VAX_INTEGER")) {
                    format = VAX_INTEGER;
                }
                if (!strcmp(key->value, "VAX_REAL")) {
                    format = VAX_FLOAT;
                }
                if (format == INT && !strcmp(key->value, "SUN_REAL")) {
                    format = FLOAT;
                }
                if (format == INT && !strcmp(key->value, "REAL")) {
                    format = FLOAT;
                }
            }


            /**
            ** Load important IMAGE values
            **/
            if ((key = OdlFindKwd(ob, "^IMAGE", NULL, 0, 0)) != NULL) {
                OdlGetFileName(key, &start, &start_type);
                if (start_type == ODL_RECORD_LOCATION) {
                    start = (start-1)*rlen;
                }
                h->dptr = start;
            }
            if (r_obj != NULL) {
                *r_obj = ob;
            } else {
                OdlFreeTree(ob);
            }

            h->org = BSQ;           /* data organization */
            h->size[0] = size[0];
            h->size[1] = size[1];
            h->size[2] = 0;
            h->format = format;
            h->offset = 0;
            h->gain = 0;
            h->prefix[0] = h->prefix[1] = h->prefix[2] = 0;
            h->suffix[0] = h->suffix[1] = h->suffix[2] = 0;
            return(1);
        }
    }
    return(0);
}

Var *
LoadISIS(FILE *fp, char *filename, struct _iheader *s)
{
    struct _iheader h;
    int org;
    Var *v;
    char hbuf[HBUFSIZE];
    void *data;
    int vax = 0;
	int i,j;

    if (is_ISIS(fp) == 0) return(NULL);

    if (GetISISHeader(fp, filename, &h, NULL) == 0) {
        return(NULL);
    }

    /**
     ** If user specified a record, subset out a specific band
     **/

    if (s != NULL) {
        /** 
        ** Set subsets
        **/
        org = h.org;

		for (i = 0 ; i < 3 ; i++) {
			j = orders[org][i];
			if (s->s_lo[j] > 0) h.s_lo[i] = s->s_lo[j];
			if (s->s_hi[j] > 0) h.s_hi[i] = s->s_hi[j];
			if (s->s_skip[j] > 0) h.s_skip[i] = s->s_skip[j];
			if (s->prefix[j] > 0) h.prefix[i] = s->prefix[j];
			if (s->suffix[j] > 0) h.suffix[i] = s->suffix[j];
		}
    }


    /**
     ** Put all this into a Var struct.
     **/
    if ((data = read_qube_data(fileno(fp), &h)) == NULL) {
        return(NULL);
    }
    v = iheader2var(&h);
    V_DATA(v) = data;

    sprintf(hbuf, "%s: ISIS %s image: %dx%dx%d, %s (%d bits)",
            filename, Org2Str(h.org),
            GetSamples(h.size, h.org), 
            GetLines(h.size, h.org), 
            GetBands(h.size, h.org), 
            Format2Str(h.format), NBYTES(h.format)*8);

    if (VERBOSE > 1) { 
        fprintf(stderr, hbuf);
        fprintf(stderr, "\n");
        fflush(stderr);
    }
    V_TITLE(v) = strdup(hbuf);

    return(v);
}

int 
WriteISIS(Var *s, FILE *fp, char *filename, char *title )
{
    int dsize;
    int fsize;
    char buf[1025];

    dsize = V_DSIZE(s);
    fsize = dsize/512+1;
    if (V_FORMAT(s) > INT) {
    fprintf(stderr, "Unable to write ISIS files with %s format.\n",
        Format2Str(V_FORMAT(s)));
    return(0);
    }

    sprintf(buf, "CCSD3ZF0000100000001NJPL3IF0PDS200000001 = SFDU_LABEL\r\n");
    sprintf(buf+strlen(buf), "RECORD_TYPE = FIXED_LENGTH\r\n");
    sprintf(buf+strlen(buf), "RECORD_BYTES = 512\r\n");
    sprintf(buf+strlen(buf), "FILE_RECORDS = %d\r\n",fsize+2);
    sprintf(buf+strlen(buf), "LABEL_RECORDS = 2\r\n");
    sprintf(buf+strlen(buf), "FILE_STATE = CLEAN\r\n");
    sprintf(buf+strlen(buf), "^QUBE = 3\r\n");
    sprintf(buf+strlen(buf), "OBJECT = QUBE\r\n");
    sprintf(buf+strlen(buf), "    AXES = 3\r\n");
    if (V_ORG(s) == BIL) {
        sprintf(buf+strlen(buf), "    AXES_NAME = (SAMPLE,BAND,LINE)\r\n");
    } else if (V_ORG(s) == BSQ) {
        sprintf(buf+strlen(buf), "    AXES_NAME = (SAMPLE,LINE,BAND)\r\n");
    } else {
        sprintf(buf+strlen(buf), "    AXES_NAME = (BAND,SAMPLE,LINE)\r\n");
    }
    sprintf(buf+strlen(buf), "    CORE_ITEMS = (%d,%d,%d)\r\n", V_SIZE(s)[0],
        V_SIZE(s)[1],
        V_SIZE(s)[2]);
    sprintf(buf+strlen(buf), "    CORE_ITEM_BYTES = %d\r\n", 
        NBYTES(V_FORMAT(s)));
    sprintf(buf+strlen(buf), "    CORE_ITEM_TYPE = SUN_INTEGER\r\n");
    sprintf(buf+strlen(buf), "    CORE_BASE = 0.0\r\n");
    sprintf(buf+strlen(buf), "    CORE_MULTIPLIER = 1.0\r\n");
    sprintf(buf+strlen(buf), "    CORE_NAME = RAW_DATA_NUMBERS\r\n");
    sprintf(buf+strlen(buf), "    CORE_UNIT = DN\r\n");
    sprintf(buf+strlen(buf), "    OBSERVATION_NOTE = \"%s\"\r\n", title);
    sprintf(buf+strlen(buf), "END_OBJECT = QUBE\r\n");
    sprintf(buf+strlen(buf), "END\r\n");
    memset(buf+strlen(buf), ' ', 1024-strlen(buf));

    if (fp == NULL) {
        if ((fp = fopen(filename, "w")) == NULL) {
            return 0;
        }
    }

    fwrite(buf, 1, 1024, fp);
    fwrite(V_DATA(s), NBYTES(V_FORMAT(s)), dsize, fp);

    fclose(fp);
    return(1);
}

int
LoadISISHeader(FILE *fp, char *filename, int rec, char *element, Var **var)
{
    int scope = ODL_THIS_OBJECT;
    OBJDESC *ob, *item;
    KEYWORD * key;
    char *err_file = NULL;
    char *p, *q, *s, buf[256], c;
    Var *v = NULL;
    int count, i, type;

    if (is_ISIS(fp) == 0) return(0);
    rewind(fp);

    if (VERBOSE > 0) err_file = NULL;
    else err_file = "/dev/null";

    ob = (OBJDESC *)OdlParseLabelFptr(fp, err_file,
                                      ODL_EXPAND_STRUCTURE | ODL_EXPAND_CATALOG, 0);

    if (!ob || (key = OdlFindKwd(ob, "RECORD_BYTES", NULL, 0, 0)) == NULL) {
        OdlFreeTree(ob);
        return(0);
    }

    strcpy(buf, element);
    p = buf;
    item = ob;
    while (p && *p && (q = strchr(p, '.')) != NULL) {
        *q = '\0';
        if ((item = OdlFindObjDesc(item, p, NULL, 0, 0, 0)) == NULL) {
            sprintf(error_buf, "header(): Unable to find ISIS object: %s\n", p);
            parse_error(NULL);
            return(1);
        }
        p = q+1;
    }
    if ((key = OdlFindKwd(item, p, NULL, 0, scope)) == NULL) {
        /**
        ** Last ditch effort to find using global scope
        **/
        key = OdlFindKwd(item, p, NULL, 0, 0);
    }
    if (key != NULL) {
        switch(OdlGetKwdValueType(key)) {
        case ODL_INTEGER:
            v = new(Var);
            make_sym(v, INT, key->value);
            V_TYPE(v) = ID_VAL;
            break;
        case ODL_REAL:
            v = new(Var);
            make_sym(v, FLOAT, key->value);
            V_TYPE(v) = ID_VAL;
            break;

        case ODL_SYMBOL:
        case ODL_TEXT:
        case ODL_DATE:
        case ODL_DATE_TIME:
            v = new(Var);
            V_STRING(v) = strdup(key->value);
            V_TYPE(v) = ID_STRING;
            break;

        case ODL_SEQUENCE:
        case ODL_SET:
            /**
            ** count number in set.
            **/

            p = key->value;
            q = (char *)OdlValueStart(p);
            s = (char *) OdlValueEnd(q);
            c = *(s+1); *(s+1) = '\0';
            type = OdlDataType(q);
            *(s+1) = c;

            count = 0;
            while ((q = OdlValueStart(p)) != NULL && *q != '\0') {
                count++;
                s = (char *) OdlValueEnd(q);
                p = s+1;
            }

            v = new(Var);
            V_TYPE(v) = ID_VAL;
            V_ORG(v) = BSQ;
            V_DSIZE(v) = V_SIZE(v)[0] = count;
            V_SIZE(v)[1] = V_SIZE(v)[2] = 1;

            p = key->value;
            if (type == ODL_REAL) {
                double *data ;
                data = (double *)calloc(count, sizeof(double));
                V_FORMAT(v) = DOUBLE;
                i = 0;
                while ((q = OdlValueStart(p)) != NULL && *q != '\0') {
                    data[i++] = strtod(q, NULL);
                    s = (char *) OdlValueEnd(q);
                    p = s+1;
                }
                V_DATA(v) = data;
            } else {
                int *data;
                data = (int *)calloc(count, sizeof(int));
                V_FORMAT(v) = INT;
                i = 0;
                while ((q = OdlValueStart(p)) != NULL && *q != '\0') {
                    data[i++] = atoi(q);
                    s = (char *) OdlValueEnd(q);
                    p = s+1;
                }
                V_DATA(v) = data;
            }
        }
    } else {
        sprintf(error_buf, "header(): Unable to find ISIS object: %s\n", buf);
        parse_error(NULL);
        return(1);
    }
    *var = v;
    return(1);
}


static unsigned int k;

/*----------------------------------------------------------------------
  VAX real to IEEE real format conversion from VICAR RTL.
  ----------------------------------------------------------------------*/

void
vax_ieee_r(float *from, float *to)
{
    unsigned int *j;
    unsigned int k0, k1;
    unsigned int sign, exp, mant;
    float *f;

    j = (unsigned int *) from;

    /* take care of byte swapping */
    k0 = ((*j & 0xFF00FF00) >> 8) & 0x00FF00FF;
    k1 = ((*j & 0x00FF00FF) << 8) | k0;

    if (k1) {
        /* get sign bit */
        sign = k1 & 0x80000000;

        /* turn off sign bit */
        k1 = k1 ^ sign;

        if (k1 & 0x7E000000) {
            /* fix exponent by subtracting one, since
               IEEE uses an implicit 2^0 not an implicit 2^-1 as VAX does */
            k = (k1 - 0x01000000) | sign;
        } else {
            exp = k1 & 0x7F800000;
            mant = k1 & 0x007FFFFF;
            if (exp == 0x01000000) {
                /* fix for subnormal numbers */
                k = (mant | 0x00800000) >> 1 | sign;
            } else if (exp == 0x00800000) {
                /* fix for subnormal numbers */
                k = (mant | 0x00800000) >> 2 | sign;
            } else if (sign) {
                /* not a number */
                k = 0x7FFFFFFF;
            } else {
                /* zero */
                k = 0x00000000;
            }
        }
    } else {
        /* zero */
        k = 0x00000000;
    }
    f = (float *) (&k);
    *to = *f;
}

void
long_byte_swap(unsigned char from[4], unsigned char to[4])
/* Types do not need to match */
{
    to[0] = from[1];
    to[1] = from[0];
    to[2] = from[3];
    to[3] = from[2];
}

/*----------------------------------------------------------------------
  IEEE real to VAX real format conversion from VICAR RTL.
  ----------------------------------------------------------------------*/
void
ieee_vax_r(int *from, float *to)
{
    unsigned int sign, exponent;
    int f;
    float g;

    f = *from;
    exponent = f & 0x7f800000;
    if (exponent >= 0x7f000000) {   /*if too large */
        sign = f & 0x80000000;  /*sign bit */
        f = 0x7fffffff | sign;
    } else if (exponent == 0) { /*if 0 */
        f = 0;      /*vax 0 */
    } else {
        f += 0x01000000;    /*add 2 to exponent */
    }
    long_byte_swap((unsigned char *)&f, (unsigned char *)&g);
    *to = g;
}

void * read_qube_suffix(int fd, struct _iheader *h,
	int s_bytes,int plane_number,int s_item_byte,int ordinate)
{
/*** ordinate: 0=minor, 1=middle, 2=major ***/

    void *data;
    void *p_data;
    int dsize;
    int i, x, y, z;
    int c_bytes;
    int dim[3];                 /* dimension of output data */
    int d[3];                   /* total dimension of file */
    int plane;
    int count;
    int err;
    int offset1,offset2,offset3;

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
    c_bytes = NBYTES(h->format);

    for (i = 0; i < 3; i++) {
        if (h->size[i] == 0) h->size[i] = 1;
        if (h->s_lo[i] == 0) h->s_lo[i] = 1;
        if (h->s_hi[i] == 0) h->s_hi[i] = h->size[i];
        if (h->s_skip[i] == 0) h->s_skip[i] = 1;
    
        h->s_lo[i]--;           /* value is 1-N.  Switch to 0-(N-1) */
        h->s_hi[i]--;
    
    
        if (i && (h->suffix[i] + h->prefix[i]) % c_bytes != 0) {
            fprintf(stderr, "Warning!  Prefix+suffix not divisible by pixel size\n");
        }
    }
    if (h->gain == 0.0)
        h->gain = 1.0;


    if (ordinate==2) {
	plane=h->size[0]*h->size[1]*s_bytes;
	dsize=plane;
	offset1=plane_number*s_bytes*h->size[0]*h->size[1];
	offset2=0;
	offset3=h->size[0]*s_bytes;
    }

    else if (ordinate==1) {
	plane=h->size[0]*h->suffix[1];
 	dsize=h->size[2]*h->size[0]*s_bytes;
	offset1=h->size[1]*h->size[0]*c_bytes+h->size[1]*h->suffix[0];
	offset2=h->size[0]*s_bytes*plane_number;
	offset3=0;
    }

    else {
	plane=h->size[0]*h->size[1]*c_bytes+h->size[1]*h->suffix[0];
	dsize=h->size[2]*h->size[1]*s_bytes;
	offset1=0;
	offset2=h->size[0]*c_bytes+plane_number*s_bytes;
	offset3=h->size[0]*c_bytes+h->suffix[0];
    }

/**
     ** compute output size, allocate memory.
     **/
    
    if ((data = malloc(dsize)) == NULL) {
        fprintf(stderr, "Unable to allocate memory.\n");
        return (NULL);
    }
    /**
     ** Allocate some temporary storage space
     **/
    
    
    if ((p_data = malloc(plane)) == NULL) {
        free(data);
        return (NULL);
    }
    /**
     ** loop, doesn't do skips yet.
     **/
    
    count = 0;

    for (z=h->s_lo[2];z<h->s_hi[2];z+=h->s_skip[2]){
//	printf("Z:=%d\n",z);
	lseek(fd,h->dptr+z*(h->size[0]*
		h->size[1]*c_bytes+
			h->size[0]*h->suffix[1]+
				h->size[1]*h->suffix[0])+
					offset1,0);

    		
	if ((err = read(fd, p_data, plane)) != plane) {
            parse_error("Early EOF");
            break;
        }

	if (ordinate==2 && h->s_skip[0]==1 && h->s_skip[1]==1){
		memcpy((char *)data,(char *)p_data,plane);
	}

	else {
		for (y=h->s_lo[1];y<h->s_hi[1];y+=h->s_skip[1]){ 
			if (ordinate==1 && h->s_skip[0]==1 && h->s_skip[2]==1){
				memcpy((char *)data+count,(char *)p_data+offset2,h->size[0]*s_bytes);
				count+=h->size[0]*s_bytes;
			}
			else { 
				for(x = h->s_lo[0]; x < h->s_hi[0]; x += h->s_skip[0]){
					memcpy((char *)data+count,(char *)p_data+
						offset2+y*offset3+
						(x-h->s_lo[0])*s_bytes,s_bytes);
					count+=s_bytes;
				}
			}
		}
	} 
   }

   if (VERBOSE > 1)
            fprintf(stderr, ".");

    /**
    ** convert VAX_FLOAT if necessary.
    **/
	
    if (s_item_byte == VAX_FLOAT) {
        int i;
        for (i = 0 ; i < (dsize/s_bytes) ; i++) {
            vax_ieee_r(&((float *)data)[i],&((float *)data)[i]);
        }
    } else if (s_item_byte == VAX_INTEGER) {
        /**
        ** byte swap 'em.
        **/
        swab((const char *)data, (char *)data, dsize);
    }

   free(p_data);
   return(data);
}

int LookUpSuffix(OBJDESC *qube,char *name, int *plane, int *type, struct _iheader *h, int *suffix)
{
    KEYWORD *key;
    char **list;
    int scope = ODL_THIS_OBJECT;
    int i,j,n;
    for (i = 0; i < 3; i++) {
            char str[256];
            char str2[256];
            switch (i) {
                case 0: strcpy(str, "SAMPLE_SUFFIX"); break;
                case 1: strcpy(str, "LINE_SUFFIX"); break;
                case 2: strcpy(str, "BAND_SUFFIX"); break;
            }
            if (suffix[orders[h->org][i]]) {
                sprintf(str2, "%s_NAME", str);
                fprintf(stderr, "\n");
                if ((key = OdlFindKwd(qube, str2, NULL, 0, scope))) {
                    n = OdlGetAllKwdValuesArray(key, &list);
                    if (n == suffix[orders[h->org][i]]) {
                        for (j = 0; j < suffix[orders[h->org][i]]; j++) {
                                if(!strcmp(list[j],name)){
					*plane=j;
					*type=orders[h->org][i];
					return(0);
				}
                        }
                    } 
                    else {
                        parse_error("suffix name mismatch\n");
			return(1);
                    }
                } 
                else {
                    parse_error("Unable to find suffix name\n");
		    return(1);

                }
            }
   }
   return(1);
}

Var * ff_read_suffix_plane(vfuncptr func, Var * arg)
{

/**************************
  type is meant as:
	0: Sample Suffix
	1: Line Suffix
	2: Band Suffix
**************************/


    OBJDESC *object, *qube;
    KEYWORD *key;
    int scope = ODL_THIS_OBJECT;

    struct _iheader s;
    char *fname, **list, fname2[256];
    FILE *fp;
    int suffix[3] = {0, 0, 0};
    int suffix_bytes;
    int sfx, n, i, j;
    char *which_qube = NULL;
    void *data;
    char *isisfile;
    char *name;
    int type=2, plane=0;
    Var	*Suffix=NULL;
    int s_suffix_item_bytes;
    
    
    char *options[]={"Sample","Line","Band",NULL};

    char *axis[]={"SAMPLE_","LINE_","BAND_"};
    char suffix_item_byte[80];
    char suffix_item_type[80];

    int ac;
    Var **av, *v;
    Alist alist[4];

    alist[0] = make_alist("filename", ID_STRING, NULL, &isisfile);
    alist[1] = make_alist("plane", ID_UNK,NULL, &Suffix);
    alist[2] = make_alist("type", ID_ENUM,options, &name);
    alist[3].name = NULL;
    
    make_args(&ac, &av, func, arg);
    if (parse_args(ac, av, alist))
        return (NULL);

    if (isisfile == NULL) {
        parse_error("No filename specified.");
        return (NULL);
    }

    if ((fname = locate_file(isisfile)) == NULL ||
        (fp = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", isisfile);
        return (NULL);
    }

    strcpy(fname2, fname);
    free(fname);
    fname = fname2;

    if (GetISISHeader(fp, fname, &s, &object) == 0) {
       free(fname);
       return (NULL);
    }

    if ((qube = OdlFindObjDesc(object, "QUBE", NULL, 0, 0, 0)) == NULL) {
      fprintf(stderr, "%s: Not a qube object.\n", isisfile);
      return (NULL);
    }

    if ((key = OdlFindKwd(qube, "SUFFIX_ITEMS", NULL, 0, scope))) {
        sscanf(key->value, "(%d,%d,%d)", &suffix[0], &suffix[1], &suffix[2]);
    }


    if (Suffix==NULL){
       fprintf(stderr, "Suffix size (Samples,Lines,Bands) = (%6d,%6d,%6d)\n",
            suffix[orders[s.org][0]], suffix[orders[s.org][1]], suffix[orders[s.org][2]]);

       for (i = 0; i < 3; i++) {
            char str[256];
            char str2[256];
            switch (i) {
                case 0: strcpy(str, "SAMPLE_SUFFIX"); break;
                case 1: strcpy(str, "LINE_SUFFIX"); break;
                case 2: strcpy(str, "BAND_SUFFIX"); break;
            }
            if (suffix[orders[s.org][i]]) {
                sprintf(str2, "%s_NAME", str);
                fprintf(stderr, "\n");
                if ((key = OdlFindKwd(qube, str2, NULL, 0, scope))) {
                    n = OdlGetAllKwdValuesArray(key, &list);
                    if (n == suffix[orders[s.org][i]]) {
                        for (j = 0; j < suffix[orders[s.org][i]]; j++) {
                            fprintf(stderr, "%s_SUFFIX Plane %3d: '%s'\n",
                                str, j, list[j]);
                        }
                    } else {
                        parse_error("suffix name mismatch\n");
                    }
                } else {
                    parse_error("Unable to find suffix names\n");

                }
            }
        }
    	
        return(NULL);
    }

    else if (V_TYPE(Suffix)==ID_STRING) {
	name=V_STRING(Suffix);
	if(LookUpSuffix(qube,name,&plane,&type,&s,suffix))
		return(NULL);
    }

    else {
		plane=extract_int(Suffix,0);
		if (!strcasecmp(name, "Band")) type=2;
		if (!strcasecmp(name, "Line")) type=1;
		if (!strcasecmp(name, "Sample")) type=0;
    }

    strcpy(suffix_item_byte,axis[type]);
    strcpy(suffix_item_type,axis[type]);
    strcat(suffix_item_byte,"SUFFIX_ITEM_BYTES");
    strcat(suffix_item_type,"SUFFIX_ITEM_TYPE");

    if (plane<0) plane=0;

    if(type <0 || type > 2){
	fprintf(stderr, "Illegal axis type specified: %d\n",type);
	return(NULL);
    }



    if ((plane)>= suffix[orders[s.org][type]]){ 
        fprintf(stderr, "The cube only has %d %s-Suffix plane%s\n",
		suffix[orders[s.org][type]],
		((type==0) ? ("Sample"): ((type==1) ? ("Line"):("Band"))),
		(suffix[orders[s.org][type]] > 1 ? ("s"):("")));
	return(NULL);
    }

    if ((key = OdlFindKwd(qube, suffix_item_byte, NULL, 0, scope))) {
       n = OdlGetAllKwdValuesArray(key, &list);

           switch (atoi(list[plane])) {
            case 1:
                s_suffix_item_bytes = BYTE;
                break;
            case 2:
                s_suffix_item_bytes = SHORT;
                break;
            case 4:
                s_suffix_item_bytes = INT;
                break;
            default:
                parse_error( "Unrecognized data format: %s = %s",
                        suffix_item_byte, key->value);
                break;
           }
     }

    if ((key = OdlFindKwd(qube, suffix_item_type, NULL, 0, scope))) {
       n = OdlGetAllKwdValuesArray(key, &list); 

       if (s_suffix_item_bytes == INT && !strcmp(list[plane], "REAL")) {
           s_suffix_item_bytes = FLOAT;
       }
       if (s_suffix_item_bytes == INT && !strcmp(list[plane], "SUN_REAL")) {
           s_suffix_item_bytes = FLOAT;
       }   
       if (s_suffix_item_bytes == INT && !strcmp(list[plane], "VAX_REAL")) {
           s_suffix_item_bytes = VAX_FLOAT;
       }   
    }
	
    suffix_bytes=s.suffix[orders[s.org][type]]/suffix[orders[s.org][type]];
    
    for (i = 0 ; i < 3 ; i++) {
        s.s_lo[i] = 0;	//Just to make sure!!
	s.s_hi[i]=s.size[i]+1;
    }

    s.s_lo[orders[s.org][type]]=s.size[orders[s.org][type]]+1;
    s.s_hi[orders[s.org][type]]=s.s_lo[orders[s.org][type]]+1;	

    data=read_qube_suffix(fileno(fp),&s,suffix_bytes,plane,s_suffix_item_bytes,orders[s.org][type]);

    s.s_hi[orders[s.org][type]]=1;
		
    if (s_suffix_item_bytes==VAX_FLOAT) s_suffix_item_bytes=FLOAT;
    if (s_suffix_item_bytes==VAX_INTEGER)  s_suffix_item_bytes=INT;
		
    return(newVal(s.org,s.s_hi[0],s.s_hi[1],s.s_hi[2],s_suffix_item_bytes,data));
}


/**
 ** read and write ISIS suffix planes
 **/

Var *
ff_isis_summary(vfuncptr func, Var * arg)
{
    OBJDESC *object, *qube;
    KEYWORD *key;
    int scope = ODL_THIS_OBJECT;

    struct _iheader h;
    char *filename, *fname, **list, fname2[256];
    FILE *fp;
    int suffix[3] = {0, 0, 0};
    int s[3], n, i, j;
    char *which_qube = NULL;
    void *data;

    char *options[] =
    {
        "core", 
		"line", "line_suffix", 
		"sample", "sample_suffix", 
		"band", "band_suffix"
    };

    int ac;
    Var **av, *v;
    Alist alist[3];
    alist[0] = make_alist("filename", ID_STRING, NULL, &filename);
    alist[1] = make_alist("qube", ID_ENUM, options, &which_qube);
    alist[2].name = NULL;

    make_args(&ac, &av, func, arg);
    if (parse_args(ac, av, alist))
        return (NULL);

    if (filename == NULL) {
        parse_error("No filename specified.");
        return (NULL);
    }
    if ((fname = locate_file(filename)) == NULL ||
        (fp = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", filename);
        return (NULL);
    }
    strcpy(fname2, fname);
    free(fname);
    fname = fname2;

    if (GetISISHeader(fp, fname, &h, &object) == 0) {
        free(fname);
        return (NULL);
    }

    if ((qube = OdlFindObjDesc(object, "QUBE", NULL, 0, 0, 0)) == NULL) {
        fprintf(stderr, "%s: Not a qube object.\n", filename);
        return (NULL);
    }
    if (which_qube == NULL) {
        /**
         ** Just output information about file
         **/
        fprintf(stderr, "%s: ISIS %s qube\n", filename, Org2Str(h.org));

        fprintf(stderr, "Core   size (Samples,Lines,Bands) = (%6d,%6d,%6d)\n",
            GetSamples(h.size, h.org),
            GetLines(h.size, h.org),
            GetBands(h.size, h.org));

        if ((key = OdlFindKwd(qube, "SUFFIX_ITEMS", NULL, 0, scope))) {
            sscanf(key->value, "(%d,%d,%d)", &suffix[0], &suffix[1], &suffix[2]);
        }
        s[0] = suffix[orders[h.org][0]];
        s[1] = suffix[orders[h.org][1]];
        s[2] = suffix[orders[h.org][2]];

        fprintf(stderr, "Suffix size (Samples,Lines,Bands) = (%6d,%6d,%6d)\n",
            s[0], s[1], s[2]);

        for (i = 0; i < 3; i++) {
            char str[256];
            char str2[256];
            switch (i) {
                case 0: strcpy(str, "SAMPLE_SUFFIX"); break;
                case 1: strcpy(str, "LINE_SUFFIX"); break;
                case 2: strcpy(str, "BAND_SUFFIX"); break;
            }
            if (s[i]) {
                sprintf(str2, "%s_NAME", str);
                fprintf(stderr, "\n");
                if ((key = OdlFindKwd(qube, str2, NULL, 0, scope))) {
                    n = OdlGetAllKwdValuesArray(key, &list);
                    if (n == s[i]) {
                        for (j = 0; j < s[i]; j++) {
                            fprintf(stderr, "%s_SUFFIX Plane %3d: '%s'\n",
                                str, j, list[j]);
                        }
                    } else {
                        parse_error("suffix name mismatch\n");
                    }
                } else {
                    parse_error("Unable to find suffix names\n");

                }
            }
        }
        return (NULL);
    } else {
#if 0
        if (!strncasecmp(which_qube, "qube", 4)) {
            return(LoadISIS(fp, fname, 0));
        } else if (!strncasecmp(which_qube, "sample", 6)) {
            i = 0;
        } else if (!strncasecmp(which_qube, "line", 4)) {
            i = 1;
        } else if (!strncasecmp(which_qube, "band", 4)) {
            i = 2;
        }
        /**
         ** Shift the core into prefix, and the suffix into core.
         **/
        j = orders[h.org][i];
        h.prefix[j] = h.size[j];
        h.size[j] = h.suffix[j];
        h.suffix[j] = 0;

        /**
         ** Find SUFFIX_BYTES
         **/
        if ((key = OdlFindKwd(qube, "SUFFIX_BYTES", NULL, 0, scope))) {
            suffix_bytes = key->value;
        }

        /**
         ** Need to locate %s_SUFFIX_ITEM_TYPE
         **/ 

         START HERE.

        

        if ((data = read_qube_data(fileno(fp), &h)) == NULL) {
            return(NULL);
        }
        v = iheader2var(&h);
        V_DATA(v) = data;
#endif
    }

/*TESTING ROUTINE...SHOULD BE REMOVED!!!!*****/

}
