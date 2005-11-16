#include <strings.h>
#include <assert.h>
#include <sys/types.h>
#include <regex.h>
#include "parser.h"
#include "dvio.h"

#ifdef HAVE_LIBISIS
#include "isisdef.h"
#include "q.h"
#endif /* HAVE_LIBISIS */

char *iformat_to_eformat(Var *obj);
static void * dv_RePackData(void *data, int file_bytes, iom_idf data_type, int data_chunk);
static void * 
dv_read_qube_suffix(int fd, 
                    struct iom_iheader *h,
                    int s_bytes,
                    int plane_number,
                    int s_item_byte_e, /* external data type as stored in file */
                    int *s_item_byte,  /* internal representation in memory */
                    int ordinate,
                    int *size);

static const char *get_type_string(int type);
static const char *get_org_string(int org);
static int convert_data(int from_type, void *from_data,
                        int to_type, void *to_data, int nelements);
static void get_suffix_buff_sizes(int core_items[3], int suffix_items[3],
                                  int suffix_item_sizes[3]);
static int get_org_from_axis_names(char *axis_names[3], int *org);
static void write_suffix_zeros(FILE *fp, int nel);


static const char *axis_names[3] = {"sample", "line", "band"};


static void
lcase(char *name)
{
    int i, n;
    
    if (!name){ return; }

    n = strlen(name);
    for(i = 0; i < n; i++){
        name[i] = tolower(name[i]);
    }
}

static void
ucase(char *str)
{
    int i, n;
    
    if (!str){ return; }
    n = strlen(str);
    for(i = 0; i < n; i++){
        str[i] = toupper(str[i]);
    }
}



Var *
write_PDS_Qube(Var *core, Var *side, Var *bottom, Var *back, FILE *fp)
{
    Var *v;
    Var *suffix[3] = { NULL, NULL, NULL} ;
    int size[3];
    int suf_size[3];
    char *suf_names[3];
    Var *plane;
    char *name;
    char buf[2560];
    char lenstr[256];
    int i, j, k;
    int error = 0;
    int nbytes;
    int pos;
    int n;
    char *filename = NULL;
	char *p;
	int rec_len, lbl_length;
	Var *zero;
	int nsuffix[3] = { 0, 0, 0 };


    if (core == NULL) {
        parse_error("No core object specified");
        return(NULL);
    }


	suffix[0]=side;
	suffix[1]=bottom;
	suffix[2]=back;

   size[0] = GetX(core);
   size[1] = GetY(core);
   size[2] = GetZ(core);
   nbytes = GetNbytes(core);



	if (suffix[0] == NULL &&
		 suffix[1] == NULL &&
		 suffix[2] == NULL) { /* We're only writing the core, just dump it! */ 

		fwrite(V_DATA(core),nbytes,size[0]*size[1]*size[2],fp);

		return(core);
	}


	zero = newInt(0);

    /*
    ** Verify the size of each suffix plane
    */
    for (i = 0 ; i < 3 ; i++) {
        if (suffix[i] != NULL) {
            for (j = 0 ; j < get_struct_count(suffix[i]) ; j++) {
                get_struct_element(suffix[i], j, &name, &plane);
                suf_size[0] = GetX(plane);
                suf_size[1] = GetY(plane);
                suf_size[2] = GetZ(plane);

                switch (i) {
                case 0:	/* side */
                    if (size[1] != suf_size[1] || 
                        size[2] != suf_size[2] ||
                        suf_size[0] != 1) 
                        error++;
                    break;
                case 1:	/* bottom */
                    if (size[0] != suf_size[0] || 
                        size[2] != suf_size[2] ||
                        suf_size[1] != 1) 
                        error++;
                    break;

                case 2: /* back */
                    if (size[0] != suf_size[0] || 
                        size[1] != suf_size[1] ||
                        suf_size[2] != 1) 
                        error++;
                    break;
                }
                if (error) {
                    parse_error("Suffix plane does not match core size: "
                                "%d [%dx%dx%d] vs %dx%dx%d\n", 
                                name, 
                                suf_size[0], suf_size[1], suf_size[2],
                                size[0], size[1], size[2]);
                    return(NULL);
                }
            }
        }
    }

	nsuffix[0] = (suffix[0] ? get_struct_count(suffix[0]) : 0);
	nsuffix[1] = (suffix[1] ? get_struct_count(suffix[1]) : 0);
	nsuffix[2] = (suffix[2] ? get_struct_count(suffix[2]) : 0);

    if (V_ORG(core) == BSQ) {
        for (k = 0 ; k < size[2] ; k++) {
            for (j = 0 ; j < size[1] ; j++) {
                pos = (k*size[1]+j)*size[0] * nbytes;
                fwrite((char *)V_DATA(core) + pos, size[0], nbytes, fp);

				/* write sample suffix */
				for (n = 0 ; n < nsuffix[0] ; n++) {
					get_struct_element(suffix[0], n, NULL, &v);
					write_one(v, 0, j, k, fp);
				}
            }
			/* write line suffix */
			for (n = 0 ; n < nsuffix[1] ; n++) {
				get_struct_element(suffix[1], n, NULL, &v);
				write_row_x(v, 0, k, fp, nsuffix[0]);
			}
        }
		/* write band suffix */
		for (n = 0 ; n < nsuffix[2] ; n++) {
			get_struct_element(suffix[2], n, NULL, &v);
			write_plane(v, V_ORG(core), 2, fp, nsuffix[0], nsuffix[1]);
		}
    } else if (V_ORG(core) == BIP) {
        for (k = 0 ; k < size[1] ; k++) {		/* y axis */
            for (j = 0 ; j < size[0] ; j++) {	/* z axis */
                pos = (k*size[0]+j)*size[2] * nbytes;
                fwrite((char *)V_DATA(core) + pos, size[2], nbytes, fp);

				for (n = 0 ; n < nsuffix[2] ; n++) {
					get_struct_element(suffix[2], n, NULL, &v);
					write_one(v, j, k, 0, fp);
				}
            }
			for (n = 0 ; n < nsuffix[0] ; n++) {
				get_struct_element(suffix[0], n, NULL, &v);
				write_row_x(v, 0, k, fp, nsuffix[2]);
			}
        }

		for (n = 0 ; n < nsuffix[1] ; n++) {
			get_struct_element(suffix[1], n, NULL, &v);
			write_plane(v, V_ORG(core), 1, fp, nsuffix[2], nsuffix[0]);
		}
	}

   return(v);
}

int * fix_unsigned(struct iom_iheader *h, unsigned short *s);

Var *dv_LoadISISFromPDS(FILE *fp, char *fn, int dptr)
{
/* Want to read label and pull out only the minimal key words
	needed to define the header strucure for iom_read_qube_data.
	The offset will be suplied and only a few things about
	the label will be used (classic qube label reading can get confused)
*/

   OBJDESC 		*ob,*qube;
   char 			*err_file=NULL;
	KEYWORD 		*key,*key1,*key2;
	int 			size[3]={0};
	int 			prefix[3]={0};
	int 			suffix[3]={0};
	int			suffix_size[3]={0};
	int 			org;
	char			*ptr, p1[16], p2[16],p3[16];
	float 		gain;
	float			offset;
	int 			suffix_bytes = 0;
	struct 		iom_iheader *h = (struct iom_iheader *)malloc(sizeof(struct iom_iheader));
	Var 			*v=NULL;
	void			*data=NULL;
	int			scope=ODL_THIS_OBJECT;
	int			format;
	int 			i;
	int			_unsigned = 0;
	int			item_bytes=0;


	iom_init_iheader(h);

	ob = (OBJDESC *)OdlParseLabelFile(fn, err_file,ODL_EXPAND_STRUCTURE, VERBOSE==0);

	qube = NULL;
	qube = OdlFindObjDesc(ob, "QUBE", NULL, 0, 0, 0);

	if (qube == NULL)
		qube = OdlFindObjDesc(ob, "SPECTRAL_QUBE", NULL, 0, 0, 0);

	if (qube == NULL) {
		parse_error("Bad Call: this PDS file doesn't contain a QUBE or SPECTRAL_QUBE");
		return(NULL);
	}


	org = -1;
	if ((key = OdlFindKwd(qube, "AXES_NAME", NULL, 0, scope)) 
	|| (key = OdlFindKwd(qube, "AXIS_NAME", NULL, 0, scope))) {
		ptr = key->value;
		sscanf(ptr, " ( %[^,] , %[^,] , %[^)] ) ", p1, p2, p3);
		if (!strcmp(p1,"SAMPLE") && !strcmp(p2,"LINE") && !strcmp(p3,"BAND"))
			org = iom_BSQ;
		else if (!strcmp(p1,"SAMPLE") && !strcmp(p2,"BAND") && !strcmp(p3,"LINE"))
			org = iom_BIL;
		else if (!strcmp(p1,"BAND") && !strcmp(p2,"SAMPLE") && !strcmp(p3,"LINE"))
			org = iom_BIP;
		else {
			if (iom_is_ok2print_unsupp_errors()){
				fprintf(stderr, "Unrecognized data organization: %s = %s",
					"AXIS_NAME", key->value);
			}
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

	if ((key2 = OdlFindKwd(qube, "CORE_ITEM_BYTES", NULL, 0, scope)))
		item_bytes = atoi(key2->value);

	/**
	** This tells us if we happen to be using float vs int
	**/

	if ((key1 = OdlFindKwd(qube, "CORE_ITEM_TYPE", NULL, 0, scope)))
		if (strstr(key1->value,"UNSIGNED"))
			_unsigned = 1;


	format = iom_ConvertISISType(key1 ? key1->value : NULL,
		NULL, key2 ? key2->value : NULL);

	if (format == iom_EDF_INVALID){
		if (iom_is_ok2print_unsupp_errors()){
			fprintf(stderr, "%s has unsupported/illegal SIZE+TYPE combination.\n",
				fn);
		}
			return 0;
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

	for (i = 0 ; i < 3 ; i++) {
		if (suffix[i]) {
			suffix_size[i] = suffix[i] * suffix_bytes;
		}
	}

	h->org = org;           /* data organization */
	h->size[0] = size[0];
	h->size[1] = size[1];
	h->size[2] = size[2];
	h->eformat = (iom_edf)format;
	h->offset = offset;
	h->gain = gain;
	h->prefix[0] = h->prefix[1] = h->prefix[2] = 0;
	h->suffix[0] = suffix_size[0];
	h->suffix[1] = suffix_size[1];
	h->suffix[2] = suffix_size[2];
	h->corner = suffix[0]*suffix[1]*suffix_bytes;

	h->dptr = dptr;

	OdlFreeTree(ob);
	data = iom_read_qube_data(fileno(fp), h);

	/*
	** We need do promote unsigned short to signed int here
	*/

	if (item_bytes == 2 && _unsigned)
		data = fix_unsigned(h, data);

	v = iom_iheader2var(h);
	V_DATA(v) = data;

	iom_cleanup_iheader(h);
	return(v);
}

/**
 ** dv_LoadISISSuffixesFromPDS()
 **
 ** Loads ISIS suffixes and returns them in a davinci structure
 ** made up of at the most three sub-structures by the name of
 ** sample, line and band.
 **
 ** Returns NULL on error.
 **/
Var *
dv_LoadISISSuffixesFromPDS(FILE *fp, char *fname)
{
    OBJDESC *object = NULL, *qube = NULL;
    KEYWORD *key = NULL;
    int      scope = ODL_THIS_OBJECT;
    struct   iom_iheader h;
    void    *data = NULL;
    char     str[256];
    char     str2[256];
    char    **name_list = NULL, **size_list = NULL, **type_list = NULL;
    Var     *suffix_data[3] = {NULL, NULL, NULL};
    iom_edf  eformat;
    int      iformat;
    int      i, j, k, n, size;
    Var     *v;
    char    *msg_file = NULL;
    char    *suffix_names[3] = {"sample", "line", "band"};
    int      suffix[3] = {0, 0, 0}; /* suffix dimensions */



    
    if (VERBOSE){
        msg_file = NULL;
    }
    else {
#ifdef ___CYGWIN__
        msg_file = "nul:";
#else
        msg_file = "/dev/null";
#endif /* __CYGWIN__ */
    }
    
    if (iom_GetISISHeader(fp, fname, &h, msg_file, &object) == 0) {
        parse_error("%s: not an ISIS file", fname);
        return (NULL);
    }
    
    qube = NULL;
    qube = OdlFindObjDesc(object, "QUBE", NULL, 0, 0, 0);
    if (qube == NULL){
        qube = OdlFindObjDesc(object, "SPECTRAL_QUBE", NULL, 0, 0, 0);
    }
    
    if (qube  == NULL) {
        parse_error("%s: Not a qube object.", fname);
        return (NULL);
    }

    if ((key = OdlFindKwd(qube, "SUFFIX_ITEMS", NULL, 0, scope))) {
        sscanf(key->value, "(%d,%d,%d)", &suffix[0], &suffix[1], &suffix[2]);
    }

    /* process sample-, then line-, and then band- suffix-planes */
    for (i = 0; i < 3; i++) {
        
        switch (i) {
        case 0: strcpy(str, "SAMPLE_SUFFIX"); break;
        case 1: strcpy(str, "LINE_SUFFIX"); break;
        case 2: strcpy(str, "BAND_SUFFIX"); break;
        }
        
        if (suffix[iom_orders[h.org][i]]) {
            /* if this suffix plane has suffix data, then */

            /* find all the suffixes on this axis */
            sprintf(str2, "%s_NAME", str);
            if ((key = OdlFindKwd(qube, str2, NULL, 0, scope)) == NULL) {
                parse_error("Unable to find suffix names\n");
                continue;
            }
            
            n = OdlGetAllKwdValuesArray(key, &name_list);
            if (n != suffix[iom_orders[h.org][i]]) {
                parse_error("suffix name list is incomplete\n");
                continue;
            }

            /* lower-case names for consistency */
            for(k = 0; k < n; k++){ lcase(name_list[k]); }
            
            sprintf(str2, "%s_ITEM_BYTES", str);
            if ((key = OdlFindKwd(qube, str2, NULL, 0, scope)) == NULL) {
                parse_error("Unable to find suffix sizes\n");
                continue;
            }
            
            n = OdlGetAllKwdValuesArray(key, &size_list);
            if (n != suffix[iom_orders[h.org][i]]) {
                parse_error("suffix size list is incomplete\n");
                continue;
            }
            
            sprintf(str2, "%s_ITEM_TYPE", str);
            if ((key = OdlFindKwd(qube, str2, NULL, 0, scope)) == NULL) {
                parse_error("Unable to find suffix types\n");
                continue;
            }
            
            n = OdlGetAllKwdValuesArray(key, &type_list);
            if (n != suffix[iom_orders[h.org][i]]) {
                parse_error("suffix type list is incomplete\n");
                continue;
            }

            suffix_data[i] = new_struct(0);
            
            for (j = 0; j < suffix[iom_orders[h.org][i]]; j++) {

                /* set the iom_iheader appropriately for the read */
                for(k=0; k<3; k++){ h.s_lo[k] = 0; h.s_hi[k] = h.size[k]+1; }
                h.s_lo[iom_orders[h.org][i]] = h.size[iom_orders[h.org][i]]+1;
                h.s_hi[iom_orders[h.org][i]] = h.s_lo[iom_orders[h.org][i]]+1;

                /* read the data */
                eformat = iom_ConvertISISType(type_list[j], NULL, size_list[j]);
                data = dv_read_qube_suffix(fileno(fp), &h, atoi(size_list[j]), j,
                                           eformat, &iformat, iom_orders[h.org][i],
                                           &size);

                /* Fix the size of data plane read to just one-deep. This is a
                   bad way of fudging it s.t. we can pass it directly to newVal() */
                h.s_hi[iom_orders[h.org][i]] = 1;

                /* Use newVal() to package the data into a davinci variable */
                v = newVal(h.org, h.s_hi[0], h.s_hi[1], h.s_hi[2], ihfmt2vfmt(iformat), data);

                /* Add this variable with the plane's name to appropriate
                   suffix_data sturcture */
                add_struct(suffix_data[i], name_list[j], v);
            }
        }
    }

    /* assemble output struct with at the most three sub structures of data,
       one for each of the sample, line and band suffixes */
    v = new_struct(0);
    for(i = 0; i < 3; i++){
        if (suffix_data[i]){
            if (get_struct_count(suffix_data[i]) > 0){
                add_struct(v, suffix_names[i], suffix_data[i]);
            }
            else {
                mem_claim(suffix_data[i]);
                free_struct(suffix_data[i]);
                /* NOTE: if one does not do mem_claim and free_struct
                   the garbage collector will take care of it */
            }
        }
    }
    
    return(v);
}


int *
fix_unsigned(struct iom_iheader *h, unsigned short *s)
{
	int len;
	int *data = (int *) s; /* JAS: explicit cast correct? 18 nov 2003 */
	int i;

	if (h->eformat == iom_MSB_INT_2 || h->eformat == iom_LSB_INT_2) {
		len = h->size[0] * h->size[1] * h->size[2];
		data = calloc(len, sizeof(int));
		for (i = 0 ; i < len ; i++) {
			data[i] = (unsigned short)s[i];
		}
		free(s);
		if (h->eformat == iom_MSB_INT_2) h->eformat = iom_MSB_INT_4;
		if (h->eformat == iom_LSB_INT_2) h->eformat = iom_LSB_INT_4;
		h->format = iom_INT;
	}
	return(data);
}




Var *
dv_LoadISIS(FILE *fp, char *filename, struct iom_iheader *s)
{
    struct iom_iheader h;
    int org;
    Var *v=NULL;
    char hbuf[HBUFSIZE];
    void *data = NULL;
    int i,j;
    char *datafile = NULL;
    int fd;
    char *msg_file = NULL;

    if (iom_isISIS(fp) == 0) return(NULL);

    if (VERBOSE){
        msg_file = NULL;
    }
    else {
#ifdef __CYGWIN__
        msg_file = "nul:";
#else
        msg_file = "/dev/null";
#endif /* __CYGWIN__ */
    }
    
    if (iom_GetISISHeader(fp, filename, &h, msg_file, NULL) == 0) {
        return(NULL);
    }

    /**
     ** If user specified a record, subset out a specific band
     **/

    if (s != NULL) {
        /** 
         ** Set subsets
         **/
        iom_MergeHeaderAndSlice(&h, s);
    }

    
    /*
    ** Handle reading a detached label
    */
    
    datafile = h.ddfname; /* get the detached data file name */
    
    if (datafile != NULL) {
#ifdef __CYGWIN__
        if ((fd = open(datafile, O_RDONLY|O_BINARY)) < 0) {
#else 
        if ((fd = open(datafile, O_RDONLY)) < 0) {
#endif /* __CYGWIN__ */
            fprintf(stderr, "Unable to open data file: %s\n", datafile);
            return(NULL);
        }
        data = iom_read_qube_data(fd, &h);
        close(fd);
        
    } else {
        /**
         ** Put all this into a Var struct.
         **/
        data = iom_read_qube_data(fileno(fp), &h);
    }
    if (data == NULL) { iom_cleanup_iheader(&h); return(NULL); }
    
    v = iom_iheader2var(&h);
    V_DATA(v) = data;

    sprintf(hbuf, "%s: ISIS %s image: %dx%dx%d, %s (%d bits)",
            filename, iom_Org2Str(h.org),
            iom_GetSamples(h.size, h.org), 
            iom_GetLines(h.size, h.org), 
            iom_GetBands(h.size, h.org), 
            iom_Format2Str(h.format), iom_NBYTESI(h.format)*8);

    if (VERBOSE > 1) { parse_error(hbuf); }

    iom_cleanup_iheader(&h);
    
    return(v);
}


int 
dv_WriteISIS(Var *s, char *filename, int force, char *title)
{
    struct iom_iheader h;
    int status;

    /*
    ** Build an _iheader structure from the "Var *s" that
    ** is suitable for writing an ISIS output file.
    */

    var2iom_iheader(s, &h);

    if (VERBOSE > 1){
        fprintf(stderr, "Writing %s: %dx%dx%d ISIS file.\n",
                filename, h.size[0], h.size[1], h.size[2]);
    }

    status = iom_WriteISIS(filename, V_DATA(s), &h, force, title);
    iom_cleanup_iheader(&h);

    if (status == 0){
        parse_error("Unable to write ISIS file %s.\n", filename);
        return 0;
    }
    
    return 1;
}


int
dv_LoadISISHeader(FILE *fp, char *filename, int rec, char *element, Var **var)
{
    int scope = ODL_THIS_OBJECT;
    OBJDESC *ob, *item;
    KEYWORD * key;
    char *err_file = NULL;
    char *p, *q, *s, buf[256], c;
    Var *v = NULL;
    int count, i, type;

    if (iom_isISIS(fp) == 0) return(0);
    rewind(fp);

    if (VERBOSE > 0) err_file = NULL;
#ifdef __CYGWIN__
    else err_file = "nul:";
#else
    else err_file = "/dev/null";
#endif /* __CYGWIN__ */

    ob = (OBJDESC *)OdlParseLabelFptr(fp, err_file,
                                      ODL_EXPAND_STRUCTURE | ODL_EXPAND_CATALOG, VERBOSE==0);

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
            v = newVar();
            make_sym(v, INT, key->value);
            V_TYPE(v) = ID_VAL;
            break;
        case ODL_REAL:
            v = newVar();
            make_sym(v, FLOAT, key->value);
            V_TYPE(v) = ID_VAL;
            break;

        case ODL_SYMBOL:
        case ODL_TEXT:
        case ODL_DATE:
        case ODL_DATE_TIME:
            v = newVar();
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

            v = newVar();
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

static void * 
dv_read_qube_suffix(int fd, 
                    struct iom_iheader *h,
                    int s_bytes,       /* suffix (item) bytes */
                    int plane_number,  /* plane serial number of the suffix-planes */
                    int s_item_byte_e, /* external data type as stored in file */
                    int *s_item_byte,  /* internal representation in memory */
                    int ordinate,      /* sample/line/band in org-order */
                    int *size)
{
/*** ordinate: 0=minor, 1=middle, 2=major ***/

    void *data;
    void *p_data;
    int dsize;
    int i, x, y, z;
    int c_bytes;
    int plane;
    int count;
    int err;
    int offset1,offset2,offset3;
    int format;
    
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
    c_bytes = iom_NBYTES(h->eformat);
    
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
        plane=h->size[0]*h->size[1]*s_bytes +
                h->suffix[0]*h->size[1] +
                h->suffix[1]*h->size[0] +
                h->corner;
        dsize=h->size[0]*h->size[1]*s_bytes;
        offset1=plane_number*plane;
        offset2=0;
        offset3=h->size[0]*s_bytes + h->suffix[0];
    }

    else if (ordinate==1) {
        plane=h->size[0]*h->suffix[1] +
                (h->suffix[0]/s_bytes)*h->suffix[1];
        dsize=h->size[2]*h->size[0]*s_bytes;
        offset1=h->size[1]*h->size[0]*c_bytes+h->size[1]*h->suffix[0];
        offset2=(h->size[0]*s_bytes+h->suffix[0])*plane_number;
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
    *size=dsize;
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
        /*  printf("Z:=%d\n",z); */
        int zz;

        if (z <= h->size[2]){ zz = z; } else { zz = h->size[2]-1; }
        lseek(fd,h->dptr+zz*(h->size[0]*h->size[1]*c_bytes+
                            h->size[0]*h->suffix[1]+
                            h->size[1]*h->suffix[0]+h->corner)+
              offset1,0);
        
            
        if ((err = read(fd, p_data, plane)) != plane) {
            parse_error("Early EOF");
            break;
        }

        if (ordinate==2 && h->s_skip[0]==1 && h->s_skip[1]==1 &&
            h->suffix[0] == 0 && h->suffix[1] == 0){
            memcpy((char *)data,(char *)p_data,plane);
        } else {
            for (y=h->s_lo[1];y<h->s_hi[1];y+=h->s_skip[1]){ 
                if (ordinate==1 && h->s_skip[0]==1 && h->s_skip[2]==1 &&
                    h->suffix[0] == 0){
                    memcpy((char *)data+count,
                           (char *)p_data+offset2,
                           h->size[0]*s_bytes);

                    count+=h->size[0]*s_bytes;
                } else { 
                    for(x = h->s_lo[0]; x < h->s_hi[0]; x += h->s_skip[0]){
                        memcpy((char *)data+count,
                               (char *)p_data+ offset2+y*offset3+ 
                               (x-h->s_lo[0])*s_bytes,
                               s_bytes);
                        count+=s_bytes;
                    }
                }
            }
        } 
    }

    if (VERBOSE > 1)
        fprintf(stderr, ".");

    *s_item_byte = iom_Eformat2Iformat(s_item_byte_e);

    if (s_bytes!=iom_NBYTESI(*s_item_byte))
        data=dv_RePackData(data,s_bytes,*s_item_byte,*size);


    /*
    ** byte_swap_data() returns the machine dependent type of
    ** data corresponding to the input data type passed in
    ** "s_item_byte_e".
    */
    format = iom_byte_swap_data(data, dsize/s_bytes, s_item_byte_e);

    free(p_data);
    return(data);
}

static void *
dv_RePackData(void *data, int file_bytes, iom_idf data_type, int data_chunk)
{
/**********************************************************
 ** data:    old data set we wish to repack
 ** file_bytes   Number of bytes per element in *data
 ** data_type    The acutal data type of the elements in *data
 **              see type "dvio_idf".
 ** data_chunk   The total size of data
 ** LOCAL:
 **  data_bytes:     Number of bytes per element for this data_type
 **  data_size:      Number of elements in *data 
 **  *p_data:    Our new re-packed data set
 *************************************************************/
    int data_bytes=iom_NBYTESI(data_type);
    int data_size=(data_chunk/file_bytes);
    void *p_data;
    int i,j,l;

    if ((p_data=malloc(data_size*data_bytes))==NULL){
        fprintf(stderr, "Unable to allocate memory.\n");
        return (NULL);
    }

    for (i=1;i<=data_size;i++){     /*Cycle through the number of elements */

        j=i*file_bytes-data_bytes;  /*Align j with LSB at element i in *data */
        l=i*data_bytes-data_bytes;  /*Align l with LSB at element i in *p_data */
        memcpy((char *)p_data+l,(char *)data+j,data_bytes);
    }

    free(data);
    return(p_data);
}


/*
** LookupSuffix() - Find the type and position of a named suffix plane
**
** Type is: 0 - sample
** Type is: 1 - line
** Type is: 2 - band
**
** Position is zero based.
*/

static int
dv_LookupSuffix(
    OBJDESC *qube,
    char *name, 
    int *plane, 
    int *type, 
    struct iom_iheader *h, 
    int *suffix)
{
    KEYWORD *key;
    char **list;
    int scope = ODL_THIS_OBJECT;
    int i,j,n;
    char str[256];
    char str2[256];
    int count = 0;

    for (i = 0; i < 3; i++) {
        switch (i) {
        case 0: strcpy(str, "SAMPLE_SUFFIX"); break;
        case 1: strcpy(str, "LINE_SUFFIX"); break;
        case 2: strcpy(str, "BAND_SUFFIX"); break;
        }
        if (suffix[iom_orders[h->org][i]]) {
            sprintf(str2, "%s_NAME", str);
            fprintf(stderr, "\n");
            if ((key = OdlFindKwd(qube, str2, NULL, 0, scope)) == NULL) {
                parse_error("Unable to find suffix names\n");
                return(1);
            }

            n = OdlGetAllKwdValuesArray(key, &list);
            if (n != suffix[iom_orders[h->org][i]]) {
                parse_error("suffix name list is incomplete\n");
                return(1);
            } else {
                for (j = 0; j < suffix[iom_orders[h->org][i]]; j++) {
                    if(!strncasecmp(list[j], name, strlen(name))){
                        /*
                        ** check if this is an exact match and return if so
                        */
                        *plane=j;
                        *type=i;
                        count++;
                        if (!strcasecmp(list[j], name)) {
                            return(0);
                        }
                    }
                }
            }
        }
    }

    if (count) {
        if (count > 1) {
            parse_error("Suffix name abbreviation is not unique: %s", name);
            return(1);
        } else {
            return(0);
        }
    }


    return(1);
}

Var *
ff_read_suffix_plane(vfuncptr func, Var * arg)
{

/**************************
                           type is meant as:
                           0: Sample Suffix
                           1: Line Suffix
                           2: Band Suffix
**************************/


    OBJDESC *object, *qube;
    KEYWORD *key, *key1, *key2;
    int scope = ODL_THIS_OBJECT;

    struct iom_iheader s;
    char *fname, **list, **list1, **list2, fname2[256];
    char **name_list, **type_list, **size_list;
    FILE *fp;
    int suffix[3] = {0, 0, 0};
    int suffix_bytes;
    int n, i, j;
    void *data;
    char *isisfile;
    char *name;
    int type=2, plane=0;
    Var *Suffix=NULL;
    int s_suffix_item_bytes;
    int chunk;
    int format;
    
    
    char *options[]={"Sample","Line","Band",NULL};

    char *axis[]={"SAMPLE_","LINE_","BAND_"};
    char suffix_item_byte[80];
    char suffix_item_type[80];
    char suffix_item_name[80];
    char *datafile = NULL;
    char *msg_file = NULL;

    
    int ac;
    Var **av;
    Alist alist[4];

    alist[0] = make_alist("filename", ID_STRING, NULL, &isisfile);
    alist[1] = make_alist("plane", ID_UNK, NULL, &Suffix);
    alist[2] = make_alist("type", ID_ENUM, options, &name);
    alist[3].name = NULL;
    
    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (isisfile == NULL) {
        parse_error("No filename specified.");
        return (NULL);
    }

    if ((fname = dv_locate_file(isisfile)) == NULL ||
        (fp = fopen(fname, "rb")) == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", isisfile);
        return (NULL);
    }

    strcpy(fname2, fname);
    free(fname);
    fname = fname2;

    
    if (VERBOSE){
        msg_file = NULL;
    }
    else {
#ifdef ___CYGWIN__
        msg_file = "nul:";
#else
        msg_file = "/dev/null";
#endif /* __CYGWIN__ */
    }

    if (iom_GetISISHeader(fp, fname, &s, msg_file, &object) == 0) {
        parse_error("%s: not an ISIS file", fname);
        fclose(fp);
        free(fname);
        return (NULL);
    }

	 qube = NULL;
	 qube = OdlFindObjDesc(object, "QUBE", NULL, 0, 0, 0);
	 if (qube == NULL)
		qube = OdlFindObjDesc(object, "SPECTRAL_QUBE", NULL, 0, 0, 0);

    if (qube  == NULL) {
        parse_error("%s: Not a qube object.", isisfile);
        fclose(fp);
        return (NULL);
    }

    if ((key = OdlFindKwd(qube, "SUFFIX_ITEMS", NULL, 0, scope))) {
        sscanf(key->value, "(%d,%d,%d)", &suffix[0], &suffix[1], &suffix[2]);
    }


    if (Suffix==NULL) {
        fprintf(stderr, "%s: ISIS %s qube\n", isisfile, iom_Org2Str(s.org));

        fprintf(stderr, "Core   size (Samples,Lines,Bands) = (%6d,%6d,%6d)\n",
                iom_GetSamples(s.size, s.org),
                iom_GetLines(s.size, s.org),
                iom_GetBands(s.size, s.org));

        fprintf(stderr, "Suffix size (Samples,Lines,Bands) = (%6d,%6d,%6d)\n",
                suffix[iom_orders[s.org][0]],
                suffix[iom_orders[s.org][1]],
                suffix[iom_orders[s.org][2]]);

        for (i = 0; i < 3; i++) {
            char str[256];
            char str2[256];
            switch (i) {
            case 0: strcpy(str, "SAMPLE_SUFFIX"); break;
            case 1: strcpy(str, "LINE_SUFFIX"); break;
            case 2: strcpy(str, "BAND_SUFFIX"); break;
            }
            if (suffix[iom_orders[s.org][i]]) {
                fprintf(stderr, "\n");

                sprintf(str2, "%s_NAME", str);
                if ((key = OdlFindKwd(qube, str2, NULL, 0, scope)) == NULL) {
                    parse_error("Unable to find suffix names\n");
                    continue;
                }
                
                n = OdlGetAllKwdValuesArray(key, &name_list);
                if (n != suffix[iom_orders[s.org][i]]) {
                    parse_error("suffix name list is incomplete\n");
                    continue;
                }

                sprintf(str2, "%s_ITEM_BYTES", str);
                if ((key = OdlFindKwd(qube, str2, NULL, 0, scope))) {
                    n = OdlGetAllKwdValuesArray(key, &size_list);
                    if (n != suffix[iom_orders[s.org][i]]) {
                        parse_error("suffix size list is incomplete\n");
                        continue;
                    }
                } else {
                    parse_error("Unable to find suffix sizes\n");
                    continue;
                }

                sprintf(str2, "%s_ITEM_TYPE", str);
                if ((key = OdlFindKwd(qube, str2, NULL, 0, scope))) {
                    n = OdlGetAllKwdValuesArray(key, &type_list);
                    if (n != suffix[iom_orders[s.org][i]]) {
                        parse_error("suffix type list is incomplete\n");
                        continue;
                    }
                } else {
                    parse_error("Unable to find suffix types\n");
                    continue;
                }

                for (j = 0; j < suffix[iom_orders[s.org][i]]; j++) {
                    iom_edf format = iom_ConvertISISType(type_list[j], NULL, size_list[j]);
                    fprintf(stderr, "%s Plane %d: '%s' (%s byte %s)\n",
                            str, j, name_list[j], 
                            size_list[j], 
                            iom_EFormat2Str(format));
                }
            }
        }
        fclose(fp);
        return(NULL);
        
    } else if (V_TYPE(Suffix)==ID_STRING) {
        name=V_STRING(Suffix);
        if(dv_LookupSuffix(qube, name, &plane, &type, &s, suffix)) {
            parse_error("Unable to find plane: %s", name);
            fclose(fp);
            return(NULL);
        }
    } else {
        plane=extract_int(Suffix,0);
        if (!strcasecmp(name, "Band")) type=2;
        if (!strcasecmp(name, "Line")) type=1;
        if (!strcasecmp(name, "Sample")) type=0;
    }

    /*
    ** Below here we are extracting a single plane
    */

    sprintf(suffix_item_byte,"%sSUFFIX_ITEM_BYTES", axis[type]);
    sprintf(suffix_item_type,"%sSUFFIX_ITEM_TYPE", axis[type]);
    sprintf(suffix_item_name,"%sSUFFIX_NAME", axis[type]);

    if (plane<0) plane=0;

    if(type <0 || type > 2){
        fprintf(stderr, "Illegal axis type specified: %d\n",type);
        fclose(fp);
        return(NULL);
    }


    if ((plane)>= suffix[iom_orders[s.org][type]]){ 
        fprintf(stderr, "The cube only has %d %s-Suffix plane%s\n",
                suffix[iom_orders[s.org][type]],
                ((type==0) ? ("Sample"): ((type==1) ? ("Line"):("Band"))),
                (suffix[iom_orders[s.org][type]] > 1 ? ("s"):("")));
        fclose(fp);
        return(NULL);
    }
    
    suffix_bytes=s.suffix[iom_orders[s.org][type]]/suffix[iom_orders[s.org][type]];

    list2 = NULL;
    if ((key2 = OdlFindKwd(qube, suffix_item_byte, NULL, 0, scope))) {
        n = OdlGetAllKwdValuesArray(key2, &list2);
    }

    list1 = NULL;
    if ((key1 = OdlFindKwd(qube, suffix_item_type, NULL, 0, scope))) {
        n = OdlGetAllKwdValuesArray(key1, &list1); 
    }

    s_suffix_item_bytes = iom_ConvertISISType(list1 ? list1[plane] : NULL,
                                              NULL,
                                              list2 ? list2[plane] : NULL);
    
    if ((key = OdlFindKwd(qube, suffix_item_name, NULL, 0, scope))) {
        n = OdlGetAllKwdValuesArray(key, &name_list);
        if (n == suffix[iom_orders[s.org][type]]) {
            fprintf(stderr, "Extracting %.*s '%s'\n", 
                    strlen(suffix_item_name)-5, 
                    suffix_item_name, 
                    name_list[plane]);
        } else {
            parse_error("name list is incomplete");
        }
    } else {
        parse_error("unable to find name list");
    }

    for (i = 0 ; i < 3 ; i++) {
        s.s_lo[i] = 0;  /*Just to make sure!! */
        s.s_hi[i]=s.size[i]+1;
    }

    s.s_lo[iom_orders[s.org][type]]=s.size[iom_orders[s.org][type]]+1;
    s.s_hi[iom_orders[s.org][type]]=s.s_lo[iom_orders[s.org][type]]+1;  

    data=dv_read_qube_suffix(fileno(fp),
                             &s,
                             suffix_bytes,plane,
                             s_suffix_item_bytes,
                             &format, /* data now s_suffix_item_bytes -> format */
                             iom_orders[s.org][type],&chunk);

    fclose(fp);
    
    s.s_hi[iom_orders[s.org][type]]=1;

/*
    if (suffix_bytes!=iom_NBYTESI(format))
        data=dv_RePackData(data,suffix_bytes,format,chunk);
*/
    
    return(newVal(s.org,
                  s.s_hi[0],
                  s.s_hi[1],
                  s.s_hi[2],
                  ihfmt2vfmt(format),
                  data));
}


Var *
write_isis_planes(vfuncptr func, Var * arg)
{

    Var *core;
    Var *v;
    Var *suffix[3] = { NULL, NULL, NULL} ;
    int size[3];
    int suf_size[3];
    char *suf_names[3];
    Var *plane;
    char *name;
    char buf[2560];
    char lenstr[256];
    int i, j, k;
    int error = 0;
    int nbytes;
    FILE *fp;
    int pos;
    int n;
    char *filename = NULL;
	char *p;
	int rec_len, lbl_length;
	Var *zero;
	int nsuffix[3] = { 0, 0, 0 };
	char *fname;
	int force=0;

    Alist alist[7];
    alist[0] = make_alist( "core",    	ID_VAL,    	NULL,    &core);
    alist[1] = make_alist( "side", 	ID_STRUCT,  NULL,    &suffix[0]);
    alist[2] = make_alist( "bottom",	ID_STRUCT,  NULL,    &suffix[1]);
    alist[3] = make_alist( "back",  	ID_STRUCT,  NULL,    &suffix[2]);
    alist[4] = make_alist( "filename",  ID_STRING,  NULL,    &filename);
    alist[5] = make_alist( "force",  INT,  NULL,    &force);
    alist[6].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (core == NULL) {
        parse_error("%s: No core object specified", func->name);
        return(NULL);
    }

    if (filename == NULL) {
        parse_error("%s: No filename specified\n", func->name);
        return(NULL);
    }
	if ((fname = dv_locate_file(filename)) == NULL) {
        parse_error("%s: Unable to expand filename %s\n", func->name, filename);
        return(NULL);
    }

    if (!force && access(fname, F_OK) == 0){
		parse_error("%s: File %s already exists.", func->name, filename);
        return(NULL);
    }

	if ((fp = fopen(fname, "wb")) == NULL) {
        parse_error("%s: Unable to open file: %s\n", func->name, filename);
        return (NULL);
    }

    size[0] = GetX(core);
    size[1] = GetY(core);
    size[2] = GetZ(core);

	zero = newInt(0);

    /*
    ** Verify the size of each suffix plane
    */
    for (i = 0 ; i < 3 ; i++) {
        if (suffix[i] != NULL) {
            for (j = 0 ; j < get_struct_count(suffix[i]) ; j++) {
                get_struct_element(suffix[i], j, &name, &plane);
                suf_size[0] = GetX(plane);
                suf_size[1] = GetY(plane);
                suf_size[2] = GetZ(plane);

                switch (i) {
                case 0:	/* side */
                    if (size[1] != suf_size[1] || 
                        size[2] != suf_size[2] ||
                        suf_size[0] != 1) 
                        error++;
                    break;
                case 1:	/* bottom */
                    if (size[0] != suf_size[0] || 
                        size[2] != suf_size[2] ||
                        suf_size[1] != 1) 
                        error++;
                    break;

                case 2: /* back */
                    if (size[0] != suf_size[0] || 
                        size[1] != suf_size[1] ||
                        suf_size[2] != 1) 
                        error++;
                    break;
                }
                if (error) {
                    parse_error("Suffix plane does not match core size: "
                                "%d [%dx%dx%d] vs %dx%dx%d\n", 
                                name, 
                                suf_size[0], suf_size[1], suf_size[2],
                                size[0], size[1], size[2]);
                    return(NULL);
                }
            }
        }
    }

    /* write the headers */

    sprintf(buf            , "PDS_VERSION = 3.0\n");
    sprintf(buf+strlen(buf), "RECORD_BYTES =       \n");
    sprintf(buf+strlen(buf), "^QUBE =        \n");
    sprintf(buf+strlen(buf), "OBJECT = QUBE\n");
    sprintf(buf+strlen(buf), "    AXES = 3\n");

	if (V_ORG(core) == BSQ) {
		sprintf(buf+strlen(buf), "    AXIS_NAME = (SAMPLE,LINE,BAND)\n");
		rec_len = size[0]*GetNbytes(core);
		if (suffix[0]) rec_len += get_struct_count(suffix[0])*4;
	} else if (V_ORG(core) == BIP) {
		sprintf(buf+strlen(buf), "    AXIS_NAME = (BAND,SAMPLE,LINE)\n");
		rec_len = size[2]*GetNbytes(core);
		if (suffix[2]) rec_len += get_struct_count(suffix[2])*4;
	}
	sprintf(buf+strlen(buf), "    CORE_ITEMS = (%d,%d,%d)\n", V_SIZE(core)[0], V_SIZE(core)[1], V_SIZE(core)[2]);
	

    sprintf(buf+strlen(buf), "    CORE_ITEM_BYTES = %d\n", GetNbytes(core));
    sprintf(buf+strlen(buf), "    CORE_ITEM_TYPE = %s\n", iformat_to_eformat(core));
    sprintf(buf+strlen(buf), "    SUFFIX_BYTES = 4\n");
    sprintf(buf+strlen(buf), "    SUFFIX_ITEMS = (");

	if (V_ORG(core) == BSQ) {
		sprintf(buf+strlen(buf), "%d", 
			suffix[0] == NULL ? 0 : get_struct_count(suffix[0]));
		sprintf(buf+strlen(buf), ",%d", 
			suffix[1] == NULL ? 0 : get_struct_count(suffix[1]));
		sprintf(buf+strlen(buf), ",%d", 
			suffix[2] == NULL ? 0 : get_struct_count(suffix[2]));
	} else if (V_ORG(core) == BIP) {
		sprintf(buf+strlen(buf), "%d", 
			suffix[2] == NULL ? 0 : get_struct_count(suffix[2]));
		sprintf(buf+strlen(buf), ",%d", 
			suffix[0] == NULL ? 0 : get_struct_count(suffix[0]));
		sprintf(buf+strlen(buf), ",%d", 
			suffix[1] == NULL ? 0 : get_struct_count(suffix[1]));
	}
    sprintf(buf+strlen(buf), ")\n");

    for (i = 0 ; i < 3 ; i++) {
        char *suf_prefix;
		int llen;
        if (suffix[i]) {
            switch (i) {
            case 0: suf_prefix = "    SAMPLE_SUFFIX"; break;
            case 1: suf_prefix = "    LINE_SUFFIX"; break;
            case 2: suf_prefix = "    BAND_SUFFIX"; break;
            }

            /* names */
			llen = strlen(buf);
            sprintf(buf+strlen(buf), "%s_NAME = (", suf_prefix);
            for (j = 0 ; j < get_struct_count(suffix[i]) ; j++) {
                if (j) sprintf(buf+strlen(buf), ",");
                get_struct_element(suffix[i], j, &name, &v);		
				if ((strlen(buf) - llen + strlen(name)) > 72) {
					sprintf(buf+strlen(buf), "\n        ");
					llen = strlen(buf)-8;
				}
                sprintf(buf+strlen(buf), "%s", name);
            }
            sprintf(buf+strlen(buf), ")\n");

            /* size */
			llen = strlen(buf);
            sprintf(buf+strlen(buf), "%s_ITEM_BYTES = (", suf_prefix);
            for (j = 0 ; j < get_struct_count(suffix[i]) ; j++) {
                if (j) sprintf(buf+strlen(buf), ",");
                get_struct_element(suffix[i], j, &name, &v);		
				if ((strlen(buf) - llen) > 72) {
					sprintf(buf+strlen(buf), "\n        ");
					llen = strlen(buf)-8;
				}
                sprintf(buf+strlen(buf), "%d", GetNbytes(v));
            }
            sprintf(buf+strlen(buf), ")\n");

            /* type */
			llen = strlen(buf);
            sprintf(buf+strlen(buf), "%s_ITEM_TYPE = (", suf_prefix);
            for (j = 0 ; j < get_struct_count(suffix[i]) ; j++) {
                if (j) sprintf(buf+strlen(buf), ",");
                get_struct_element(suffix[i], j, &name, &v);		
				if ((strlen(buf) - llen) > 72) {
					sprintf(buf+strlen(buf), "\n        ");
					llen = strlen(buf)-8;
				}
                sprintf(buf+strlen(buf), "%s", iformat_to_eformat(v));
            }
            sprintf(buf+strlen(buf), ")\n");
        }
    }

    sprintf(buf+strlen(buf), "END_OBJECT = QUBE\n");
    sprintf(buf+strlen(buf), "END\n");

    p = strstr(buf, "RECORD_BYTES =");
    p += strlen("RECORD_BYTES = ");
    sprintf(lenstr, "%d", rec_len);
    strncpy(p, lenstr, strlen(lenstr));

    lbl_length = ceil(((float)strlen(buf))/rec_len);

    p = strstr(buf, "^QUBE =");
    p += strlen("^QUBE = ");
    sprintf(lenstr, "%d", lbl_length+1);
    strncpy(p, lenstr, strlen(lenstr));

	fwrite(buf, strlen(buf), 1, fp);

	for (i = lbl_length*rec_len - strlen(buf) ; i > 0 ; i-=16) {
		fwrite("                ", min(i, 16), 1, fp);
	}

    nbytes = GetNbytes(core);
	nsuffix[0] = (suffix[0] ? get_struct_count(suffix[0]) : 0);
	nsuffix[1] = (suffix[1] ? get_struct_count(suffix[1]) : 0);
	nsuffix[2] = (suffix[2] ? get_struct_count(suffix[2]) : 0);

    if (V_ORG(core) == BSQ) {
        for (k = 0 ; k < size[2] ; k++) {
            for (j = 0 ; j < size[1] ; j++) {
                pos = (k*size[1]+j)*size[0] * nbytes;
                fwrite((char *)V_DATA(core) + pos, size[0], nbytes, fp);

				/* write sample suffix */
				for (n = 0 ; n < nsuffix[0] ; n++) {
					get_struct_element(suffix[0], n, NULL, &v);
					write_one(v, 0, j, k, fp);
				}
            }
			/* write line suffix */
			for (n = 0 ; n < nsuffix[1] ; n++) {
				get_struct_element(suffix[1], n, NULL, &v);
				write_row_x(v, 0, k, fp, nsuffix[0]);
			}
        }
		/* write band suffix */ 
		for (n = 0 ; n < nsuffix[2] ; n++) {
			get_struct_element(suffix[2], n, NULL, &v);
            /* write band suffix along with band-sample intersect */
			write_plane(v, V_ORG(core), 2, fp, nsuffix[0], nsuffix[1]);
            /* write band-line intersect */
            for (k = 0; k < nsuffix[1];  k++){
                write_suffix_zeros(fp, size[0]+nsuffix[0]);
            }
		}
    } else if (V_ORG(core) == BIP) {
        for (k = 0 ; k < size[1] ; k++) {		/* y axis */
            for (j = 0 ; j < size[0] ; j++) {	/* z axis */
                pos = (k*size[0]+j)*size[2] * nbytes;
                fwrite((char *)V_DATA(core) + pos, size[2], nbytes, fp);

				for (n = 0 ; n < nsuffix[2] ; n++) {
					get_struct_element(suffix[2], n, NULL, &v);
					write_one(v, j, k, 0, fp);
				}
            }
			for (n = 0 ; n < nsuffix[0] ; n++) {
				get_struct_element(suffix[0], n, NULL, &v);
                write_row_z(v, 0, k, fp, nsuffix[2]);
			}
        }

		for (n = 0 ; n < nsuffix[1] ; n++) {
			get_struct_element(suffix[1], n, NULL, &v);
			write_plane(v, V_ORG(core), 1, fp, nsuffix[2], nsuffix[0]);
            for(k = 0; k < nsuffix[0]; k++){
                write_suffix_zeros(fp, size[2]+nsuffix[2]);
            }
		}

	}
    fclose(fp);
    return(NULL);
}

static void
write_suffix_zeros(FILE *fp, int nel)
{
    int i;
    int zero = 0;
    
    for(i = 0; i < nel; i++){
        fwrite(&zero, 4, 1, fp);
    }
}

/**
*** Each suffix element has to be aligned in a 4-word frame.
**/

write_one(Var *v, int x, int y, int z, FILE *fp) 
{ 
    int  pos = cpos(x, y, z, v);
    int nbytes = GetNbytes(v);
    int zero = 0;

    if (nbytes < 4) {
        fwrite(&zero, 4-nbytes, 1, fp);
    }
    fwrite((char *)V_DATA(v) + pos*nbytes, 1, nbytes, fp);
} 

write_row_x(Var *v, int y, int z, FILE *fp, int corner1)
{
    int i;
    int x = GetX(v);
    int zero = 0;

    for (i = 0 ; i < x ; i++) {
        write_one(v, i, y, z, fp);
    }
	for (i = 0 ; i < corner1 ; i++) {
        fwrite(&zero, 4, 1, fp);
	}
}
write_row_y(Var *v, int x, int z, FILE *fp, int corner1)
{
    int i;
    int y = GetY(v);
    int zero = 0;

    for (i = 0 ; i < y ; i++) {
        write_one(v, x, i, z, fp);
    }
	for (i = 0 ; i < corner1 ; i++) {
        fwrite(&zero, 4, 1, fp);
	}
}

write_row_z(Var *v, int x, int y, FILE *fp, int corner1)
{
    int i;
    int z = GetZ(v);
    int zero = 0;

    for (i = 0 ; i < z ; i++) {
        write_one(v, x, y, i, fp);
    }
	for (i = 0 ; i < corner1 ; i++) {
        fwrite(&zero, 4, 1, fp);
	}
}

write_plane(Var *v, int org, int plane, FILE *fp, int corner1, int corner2) 
{
    int i;
    int x = GetX(v);
    int y = GetY(v);
    int z = GetZ(v);

	/*
	** corner1 : number of suffix elements on fastest changing axis
	** corner2 : number of suffix elements on slowest changing axis
	** Doesn't currently put in values for corner2.
	*/
    
    if (plane == 0) {
        if (org == BIL) {		/* write rows of Y */
            for (i = 0 ; i < z ; i++) {
                write_row_y(v, 0, i, fp, corner1);
            }
        } else if (org == BIP) {	/* write rows of Z */
            for (i = 0 ; i < y ; i++) {
                write_row_z(v, 0, i, fp, corner1);
            }
        }
    }
    if (plane == 1) {
        if (org == BIL) {		/* write rows of x */
            for (i = 0 ; i < z ; i++) {
                write_row_x(v, 0, i, fp, corner1);
            }
        } else if (org == BIP) {	/* write rows of z */
            for (i = 0 ; i < x ; i++) {
                write_row_z(v, i, 0, fp, corner1);
            }
        }
    }
    if (plane == 2) {
        for (i = 0 ; i <y ; i++) {	/* write rows of X */
            write_row_x(v, i, 0, fp, corner1);
        }
    }
}

char *
iformat_to_eformat(Var *obj) 
{
    static char buf[256];
#ifdef WORDS_BIGENDIAN
    switch (V_FORMAT(obj)) {
    case BYTE: sprintf(buf,"MSB_UNSIGNED_INTEGER"); break;
    case SHORT: sprintf(buf,"MSB_INTEGER"); break;
    case INT: sprintf(buf,"MSB_INTEGER"); break;
    case FLOAT: sprintf(buf,"IEEE_REAL"); break;
    case DOUBLE: sprintf(buf,"IEEE_REAL"); break;
    }
#else /* little-endian */
    switch (V_FORMAT(obj)) {
    case BYTE: sprintf(buf,"PC_UNSIGNED_INTEGER"); break;
    case SHORT: sprintf(buf,"PC_INTEGER"); break;
    case INT: sprintf(buf,"PC_INTEGER"); break;
    case FLOAT: sprintf(buf,"PC_REAL"); break;
    case DOUBLE: sprintf(buf,"PC_REAL"); break;
    }
#endif /* WORDS_BIGENDIAN */
    return(buf);
}

static const char *
get_type_string(int type)
{
    switch(type){
    case BYTE:   return "BYTE";
    case SHORT:  return "SHORT";
    case INT:    return "INT";
    case FLOAT:  return "FLOAT";
    case DOUBLE: return "DOUBLE";
    }
    return "UNKNOWN";
}

static const char *
get_org_string(int org)
{
    switch(org){
    case BSQ: return "BSQ";
    case BIP: return "BIP";
    case BIL: return "BIL";
    }
    return "UNKNOWN";
}


/***********************************************************************/
/* ISIS Cube Writer Code Follows                                       */
/***********************************************************************/

#ifdef HAVE_LIBISIS

static int
convert_data(int from_type, void *from_data, int to_type, void *to_data, int nelements)
{
    int i;
    
    unsigned char   *from_byte_data   = (unsigned char *)   from_data;
    short  *from_short_data  = (short *)  from_data;
    int    *from_int_data    = (int *)    from_data;
    float  *from_float_data  = (float *)  from_data;
    double *from_double_data = (double *) from_data;

    unsigned char   *to_byte_data     = (unsigned char *)   to_data;
    short  *to_short_data    = (short *)  to_data;
    int    *to_int_data      = (int *)    to_data;
    float  *to_float_data    = (float *)  to_data;
    double *to_double_data   = (double *) to_data;
    

    for(i = 0; i < nelements; i++){
        switch(from_type){
        case BYTE:
            switch(to_type){
            case BYTE:   to_byte_data[i]   = from_byte_data[i];   break;
            case SHORT:  to_short_data[i]  = from_byte_data[i];  break;
            case INT:    to_int_data[i]    = from_byte_data[i];    break;
            case FLOAT:  to_float_data[i]  = saturate_float(from_byte_data[i]);  break;
            case DOUBLE: to_double_data[i] = saturate_double(from_byte_data[i]); break;
            default: return 0;
            }
            break;
        case SHORT:
            switch(to_type){
            case BYTE:   to_byte_data[i]   = saturate_byte(from_short_data[i]);   break;
            case SHORT:  to_short_data[i]  = from_short_data[i];  break;
            case INT:    to_int_data[i]    = from_short_data[i];    break;
            case FLOAT:  to_float_data[i]  = from_short_data[i];  break;
            case DOUBLE: to_double_data[i] = from_short_data[i]; break;
            default: return 0;
            }
            break;
        case INT:
            switch(to_type){
            case BYTE:   to_byte_data[i]   = saturate_byte(from_int_data[i]);   break;
            case SHORT:  to_short_data[i]  = saturate_short(from_int_data[i]);   break;
            case INT:    to_int_data[i]    = from_int_data[i];    break;
            case FLOAT:  to_float_data[i]  = saturate_float(from_int_data[i]);  break;
            case DOUBLE: to_double_data[i] = saturate_double(from_int_data[i]); break;
            default: return 0;
            }
            break;
        case FLOAT:
            switch(to_type){
            case BYTE:   to_byte_data[i]   = saturate_byte(from_float_data[i]);   break;
            case SHORT:  to_short_data[i]  = saturate_short(from_float_data[i]);  break;
            case INT:    to_int_data[i]    = saturate_int(from_float_data[i]);    break;
            case FLOAT:  to_float_data[i]  = saturate_float(from_float_data[i]);  break;
            case DOUBLE: to_double_data[i] = saturate_double(from_float_data[i]); break;
            default: return 0;
            }
            break;
        case DOUBLE:
            switch(to_type){
            case BYTE:   to_byte_data[i]   = saturate_byte(from_double_data[i]);   break;
            case SHORT:  to_short_data[i]  = saturate_short(from_double_data[i]);  break;
            case INT:    to_int_data[i]    = saturate_int(from_double_data[i]);    break;
            case FLOAT:  to_float_data[i]  = saturate_float(from_double_data[i]);  break;
            case DOUBLE: to_double_data[i] = saturate_double(from_double_data[i]); break;
            default: return 0;
            }
            break;
        default: return 0;
        }
    }

    return 1;
}

/**
 ** get_suffix_buff_size()
 **
 ** Returns the suffix buffer sizes required for using
 ** libisis/q/q_pio_qube.c. q_pio_qube gives the size
 ** calculations in it.
 **
 ** The returned sizes are in items not bytes.
 **/
static void
get_suffix_buff_sizes(
    int core_items[3],
    int suffix_items[3],
    int suffix_item_sizes[3]
    )
{
    suffix_item_sizes[0] =
        suffix_items[0]*core_items[1]*core_items[2] +
        suffix_items[0]*core_items[1]*suffix_items[2];
    
    suffix_item_sizes[1] =
        core_items[0]*suffix_items[1]*core_items[2] +
        suffix_items[0]*suffix_items[1]*(core_items[2]+suffix_items[2]);
    
    suffix_item_sizes[2] =
        core_items[0]*core_items[1]*suffix_items[2];
}

/**
 ** get_org_from_band_names()
 **
 ** Determines davinci-org based on the band-names.
 ** Assumes all three band-names are populated.
 ** Return value is 0 on failure and non-zero on success.
 **/
static int
get_org_from_axis_names(
    char *axis_names[3],
    int *org
    )
{
    int rc = 0;
    
    if (strcasecmp(axis_names[0], "sample") == 0){
        if      (strcasecmp(axis_names[1], "line") == 0){ *org = BSQ; rc = 1; }
        else if (strcasecmp(axis_names[1], "band") == 0){ *org = BIL; rc = 1; }
    }
    else if (strcasecmp(axis_names[0], "band") == 0){ *org = BIP; rc = 1; }

    return rc;
}


/**
 ** Returns the ISIS encoded order based on the davinci ORG
 **/
static int
get_isis_order_for_org(
    int org,
    int *order
    )
{
    switch(org){
    case BSQ: *order = BSQ_STORAGE; break;
    case BIP: *order = BIP_STORAGE; break;
    case BIL: *order = BIL_STORAGE; break;
    default:
        return 0;
    }

    return 1;
}

/**
 ** Various structures used in extraction of core/suffix-data
 ** specifications.
 **/

typedef struct {
    char    *name;                    /* name of suffix plane */
    int      org;                     /* org of the data */
    char    *item_type_str;           /* type-string of suffix plane (from label) */
    iom_edf  item_type_edf;           /* coded type of suffix plane */
    int      item_type;               /* davinci type corresponding to item_type_edf */
    int      item_bytes;              /* byte-size of each element of the plane */
    float    scale[2];                /* core base & multiplier */
    char    *unit;                    /* unit string associated with the suffix plane */
    Var     *data_var;                /* Var * pointing to suffix plane data - only
                                         filled by get_data_attrs_from_data */
} SfxDataSpecs;

typedef struct {
    int      axes;                    /* number of qube axes */
    char    *axis_name[3];            /* names of axes (org-order) */
    int      org;                     /* org of data (derived from axis_name) */
    int      core_items[3];           /* items in each qube dimension (org-order) */
    int      core_item_bytes;         /* byte-size of each qube element */
    char    *core_item_type_str;      /* external data-type-name of core-data */
    iom_edf  core_item_type_edf;      /* coded type of core items */
    int      core_item_type;          /* internal representation of item-type */
    int      suffix_items[3];         /* suffixes in each dimension (org-order) */
    int      suffix_bytes;            /* byte-size of suffix data */
    SfxDataSpecs **suffix_attr[3];  /* array of suffix data attrs (org-order) */
    char   **suffix_plane_order[3];   /* order in which suffix planes appear (org-order) */
    float    core_scale[2];           /* core base & multiplier */
    Var     *core_data_var;           /* Var * pointing to qube.data - only filled by
                                         get_data_attrs_from_data */
} CoreDataSpecs;

static CoreDataSpecs *
new_CoreDataSpecs()
{
    CoreDataSpecs *a = (CoreDataSpecs *)calloc(1, sizeof(CoreDataSpecs));
    
    if (a != NULL){
        /* set core multiplier to 1 */
        a->core_scale[1] = 1.0;
    }
    
    return a;
}

static SfxDataSpecs *
new_SfxDataSpecs()
{
    SfxDataSpecs *s = (SfxDataSpecs *)calloc(1, sizeof(SfxDataSpecs));

    if (s != NULL){
        /* set core multiplier to 1 */
        s->scale[1] = 1.0;
    }

    return s;
}

static void
free_SfxDataSpecs(SfxDataSpecs **a)
{
    free((*a)->name);
    free((*a)->item_type_str);
    free((*a)->unit);
    free(*a);
    *a = NULL;
}

static void
free_CoreDataSpecs(CoreDataSpecs **a)
{
    int i, k;
    
    free((*a)->axis_name);
    free((*a)->core_item_type_str);

    for(k = 0; k < 3; k++){
        for(i = 0; i < (*a)->suffix_items[k]; i++){
            free_SfxDataSpecs(&(*a)->suffix_attr[k][i]);
            free((*a)->suffix_plane_order[k][i]);
        }
        free((*a)->suffix_attr[k]);
    }

    free(*a);
    *a = NULL;
}

/**
 ** UNUSED
 **
 ** Comparator function, can be used to sort an array
 ** of SfxDataSpecs pointers by name.
 **/
static int
compare_SfxDataSpecs(const void *v1, const void *v2)
{
    SfxDataSpecs *a = (SfxDataSpecs *)v1;
    SfxDataSpecs *b = (SfxDataSpecs *)v2;

    return strcmp(a->name, b->name);
}

/**
 ** Check to see if the given variable has the specified
 ** (x,y,z) dimensions.
 **/
static int
has_dim(Var *v, int x, int y, int z)
{
    if (V_TYPE(v) == ID_VAL){
        return (GetX(v) == x && GetY(v) == y && GetZ(v) == z);
    }
    return 0;
}

/**
 ** Check to see if the given string variable has the given
 ** number of rows.
 **/
static int
has_n_rows(Var *v, int n)
{
    switch(V_TYPE(v)){
    case ID_STRING:
        return (n == 1);
    case ID_TEXT:
        return (V_TEXT(v).Row == n);
    }

    return 0;
}

/**
 ** Returns the maximum of the lengths of the input strings.
 **/
static int
get_max_strlen(char **strs, int n)
{
    int len = 0;
    int i;

    for(i = 0; i < n; i++){
        len = MAX(len, strlen(strs[i]));
    }

    return len;
}

/**
 ** UNUSED
 **
 ** Generic byte-swap function.
 **/
static void
swap_data(void *data, int nitems, int item_size_bytes)
{
    int i, j, half_item_size_bytes;
    unsigned char *d, *p, t;

    d = (unsigned char *)data;
    half_item_size_bytes = item_size_bytes / 2;
    
    for(i = 0; i < nitems; i++){
        p = d + item_size_bytes * i;
        
        for(j = 0; j < half_item_size_bytes; j++){
            t = p[j];
            p[j] = p[item_size_bytes-1-j];
            p[item_size_bytes-1-j] = t;
        }
    }
}

/**
 ** Helper function that retrieve specifications of core-data
 ** from the structural hierarchy obtained by load_pds().
 ** This specification is used in writing of ISIS cube file.
 **/
static CoreDataSpecs *
get_data_attrs_from_lbl(
    Var *qube           /* struct.qube / struct.spectral_qube */
    )
{
    Var *v = NULL;
    Var *core_data_var = NULL;
    Var *sfx_data_var = NULL;
    Var *sfx_data = NULL;
    CoreDataSpecs *a;
    char buff[128];
    int i, k, kk, j;
    int rc;

    a = new_CoreDataSpecs();
    
    /* get qube.axes */
    if (find_struct(qube, "axes", &v) < 0 || (a->axes = V_INT(v)) != 3){
        parse_error("Expecting qube.axes = 3, got %d instead\n", a->axes);
        free_CoreDataSpecs(&a); return NULL;
    }
    
    /* get qube.axis_name */
    if (find_struct(qube, "axis_name", &v) < 0 ||
        V_TYPE(v) != ID_TEXT || !has_n_rows(v, a->axes)){
        parse_error("Expecting qube.axis_name as text array of %d lines\n", a->axes);
        free_CoreDataSpecs(&a); return NULL;
    }
    for(i = 0; i < a->axes; i++){
        a->axis_name[i] = strdup(V_TEXT(v).text[i]);
    }

    /* ascertain the qube's org based on axis_name */
    rc = get_org_from_axis_names(a->axis_name, &a->org);
    if (!rc){
        parse_error("Cannot determine core's org based on (%s,%s,%s)\n",
                    a->axis_name[0], a->axis_name[1], a->axis_name[2]);
        free_CoreDataSpecs(&a); return NULL;
    }
    
    /* get qube.core_items */
    if (find_struct(qube, "core_items", &v) < 0 || !has_dim(v, a->axes, 1, 1)){
        parse_error("Expecting qube.core_items of dim: %dx%dx%d\n", a->axes, 1, 1);
        free_CoreDataSpecs(&a); return NULL;
    }
    for(i = 0; i < a->axes; i++){
        a->core_items[i] = extract_int(v, cpos(i, 0, 0, v));
    }
    
    /* get qube.core_item_bytes */
    if (find_struct(qube, "core_item_bytes", &v) < 0){
        parse_error("Expecting qube.core_item_bytes.\n");
        free_CoreDataSpecs(&a); return NULL;
    }
    a->core_item_bytes = V_INT(v);

    /* get qube.core_item_type */
    if (find_struct(qube, "core_item_type", &v) < 0 || V_TYPE(v) != ID_STRING){
        parse_error("Expecting qube.core_item_type as a string.\n");
        free_CoreDataSpecs(&a); return NULL;
    }
    a->core_item_type_str = strdup(V_STRING(v));
    ucase(a->core_item_type_str);

    sprintf(buff, "%d", a->core_item_bytes);
    a->core_item_type_edf = iom_ConvertISISType(a->core_item_type_str, NULL, buff);
    a->core_item_type = ihfmt2vfmt(iom_Eformat2Iformat(a->core_item_type_edf));

    /* get qube.data */
    if (find_struct(qube, "data", &a->core_data_var) < 0 ||
        !has_dim(a->core_data_var, a->core_items[iom_orders[a->org][0]],
                 a->core_items[iom_orders[a->org][1]],
                 a->core_items[iom_orders[a->org][2]])){
        parse_error("Expecting qube.data of dim: %dx%dx%d\n",
                    a->core_items[iom_orders[a->org][0]],
                    a->core_items[iom_orders[a->org][1]],
                    a->core_items[iom_orders[a->org][2]]);
        free_CoreDataSpecs(&a); return NULL;
    }

    /* get qube.core_base */
    if (find_struct(qube, "core_base", &v) >= 0){
        a->core_scale[0] = extract_float(v, 0);
    }
    
    /* get qube.core_multiplier */
    if (find_struct(qube, "core_multiplier", &v) >= 0){
        a->core_scale[1] = extract_float(v, 0);
    }

    /* get qube.suffix_items */
    if (find_struct(qube, "suffix_items", &v) < 0 || !has_dim(v, a->axes, 1, 1)){
        parse_error("Expecting qube.suffix_items of dim: %dx%dx%d\n", a->axes, 1, 1);
        free_CoreDataSpecs(&a); return NULL;
    }
    for(i = 0; i < a->axes; i++){
        a->suffix_items[i] = extract_int(v, cpos(i, 0, 0, v));
    }

    /* get qube.suffix_bytes */
    if (find_struct(qube, "suffix_bytes", &v) < 0 || (a->suffix_bytes = V_INT(v)) != 4){
        parse_error("Expecting qube.suffix_bytes = 4.\n");
        free_CoreDataSpecs(&a); return NULL;
    }

    /* get qube.suffix_data */
    find_struct(qube, "suffix_data", &sfx_data_var);

    /* get suffix items whenever available */
    for(k = 0; k < a->axes; k++){
        
        /* get axis-index in org-order */
        kk = iom_orders[a->org][k];
        
        /* get qube.xxx_suffix_name */
        sprintf(buff, "%s_suffix_name", a->axis_name[kk]); lcase(buff);
        if (find_struct(qube, buff, &v) < 0){
            /* suffix data does not exist for this axis, skip ahead */
            continue;
        }
        if (!has_n_rows(v, a->suffix_items[kk])){
            parse_error("Expecting qube.%s with %d lines\n", buff, a->suffix_items[kk]);
            free_CoreDataSpecs(&a); return NULL;
        }

        /* if we reached here then we have at least one suffix plane that
           we need to process */
        a->suffix_attr[kk] =
            (SfxDataSpecs **)calloc(a->suffix_items[kk], sizeof(SfxDataSpecs *));

        /* save the order in which the suffix planes appear, and initialize the
           SfxDataSpecs for each of the suffix planes */
        a->suffix_plane_order[kk] =
            (char **)calloc(a->suffix_items[kk], sizeof(char *));
        for(i = 0; i < a->suffix_items[kk]; i++){
            a->suffix_plane_order[kk][i] =
                strdup(V_TYPE(v) == ID_STRING? V_STRING(v): V_TEXT(v).text[i]);
            lcase(a->suffix_plane_order[kk][i]);
            a->suffix_attr[kk][i] = new_SfxDataSpecs();
            a->suffix_attr[kk][i]->name = strdup(a->suffix_plane_order[kk][i]);
        }
        
        /* get qube.xxx_suffix_item_bytes */
        sprintf(buff, "%s_suffix_item_bytes", a->axis_name[kk]); lcase(buff);
        if (find_struct(qube, buff, &v) < 0 || !has_dim(v, a->suffix_items[kk], 1, 1)){
            parse_error("Expecting qube.%s of dim: %dx%dx%d\n", buff,
                    a->suffix_items[kk], 1, 1);
            free_CoreDataSpecs(&a); return NULL;
        }
        for(i = 0; i < a->suffix_items[kk]; i++){
            a->suffix_attr[kk][i]->item_bytes = extract_int(v, cpos(i, 0, 0, v));
        }
        
        /* get qube.xxx_suffix_item_type */
        sprintf(buff, "%s_suffix_item_type", a->axis_name[kk]); lcase(buff);
        if (find_struct(qube, buff, &v) < 0 || !has_n_rows(v, a->suffix_items[kk])){
            parse_error("Expecting qube.%s with %d lines\n", buff,
                        a->suffix_items[kk]);
            free_CoreDataSpecs(&a); return NULL;
        }
        for(i = 0; i < a->suffix_items[kk]; i++){
            a->suffix_attr[kk][i]->item_type_str =
                strdup(V_TYPE(v) == ID_STRING? V_STRING(v): V_TEXT(v).text[i]);

            /* convert string representation of type to internal representation */
            sprintf(buff, "%d", a->suffix_attr[kk][i]->item_bytes); lcase(buff);
            a->suffix_attr[kk][i]->item_type_edf =
                iom_ConvertISISType(a->suffix_attr[kk][i]->item_type_str, NULL, buff);
            a->suffix_attr[kk][i]->item_type =
                ihfmt2vfmt(iom_Eformat2Iformat(a->suffix_attr[kk][i]->item_type_edf));

            /* assume all suffix planes have the same org as the core */
            a->suffix_attr[kk][i]->org = a->org;
        }

        /* get qube.xxx_suffix_unit */
        sprintf(buff, "%s_suffix_unit", a->axis_name[kk]); lcase(buff);
        if (find_struct(qube, buff, &v) < 0){
            for(i = 0; i < a->suffix_items[kk]; i++){
                a->suffix_attr[kk][i]->unit = strdup("DIMENSIONLESS");
            }
        }
        else if (!has_n_rows(v, a->suffix_items[kk])){
            parse_error("Expecting qube.%s with %d lines\n",
                        buff, a->suffix_items[kk]);
            free_CoreDataSpecs(&a); return NULL;
        }
        else {
            for(i = 0; i < a->suffix_items[kk]; i++){
                a->suffix_attr[kk][i]->unit =
                    strdup(V_TYPE(v) == ID_STRING? V_STRING(v): V_TEXT(v).text[i]);
            }
        }
        

        /* get qube.xxx_suffix_base */
        sprintf(buff, "%s_suffix_base", a->axis_name[kk]); lcase(buff);
        if (find_struct(qube, buff, &v) < 0){
            for(i = 0; i < a->suffix_items[kk]; i++){
                a->suffix_attr[kk][i]->scale[0] = 0.0;
            }
        }
        else if (!has_dim(v, a->suffix_items[kk], 1, 1)){
            parse_error("Expecting qube.%s with dim: %dx%dx%d\n",
                        buff, a->suffix_items[kk], 1, 1);
            free_CoreDataSpecs(&a); return NULL;
        }
        else {
            for(i = 0; i < a->suffix_items[kk]; i++){
                a->suffix_attr[kk][i]->scale[0] = extract_float(v, cpos(i, 0, 0, v));
            }
        }

        /* get qube.xxx_suffix_multiplier */
        sprintf(buff, "%s_suffix_multiplier", a->axis_name[kk]); lcase(buff);
        if (find_struct(qube, buff, &v) < 0){
            for(i = 0; i < a->suffix_items[kk]; i++){
                a->suffix_attr[kk][i]->scale[1] = 1.0;
            }
        }
        else if (!has_dim(v, a->suffix_items[kk], 1, 1)){
            parse_error("Expecting qube.%s with dim: %dx%dx%d\n",
                        buff, a->suffix_items[kk], 1, 1);
            free_CoreDataSpecs(&a); return NULL;
        }
        else {
            for(i = 0; i < a->suffix_items[kk]; i++){
                a->suffix_attr[kk][i]->scale[1] = extract_float(v, cpos(i, 0, 0, v));
            }
        }

        
        /* find the variable associated with the suffix planes */
        if (sfx_data_var == NULL){
            parse_error("Expecting qube.suffix_data\n");
            free_CoreDataSpecs(&a); return NULL;
        }
        
        strcpy(buff, a->axis_name[kk]); lcase(buff);
        if (find_struct(sfx_data_var, buff, &sfx_data) < 0){
            parse_error("Expecting qube.suffix_data.%s\n", buff);
            free_CoreDataSpecs(&a); return NULL;
        }
        
        for(i = 0; i < a->suffix_items[kk]; i++){
            int suffix_plane_size[3];

            /**
             ** Determine the expected size of the suffix plane.
             ** CAUTION: I am unsure about this. May be the suffix-planes
             ** all come around in SAMPLExLINE configuration. If that is
             ** the case, then this code has to be modified.
             **/
            for(j = 0; j < 3; j++){
                suffix_plane_size[j] = a->core_items[iom_orders[a->org][j]];
            }
            suffix_plane_size[k] = 1;

            if (find_struct(sfx_data, a->suffix_attr[kk][i]->name,
                            &a->suffix_attr[kk][i]->data_var) < 0 ||
                !has_dim(a->suffix_attr[kk][i]->data_var, suffix_plane_size[0],
                         suffix_plane_size[1], suffix_plane_size[2])){
                
                parse_error("Expecting qube.suffix_data.%s.%s of dim: %dx%dx%d\n",
                            buff, a->suffix_attr[kk][i]->name, suffix_plane_size[0],
                            suffix_plane_size[1], suffix_plane_size[2]);
                free_CoreDataSpecs(&a); return NULL;
            }
        }

        /* sort the suffix_attr values
        qsort(a->suffix_attr[kk], a->suffix_items[kk],
              sizeof(SfxDataSpecs *),
              compare_SfxDataSpecs); */
    }
    
    
    return a;
}

/**
 ** UNUSED - TO BE DELETED
 **
 ** Helper function that retrieve specifications of core-data
 ** only from the data & suffix_data portions of structure
 ** returned vis load_pds().
 ** This specification is used in writing of ISIS cube file.
 **/
static CoreDataSpecs *
get_data_attrs_from_data(Var *qube)
{
    CoreDataSpecs *a = NULL;
    Var *suffix_axis_data, *v;
    int i, k, kk;
    char buff[1024];
    Var *data = NULL;
    Var *suffix_data = NULL;

    /* find qube.data - which is required */
    if (find_struct(qube, "data", &data) < 0){
        parse_error("Expected qube.data.\n");
        return NULL;
    }

    /* find qube.suffix_data - which is optional */
    find_struct(qube, "suffix_data", &suffix_data);
    
    a = new_CoreDataSpecs();

    /* davinci objects are 3D by definition */
    a->axes = 3;

    /* davinci objects have org attached to them, we can use this
       org value to determine the order of axis_name[] */
    a->org = V_ORG(data);

    /* assign core-name based on org definition */
    for(i = 0; i < a->axes; i++){
        a->axis_name[i] = strdup(axis_names[iom_orders[a->org][i]]);
    }

    /* get core_items of the data */
    for(i = 0; i < a->axes; i++){
        a->core_items[i] = V_SIZE(data)[i];
    }

    a->core_item_type = V_FORMAT(data);
    a->core_item_bytes = NBYTES(a->core_item_type);

    /* hold on to the data variable pointer */
    a->core_data_var = data;

    /* suffix bytes is alwarys 4 */
    a->suffix_bytes = 4;

    /* populate suffix data if there is any */
    if (suffix_data != NULL){
        for(k = 0; k < a->axes; k++){

            /* get axis-index in org-order */
            kk = iom_orders[a->org][k];
            
            sprintf(buff, "%s", a->axis_name[kk]);
            if (find_struct(suffix_data, buff, &suffix_axis_data) >= 0){
                a->suffix_items[kk] =
                    get_struct_names(suffix_axis_data, &a->suffix_plane_order[kk], NULL);
                
                a->suffix_attr[kk] = (SfxDataSpecs **)
                    calloc(a->suffix_items[kk], sizeof(SfxDataSpecs **));
                
                for(i = 0; i < a->suffix_items[kk]; i++){
                    v = NULL;
                    find_struct(suffix_axis_data, a->suffix_plane_order[kk][i], &v);
                    assert(v != NULL);

                    if (V_FORMAT(v) == ID_VAL){
                        a->suffix_attr[kk][i] = new_SfxDataSpecs();
                        a->suffix_attr[kk][i]->name = strdup(a->suffix_plane_order[kk][i]);
                        a->suffix_attr[kk][i]->item_type = V_FORMAT(v);
                        a->suffix_attr[kk][i]->item_bytes = NBYTES(V_FORMAT(v));
                        a->suffix_attr[kk][i]->org = V_ORG(v);
                        a->suffix_attr[kk][i]->data_var = v;
                    }
                    else {
                        parse_error("Unhandled type of element in suffix_data: %s\n",
                                    a->suffix_plane_order[kk][i]);
                        free_CoreDataSpecs(&a); return NULL;
                    }
                }
            }
            
            /* sort the suffix_attr values 
            qsort(a->suffix_attr[kk], a->suffix_items[kk],
                  sizeof(SfxDataSpecs *),
                  compare_SfxDataSpecs); */
        }
    }
    
    return a;
}


/**
 ** extract_data_band_as_bsq()
 **
 ** Extracts one band of data from the given davinci variable
 ** and stores it in the buffer specified by buff in BSQ order.
 ** The buffer "buff" must be pre-allocated to hold one
 ** sample-line plane of data in V_FORMAT(v).
 **/
static int
extract_data_band_as_bsq(
    Var   *v,
    int    band_no,
    void  *buff
    )
{
    int      x, y, pos, idx;
    unsigned char *byte_buff = (char *)buff;
    short   *short_buff      = (short *)buff;
    int     *int_buff        = (int *)buff;
    float   *float_buff      = (float *)buff;
    double  *double_buff     = (double *)buff;

    idx = 0; /* index into the output data array */
    for(y = 0; y < GetY(v); y++){
        for(x = 0; x < GetX(v); x++){
            
            pos = cpos(x, y, band_no, v);
            
            switch(V_FORMAT(v)){
            case BYTE:
                byte_buff[idx++]   = (unsigned char)extract_int(v, pos); break;
            case SHORT:
                short_buff[idx++]  = (short)extract_int(v, pos); break;
            case INT:
                int_buff[idx++]    = extract_int(v, pos); break;
            case FLOAT:
                float_buff[idx++]  = extract_float(v, pos); break;
            case DOUBLE:
                double_buff[idx++] = extract_double(v, pos); break;
            default:
                fprintf(stderr, "extract_data_band_as_bsq passed an "
                        "unhandled data type at %s:%d\n", __FILE__, __LINE__);
                return 0;
            }
        }
    }

    return 1;
}


/**
 ** UNUSED
 **
 ** convert_data_to_ext_fmt()
 **
 ** Converts the numeric binary data from in_buff to the external (binary)
 ** data format in out_buff. This may involve byte-swapping of data.
 ** Both buffers must be pre-allocated and in_buff and out_buff must not
 ** be the same memory space.
 **/
static int
convert_data_to_ext_fmt(
    char    *in_buff,   /* input buffer in on of davinci internal data formats */
    int      in_fmt,    /* davinci internal data format of the input buffer */
    char    *out_buff,  /* output buffer in one of the external data formats */
    iom_edf  out_fmt,   /* external data format to use for the output buffer */
    int      n          /* number of items */
    )
{
    int i;
    int dv_out_fmt = ihfmt2vfmt(iom_Eformat2Iformat(out_fmt));

    if (convert_data(in_fmt, in_buff, dv_out_fmt, out_buff, n) < 0){
        parse_error("convert_data_to_ext_fmt\n");
    }
    return (iom_byte_swap_data(out_buff, n, out_fmt) >= 1);
}


/**
 ** concat_column_data()
 **
 ** Concat columar data as columns to the specified buffer.
 ** Note that the actual number of rows in a column within buff
 ** may be larger than what we are concating.
 **/
static int
concat_column_data(
    char   *buff,              /* buff accumulator of data */
    int     rec_len,           /* total width of the buffer */
    int     rows,              /* total number of rows in the buffer */
    char   *data,              /* column data to be concated to buffer */
    int     data_item_bytes,   /* size of individual items of data */
    char  **col_pos            /* should be NULL on first call, reuse in rest */ 
    )
{
    int   i;
    char *p = (*col_pos);

    if (p == NULL){
        /* first call to concat: init *col_pos */
        p = *col_pos = buff;
    }
    
    for(i = 0; i < rows; i++){
        memcpy(p, data, data_item_bytes);
        p += rec_len;
        data += data_item_bytes;
    }

    *col_pos += data_item_bytes;

    return 1;
}


/**
 ** concat_row_data()
 **
 ** Concat row data as rows to the specified buffer.
 ** Note that the actual number of columns in a row within buff
 ** may be larger than what we are concating.
 **/
static int
concat_row_data(
    char   *buff,              /* buff accumulator of data */
    int     rec_len,           /* total width of the buffer */
    int     columns,           /* total number of columns in the buffer */
    char   *data,              /* row data to be concated to buffer */
    int     data_item_bytes,   /* size of individual items of data */
    char  **row_pos            /* should be NULL on first call, reuse in rest */ 
    )
{
    int   i;
    char *p = (*row_pos);

    if (p == NULL){
        /* first call to concat: init *col_pos */
        p = *row_pos = buff;
    }
    
    for(i = 0; i < columns; i++){
        memcpy(p, data, data_item_bytes);
        p += data_item_bytes;
        data += data_item_bytes;
    }

    *row_pos += rec_len; /*  - columns * data_item_bytes); */

    return 1;
}


/**
 ** extract_suffix_data_for_band()
 **
 ** Extracts side- and bottom-planes for a given band into
 ** the specified side_buff and bot_buff respectively.
 ** The variable core_specs contains the core data specs
 ** as loaded by get_data_attr_from_lbl.
 **/
static int
extract_suffix_data_for_band(
    CoreDataSpecs *core_specs,
    int            band_no,
    void          *side_buff,
    void          *bot_buff
    )
{
    int   sample_idx = iom_orders[core_specs->org][0];
    int   line_idx  = iom_orders[core_specs->org][1];
    SfxDataSpecs *s;
    int   side_items, bot_items;
    char *buff1 = NULL, *buff2 = NULL;
    char *col_pos = NULL, *row_pos = NULL;
    int   side_buff_sz = 0;
    int   bot_buff_sz = 0;
    int   i;

    side_items = core_specs->suffix_items[sample_idx];
    bot_items = core_specs->suffix_items[line_idx];
    side_buff_sz = side_items * core_specs->core_items[line_idx];
    bot_buff_sz = bot_items * core_specs->core_items[sample_idx];

    /* NOTE: suffix_bytes are always set to 4 and none of the data
       stored in the suffixes is larger than 4 bytes per item */
    buff1 = (char *)calloc(side_buff_sz, core_specs->suffix_bytes);
    buff2 = (char *)calloc(side_buff_sz, core_specs->suffix_bytes);
    if (buff1 == NULL || buff2 == NULL){
        parse_error("extract_suffix_data_for_band: Mem alloc error.\n");
        free(buff1); free(buff2);
        return 0;
    }

    /* process side planes */
    col_pos = NULL; /* work-var for concat_column_data */
    for(i = 0; i < side_items; i++){
        s = core_specs->suffix_attr[sample_idx][i];

        /* extract the specified band from the sample-suffix data */
        if (!extract_data_band_as_bsq(s->data_var, band_no, buff1)){
            parse_error("band %d extraction failed.\n", band_no);
            free(buff1); free(buff2); return 0;
        }

        /* The data variable from which data extraction has been done
           may not match the data type of the external type in which
           the data should be output. Convert one internal type to
           the other */
        if (!convert_data(V_FORMAT(s->data_var), buff1,
                          s->item_type, buff2,
                          core_specs->core_items[line_idx])){
            parse_error("%s -> %s conversion failed.\n",
                        get_type_string(V_FORMAT(s->data_var)),
                        get_type_string(s->item_type));
            free(buff1); free(buff2); return 0;
        }

        /* Concatenate the extracted data buffer in column-wise
           direction to side_buff */
        concat_column_data(side_buff,
                           side_items * core_specs->suffix_bytes,
                           core_specs->core_items[line_idx],
                           buff2, core_specs->suffix_bytes,
                           &col_pos);
    }

    free(buff1); free(buff2);

    /* NOTE: suffix_bytes are always set to 4 and none of the data
       stored in the suffixes is larger than 4 bytes per item */
    buff1 = (char *)calloc(bot_buff_sz, core_specs->suffix_bytes);
    buff2 = (char *)calloc(bot_buff_sz, core_specs->suffix_bytes);
    if (buff1 == NULL || buff2 == NULL){
        parse_error("extract_suffix_data_for_band: Mem alloc error.\n");
        free(buff1); free(buff2);
        return 0;
    }
    
    /* process bottom planes */
    row_pos = NULL; /* work-var for concat_row_data */
    for(i = 0; i < bot_items; i++){
        s = core_specs->suffix_attr[line_idx][i];

        /* extract the specified band from the line-suffix data */
        if (!extract_data_band_as_bsq(s->data_var, band_no, buff1)){
            parse_error("band %d extraction failed.\n", band_no);
            free(buff1); free(buff2); return 0;
        }

        /* The davinci variable from which the data is extracted may
           have a different data type then that required by the
           external data type (size differences usually). Convert
           one internal type to the other. */
        if (!convert_data(V_FORMAT(s->data_var), buff1,
                          s->item_type, buff2,
                          core_specs->core_items[sample_idx])){
            parse_error("%s -> %s conversion failed.\n",
                        get_type_string(V_FORMAT(s->data_var)),
                        get_type_string(s->item_type));
            free(buff1); free(buff2); return 0;
        }

        /* Concatenate the extracted data buffer in row-wise
           direction to the bot_buff */
        concat_row_data(bot_buff,
                        core_specs->core_items[sample_idx] *
                        core_specs->suffix_bytes,
                        core_specs->core_items[sample_idx],
                        buff2, core_specs->suffix_bytes,
                        &row_pos);
    }

    free(buff1); free(buff2);

    return 1;
}


/**
 ** extract_core_data_for_band()
 **
 ** Extracts a core data band into core_buff. 
 ** The variable core_specs contains the core data specs
 ** as loaded by get_data_attr_from_lbl.
 **/
static int
extract_core_data_for_band(
    CoreDataSpecs *core_specs,
    int            band_no,
    void          *core_buff
    )
{
    int sample_idx = iom_orders[core_specs->org][0];
    int line_idx = iom_orders[core_specs->org][1];
    int n;
    char *buff1;

    n = core_specs->core_items[sample_idx] * core_specs->core_items[line_idx];
    buff1 = (char *)calloc(n, NBYTES(V_FORMAT(core_specs->core_data_var)));
    if (buff1 == NULL){
        parse_error("extract_core_data_for_band: Mem alloc failure.\n");
        return 0;
    }

    /* extract specified band from the qube */
    if (!extract_data_band_as_bsq(core_specs->core_data_var, band_no, buff1)){
        parse_error("Failed to extract core band %d\n", band_no);
        free(buff1); return 0;
    }

    /* convert from davinci variable type to a davinci variable type suitable
       for output */
    if (!convert_data(V_FORMAT(core_specs->core_data_var), buff1,
                      core_specs->core_item_type, core_buff, n)){
        parse_error("%s -> %s conversion failed.\n",
                    get_type_string(V_FORMAT(core_specs->core_data_var)),
                    get_type_string(core_specs->core_item_type));
        free(buff1); return 0;
    }

    free(buff1);

    return 1;
}


/**
 ** extract_backplane_data_for_band()
 **
 ** Extracts a back-plane data band into back_buff. 
 ** The variable core_specs contains the core data specs
 ** as loaded by get_data_attr_from_lbl.
 **/
static int
extract_backplane_data_for_band(
    CoreDataSpecs *core_specs,
    int            band_no,
    void          *back_buff
    )
{
    int  sample_idx = iom_orders[core_specs->org][0];
    int  line_idx = iom_orders[core_specs->org][1];
    int  band_idx = iom_orders[core_specs->org][2];
    int  item_bytes;
    int  n;
    Var *data_var;
    SfxDataSpecs *s;
    char *buff1;


    /* calculate total number of elements in the plane */
    n = core_specs->core_items[sample_idx] * core_specs->core_items[line_idx];

    s = core_specs->suffix_attr[band_idx][band_no];
    data_var = s->data_var;
    item_bytes = NBYTES(V_FORMAT(data_var));

    /* alloc a buffer big enough to accomodate the data */
    buff1 = (char *)calloc(n, item_bytes);
    if (buff1 == NULL){
        parse_error("extract_backplane_data_for_band: Mem alloc failure.\n");
        return 0;
    }

    /* extract specified back-plane from the qube */
    if (!extract_data_band_as_bsq(data_var, 0, buff1)){
        parse_error("Unable to extract back-plan %d\n", band_no);
        free(buff1); return 0;
    }

    /* convert from davinci variable type a type suitable for output */
    if (!convert_data(V_FORMAT(data_var), buff1, s->item_type, back_buff, n)){
        parse_error("%s -> %s conversion failed.\n",
                    get_type_string(V_FORMAT(data_var)),
                    get_type_string(s->item_type));
        free(buff1); return 0;
    }

    free(buff1);
    
    return 1;
}


/**
 ** get_generic_item_type()
 **
 ** Removes the system specific part from a given string
 ** representation of data format/type. For example:
 ** MSB_UNSIGNED_INTEGER will be returned as UNSIGNED_INTEGER.
 ** It is the user's responsibility to free the returned
 ** string.
 **/
static char *
get_generic_item_type(const char *item_type)
{
    char *s;

    s = strdup(item_type);

    if (strncasecmp(s, "MSB_", 4) == 0 || strncasecmp(s, "SUN_", 4) == 0 ||
        strncasecmp(s, "LSB_", 4) == 0 || strncasecmp(s, "MAC_", 4) == 0){
        strcpy(s, &s[4]);
    }
    else if (strncasecmp(s, "PC_", 3) == 0){
        strcpy(s, &s[3]);
    }
    else if (strncasecmp(s, "VAX_", 4) == 0){
        strcpy(s, &s[4]);
    }
    else if (strncasecmp(s, "VAXG_", 5) == 0){
        strcpy(s, &s[5]);
    }
    else if (strncasecmp(s, "IEEE_", 5) == 0){
        strcpy(s, &s[5]);
    }
    else if (strncasecmp(s, "RIEEE_", 7) == 0){
        strcpy(s, &s[7]);
    }

    return s;
}


/**
 ** set_suffix_names()
 **
 ** Installs the suffix item names into the ISIS cube referenced
 ** by fid.
 ** The variable core_specs contains the core data specs
 ** as loaded by get_data_attr_from_lbl.
 **/
static int
set_suffix_names(
    int fid,
    CoreDataSpecs *core_specs
    )
{
    int k, kk, i;
    int sfx_idx;
    int rc;
    SfxDataSpecs *s;
    char *item_type_str;
    
    for(k = 0; k < 3; k++){
        kk = iom_orders[core_specs->org][k];
        
        for(i = 0; i < core_specs->suffix_items[kk]; i++){
            
            sfx_idx = i+1;
            s = core_specs->suffix_attr[kk][i];

            /**
             ** Convert item type string to something that ISIS can
             ** work with. This fudging is necessary since the current
             ** version of ISIS attaches a machine-specific prefix to
             ** the type string specified in q_set_suffix_keys()
             ** irrespective of whether it already has one.
             **/
            item_type_str = get_generic_item_type(s->item_type_str);
            q_set_suffix_keys(fid, /* add */ 1,
                              core_specs->axis_name[kk],
                              &sfx_idx, s->name, s->item_bytes,
                              item_type_str, s->scale, s->unit, &rc);
            free(item_type_str);
            
            if (rc != 0){
                parse_error("q_set_suffix_keys() failed with rc=%d\n", rc);
                return 0;
            }
        }
    }

    return 1;
}

/**
 ** remove_enclosing_quotes()
 **
 ** Removes one set of enclosing quotes from the specified string.
 **/
static char *
remove_enclosing_quotes(char *s)
{
    if (s[0] == '\"' || s[0] == '\''){
        strcpy(s, &s[1]);
        s[strlen(s)-1]='\0';
    }

    return s;
}


/**
 ** propagate_xxx_keyord()
 **
 ** Writes the value of the specified keyword in the specified
 ** object within the specified group.
 **/
static int
propagate_string_keyword(
    int   fid,
    char *object,
    char *group,
    char *name,
    Var  *v
    )
{
    int   nvals;
    char *vals;
    int   maxlen;
    int   vlen[1];
    int   rc;
    int   double_quoted;

    nvals = 1;
    vals = strdup(V_STRING(v));
    double_quoted = vals[0] == '\"';
    remove_enclosing_quotes(vals);
    maxlen = strlen(vals);
    vlen[0] = maxlen;

    if (double_quoted){
        /* value is a string */
        p_set_str_key(fid, object, group, name, /* set */ 1,
                      &nvals, vals, maxlen, vlen, &rc);
    }
    else {
        /* value is a literal */
        p_set_lit_key(fid, object, group, name, /* set */ 1,
                      &nvals, vals, maxlen, vlen, &rc);
    }

    free(vals);

    return (rc == 0);
}


static int
propagate_string_array_keyword(
    int   fid,
    char *object,
    char *group,
    char *name,
    Var  *v
    )
{
    int   nvals;
    char *vals;
    int   maxlen;
    int  *vlen;
    int   rc;
    int   double_quoted = 0;
    int   i;
    char *s;

    nvals = V_TEXT(v).Row;
    maxlen = get_max_strlen(V_TEXT(v).text, nvals) + 1;
    vals = (char *)calloc(nvals, maxlen);
    vlen = (int *)calloc(nvals, sizeof(int));
    for(i = 0; i < nvals; i++){ vlen[i] = strlen(V_TEXT(v).text[i]); }

    /* see if the strings are double-quoted or not */
    for(i = 0; i < nvals && !double_quoted; i++){
        double_quoted = (V_TEXT(v).text[i][0] == '\"');
    }

    /* get rid of double-quotes - if any */
    s = (char *)calloc(maxlen+1, sizeof(char));
    for(i = 0; i < nvals; i++){
        strcpy(s, V_TEXT(v).text[i]);
        remove_enclosing_quotes(s);
        strcpy(&vals[i*maxlen], s);
        vlen[i] = strlen(s);
    }
    free(s);
    
    if (double_quoted){
        /* values are strings */
        p_set_str_key(fid, object, group, name, /* set */ 1,
                      &nvals, vals, maxlen, vlen, &rc);
    }
    else {
        /* values are literals */
        p_set_lit_key(fid, object, group, name, /* set */ 1,
                      &nvals, vals, maxlen, vlen, &rc);
    }

    free(vals); free(vlen);

    return (rc == 0);
}


static int
propagate_int_keyword(
    int   fid,
    char *object,
    char *group,
    char *name,
    Var  *v
    )
{
    int  nvals;
    int *vals;
    int  rc;
    int  x, y, z;
    int  xn, yn, zn;
    int   i;

    xn = GetX(v); yn = GetY(v); zn=GetZ(v);
    nvals = xn * yn * zn;
    vals = (int *)calloc(nvals, sizeof(int));

    i = 0;
    for(z = 0; z < zn; z++){
        for(y = 0; y < yn; y++){
            for(x = 0; x < xn; x++){
                vals[i++] = extract_int(v, cpos(x, y, z, v));
            }
        }
    }
    
    p_set_int_key(fid, object, group, name, /* set */ 1, &nvals, vals, &rc);

    free(vals);

    return (rc == 0);
}


static int
propagate_float_keyword(
    int   fid,
    char *object,
    char *group,
    char *name,
    Var  *v
    )
{
    int    nvals;
    float *vals;
    int    rc;
    int    x, y, z;
    int    xn, yn, zn;
    int    i;

    xn = GetX(v); yn = GetY(v); zn=GetZ(v);
    nvals = xn * yn * zn;
    vals = (float *)calloc(nvals, sizeof(float));

    i = 0;
    for(z = 0; z < zn; z++){
        for(y = 0; y < yn; y++){
            for(x = 0; x < xn; x++){
                vals[i++] = extract_float(v, cpos(x, y, z, v));
            }
        }
    }
    
    p_set_real_key(fid, object, group, name, /* set */ 1, &nvals, vals,
                   /* F-fmt */ 1, /* dec-digits */ 6, &rc);

    free(vals);

    return (rc == 0);
}

static int
propagate_double_keyword(
    int   fid,
    char *object,
    char *group,
    char *name,
    Var  *v
    )
{
    int     nvals;
    double *vals;
    int     rc;
    int     x, y, z;
    int     xn, yn, zn;
    int     i;

    xn = GetX(v); yn = GetY(v); zn=GetZ(v);
    nvals = xn * yn * zn;
    vals = (double *)calloc(nvals, sizeof(double));

    i = 0;
    for(z = 0; z < zn; z++){
        for(y = 0; y < yn; y++){
            for(x = 0; x < xn; x++){
                vals[i++] = extract_double(v, cpos(x, y, z, v));
            }
        }
    }
    
    p_set_dbl_key(fid, object, group, name, /* set */ 1, &nvals, vals,
                  /* F-fmt */ 1, /* dec-digits */ 6, &rc);

    free(vals);

    return (rc == 0);
}


/**
 ** keyword_already_exists()
 **
 ** Checks to see if the specified keyword already exists in the
 ** specified object and/or group within the specified ISIS cube file.
 **
 **/
static int
keyword_already_exists(
    int   fid,
    char *object,
    char *group,
    char *name
    )
{
    int nvals = 0;
    int rc;

    p_check_key(fid, object, group, name, &nvals, &rc);
    
    return (rc == 0);
}


/**
 ** structure_element_to_keyword()
 **
 ** Undoes name mapping done while reading the data in using load_pds().
 **/
static char *
structure_element_to_keyword(const char *s)
{
    char *kw = strdup(s);
    
    if (strncasecmp(s, "PTR_TO_", 7) == 0){
        sprintf(kw, "^%s", &s[7]);
    }

    return kw;
}

/**
 ** Keep track of which keywords are overridable and which
 ** are not.
 **/
static int
overridable_keyword(
    const char *obj,
    const char *group,
    const char *kw
    )
{
    if (strcasecmp(obj, "qube") == 0 && strcasecmp(group, "") == 0){
        if (strcasecmp(kw, "core_null") == 0){ return 1; }
        if (strcasecmp(kw, "core_valid_minimum") == 0){ return 1; }
        if (strcasecmp(kw, "core_low_repr_saturation") == 0){ return 1; }
        if (strcasecmp(kw, "core_low_instr_saturation") == 0){ return 1; }
        if (strcasecmp(kw, "core_high_repr_saturation") == 0){ return 1; }
        if (strcasecmp(kw, "core_high_instr_saturation") == 0){ return 1; }
    }

    return 0;
}

/**
 ** Keep track of which keywords should never make it to their
 ** respective objects/groups.
 **/
static int
barred_keyword(
    const char *obj,
    const char *group,
    const char *kw
    )
{
    if (strcasecmp(kw, "Object") == 0){ return 1; }
    if (strcasecmp(kw, "data") == 0){ return 1; }
    if (strcasecmp(kw, "suffix_data") == 0){ return 1; }

    if (strcmp(obj, "") == 0 && strcasecmp(group, "") == 0){
        if (kw[0] == '^' && strcasecmp(kw, "^description") != 0){
            /**
             ** Any ^blah keyword other than ^description is barred
             ** at the file level
             **/
            return 1;
        }
    }

    return 0;
}

/**
 ** propagate_keywords_0()
 **
 ** Worker function used by propagate_keywords().
 **/
static int
propagate_keywords_0(
    int    fid,
    Var   *s,
    char  *object,
    char  *group
    )
{
    int    n = 0;
    char  *element_name = NULL;
    Var   *v = NULL;
    int    i;
    int    struct_is_a_group;
    char  *name;
    Var   *so;
    

    /* get a count of structure elements */
    n = get_struct_count(s);

    /* traverse all keywords at this level */
    for(i = 0; i < n; i++){
        
        /* get the next element from the structure */
        get_struct_element(s, i, &element_name, &v);

        /**
         ** undo stuff like:
         ** ^qube => ptr_to_qube
         **/
        name = structure_element_to_keyword(element_name);

        /**
         ** If the current keyword is barred, toss it.
         **/
        if (barred_keyword(object, group, name)){
            free(name); continue;
        }

        /**
         ** If this keyword already exists in the file, then ignore
         ** it. Otherwise, write this new keyword into the file.
         **/
        if (keyword_already_exists(fid, object, group, name)){
            if (overridable_keyword(object, group, name)){
                /**
                 ** If this is an overridable qube keyword, like:
                 ** CORE_VALID_MINIMUM, CORE_NULL, ...
                 ** then override it by letting it pass through
                 ** the standard processing.
                 **/
            }
            else {
                /**
                 ** Else ignore this keyword as it already exists in
                 ** the file and we want to keep the value already
                 ** in the file.
                 **/
                free(name); continue;
            }
        }

        switch(V_TYPE(v)){
        case ID_VAL:    /* numeric array or a single value */
            switch(V_FORMAT(v)){
            case BYTE:
            case SHORT:
            case INT:
                propagate_int_keyword(fid, object, group, name, v);
                break;
                
            case FLOAT:
                propagate_float_keyword(fid, object, group, name, v);
                break;
                
            case DOUBLE:
                propagate_double_keyword(fid, object, group, name, v);
                break;
                
            default:
                parse_error("Ignoring keyword %s in scope %s of unhandled type.\n",
                            name, object);
                break;
            }
            break;
            
        case ID_STRING: /* a single string */
            propagate_string_keyword(fid, object, group, name, v);
            break;
            
        case ID_TEXT:   /* array of strings */
            propagate_string_array_keyword(fid, object, group, name, v);
            break;
            
        case ID_STRUCT: /* a grouping/complex object */
            struct_is_a_group = 1;
            so = NULL;
            
            if (find_struct(v, "Object", &so) < 0){
                struct_is_a_group = 1;
            }
            else {
                if (V_TYPE(so) == ID_STRING){
                    if (strcasecmp(V_STRING(so), "group") == 0){
                        struct_is_a_group = 1;
                    }
                    else if (strcasecmp(V_STRING(so), "qube") == 0){
                        struct_is_a_group = 0;
                    }
                    else if (strcasecmp(V_STRING(so), "spectral_qube") == 0){
                        struct_is_a_group = 0;
                        free(name); name = strdup("qube");
                    }
                    else if (strcasecmp(V_STRING(so), "history") == 0){
                        /* history is handled elsewhere */
                        break;
                    }
                    else {
                        parse_error("Ignoring %s ...\n", name);
                        break;
                    }
                }
                else {
                    struct_is_a_group = 0;
                }
            }


            if (struct_is_a_group){
                /* structure contains a group */
                propagate_keywords_0(fid, v, object, name);
            }
            else {
                /* structure contains an object */
                propagate_keywords_0(fid, v, name, group);
            }

            break;
            
        default:
            parse_error("Ignoring keyword %s in scope %s of unhandled type.\n",
                        name, object);
            break;
        }
        free(name);
    }

    return 1;
}


/**
 ** propagate_keywords()
 **
 ** Writes keywords from the structural heirarchy in "s" to the
 ** ISIS cube. No existing keywords are overwritten and the history,
 ** table, and image objects are not dealt with here.
 **
 ** NOTE: Right now I don't have an interface to specify what to
 ** deal with and what to ignore when propagating keywords. There
 ** are some hard-wired rules in propagate_keywords_0() though.
 **/
static int
propagate_keywords(
    int  fid,
    Var *s
    )
{
    return propagate_keywords_0(fid, s, "", "");
}


/**
 ** propagate_history_xxx_keyword()
 **
 ** Writes the specified keyword into the cube's history using
 ** the values given by the davinci variable "v".
 **/

static int
propagate_history_string_keyword(
    int   indent,
    char *name,
    Var  *v
    )
{
    int   nvals;
    char *vals;
    int   maxlen;
    int   vlen[1];
    int   rc;
    int   double_quoted;

    nvals = 1;
    vals = strdup(V_STRING(v));
    double_quoted = vals[0] == '\"';
    remove_enclosing_quotes(vals);
    maxlen = strlen(vals);
    vlen[0] = maxlen;

    if (double_quoted){
        /* value is a string */
        u_put_str_key(4, indent, name, nvals, vals, maxlen, vlen, &rc);
    }
    else {
        /* value is a literal */
        u_put_lit_key(4, indent, name, nvals, vals, maxlen, vlen, &rc);
    }

    free(vals);

    return (rc == 0);
}


static int
propagate_history_string_array_keyword(
    int   indent,
    char *name,
    Var  *v
    )
{
    int   nvals;
    char *vals;
    int   maxlen;
    int  *vlen;
    int   rc;
    int   double_quoted = 0;
    int   i;
    char *s;

    nvals = V_TEXT(v).Row;
    maxlen = get_max_strlen(V_TEXT(v).text, nvals) + 1;
    vals = (char *)calloc(nvals, maxlen);
    vlen = (int *)calloc(nvals, sizeof(int));
    for(i = 0; i < nvals; i++){ vlen[i] = strlen(V_TEXT(v).text[i]); }

    /* see if the strings are double-quoted or not */
    for(i = 0; i < nvals && !double_quoted; i++){
        double_quoted = (V_TEXT(v).text[i][0] == '\"');
    }

    /* get rid of double-quotes - if any */
    s = (char *)calloc(maxlen+1, sizeof(char));
    for(i = 0; i < nvals; i++){
        strcpy(s, V_TEXT(v).text[i]);
        remove_enclosing_quotes(s);
        strcpy(&vals[i*maxlen], s);
        vlen[i] = strlen(s);
    }
    free(s);
    
    if (double_quoted){
        /* values are strings */
        u_put_str_key(4, indent, name, nvals, vals, maxlen, vlen, &rc);
    }
    else {
        /* values are literals */
        u_put_lit_key(4, indent, name, nvals, vals, maxlen, vlen, &rc);
    }

    free(vals); free(vlen);

    return (rc == 0);
}


static int
propagate_history_int_keyword(
    int   indent,
    char *name,
    Var  *v
    )
{
    int  nvals;
    int *vals;
    int  rc;
    int  x, y, z;
    int  xn, yn, zn;
    int  i;

    xn = GetX(v); yn = GetY(v); zn=GetZ(v);
    nvals = xn * yn * zn;
    vals = (int *)calloc(nvals, sizeof(int));

    i = 0;
    for(z = 0; z < zn; z++){
        for(y = 0; y < yn; y++){
            for(x = 0; x < xn; x++){
                vals[i++] = extract_int(v, cpos(x, y, z, v));
            }
        }
    }
    
    u_put_int_key(4, indent, name, nvals, vals, &rc);

    free(vals);

    return (rc == 0);
}


static int
propagate_history_real_keyword(
    int   indent,
    char *name,
    Var  *v
    )
{
    int     nvals;
    float  *vals;
    int     rc;
    int     x, y, z;
    int     xn, yn, zn;
    int     i;

    xn = GetX(v); yn = GetY(v); zn=GetZ(v);
    nvals = xn * yn * zn;
    vals = (float *)calloc(nvals, sizeof(float));

    i = 0;
    for(z = 0; z < zn; z++){
        for(y = 0; y < yn; y++){
            for(x = 0; x < xn; x++){
                vals[i++] = extract_float(v, cpos(x, y, z, v));
            }
        }
    }
    
    u_put_real_key(4, indent, name, nvals, vals, 1, 6, &rc);

    free(vals);

    return (rc == 0);
}


/**
 ** propagate_history_group_keywords()
 **
 ** Work-horse behind propagate_history_group(). It contains
 ** certain hard-wired stuff.
 **/
static int
propagate_history_group_keywords(
    Var  *s,
    const char *group,
    int   indent,
    int   level
    )
{
    int   i, n;
    Var  *v;
    char *name;
    int   rc;
    
    n = get_struct_count(s);

    for(i = 0; i < n; i++){
        get_struct_element(s, i, &name, &v);
        if (strcasecmp(name, "Object") == 0){
            continue;
        }

        switch(V_TYPE(v)){
        case ID_STRUCT:
            if (level == 1 && strcasecmp(name, "parameters") == 0){
                int vlen[1], maxlen;

                maxlen = vlen[0] = strlen("parameters");
                u_put_lit_key(4, indent, "group", 1, "parameters", maxlen, vlen, &rc);
                
                propagate_history_group_keywords(v, group, indent+1, level+1);
                
                u_put_lit_key(4, indent, "end_group", 1, "parameters", maxlen, vlen, &rc);
            }
            else {
                parse_error("Invalid section %s within history group %s. Ignored.",
                            name, group);
                continue;
            }
            break;

        case ID_STRING:
            propagate_history_string_keyword(indent, name, v); break;
            break;

        case ID_TEXT:
            propagate_history_string_array_keyword(indent, name, v); break;
            break;
            
        case ID_VAL:
            switch(V_FORMAT(v)){
            case BYTE:
            case SHORT:
            case INT:
                propagate_history_int_keyword(indent, name, v); break;
            case FLOAT:
            case DOUBLE:
                propagate_history_real_keyword(indent, name, v); break;
            default:
                parse_error("Invalid format for %s within history group %s. Ignored.",
                            name, group);
                break;
            }
            break;
            
        default:
            parse_error("Invalid type for %s within history group %s. Ignored.",
                        name, group);
            break;
        }
    }

    return 1;
}


/**
 ** Writes one block of history into the output ISIS file.
 **/
static int
propagate_history_group(
    int   fid,
    char *group_name,
    Var  *hg
    )
{
    int rc;
    
    h_init_buf(group_name);

    propagate_history_group_keywords(hg, group_name, 1, 1);
    
    h_end_parm();
    /* h_put(fid, &rc); when left here caused the last history
       entry to be duplicated. Moving this call before the
       history group is written seems to fix the problem. */

    return(rc == 0);
}


/**
 ** Outputs history from davinci variable to an ISIS file.
 ** Groups which end in _999 where 999 is any number are
 ** replaced with group-name without this serial-number
 ** suffix. This serial number suffix is added during reading
 ** of history groups, many of which may have duplicate
 ** names, which is not supported by davinci.
 **/
static int
propagate_history(
    int  fid,
    Var *h      /* history structure */
    )
{
    int i, n;
    Var *v;
    char *group_name;
    char *norm_group_name;
    regex_t serial_extended_group_pattern;
    regmatch_t matches[1];
    int rc;
    int entry_no = 0;

    regcomp(&serial_extended_group_pattern, "_[0-9][0-9]*$", 0);
    
    /* get number of elements in the history structure */
    n = get_struct_count(h);

    /* process each of the history elements */
    for(i = 0; i < n; i++){
        
        /* get the next element from the structure */
        get_struct_element(h, i, &group_name, &v);

        if (V_TYPE(v) == ID_STRUCT){
            norm_group_name = strdup(group_name);
            if (regexec(&serial_extended_group_pattern, norm_group_name, 1, matches, 0) == 0){
                norm_group_name[matches[0].rm_so] = '\0';
            }

            if (++entry_no > 1){
                h_put(fid, &rc);
            }
            propagate_history_group(fid, norm_group_name, v);
            free(norm_group_name);
        }
        else {
            if (strcasecmp(group_name, "Object") != 0){
                parse_error("Ignoring non-structure %s in history\n", group_name);
            }
        }
    }

    return 1;
}


/**
 ** dv_WriteISISStruct()
 **
 ** Writes the data pointed to by the structure "s" into
 ** the specified file. The minimal structure is organized
 ** as follows:
 **
 ** TOPLEVEL
 **   + qube|spectral_qube            qube or spectral_qube
 **       + Object                    (must be object)
 **       + axes                      #dims (must be 3)
 **       + axis_name
 **       + core_items
 **       + core_item_bytes
 **       + core_item_type
 **       + core_base                 
 **       + core_multiplier           
 **       + suffix_items
 **       + suffix_bytes
 **       + data                      the core data
 **       + suffix_data               data for the core suffixes
 **         + sample                  side-planes
 **         + line                    bottom-planes
 **         + band                    back-planes
 **       + xxx_suffix_name           xxx = {sample,line,band}
 **       + xxx_suffix_item_bytes    
 **       + xxx_suffix_item_type
 **       + xxx_suffix_unit
 **       + xxx_suffix_base           
 **       + xxx_suffix_multiplier     
 **   + history                       optional history object
 **       + data                      history data (kw=value pairs)
 **
 ** xxx_suffix_yyy refers to {sample,line,band} suffix-planes.
 ** Say if band-suffixes exist then band_suffix_name,
 ** band_suffix_item_bytes, band_suffix_item_type,
 ** and band_suffix_unit are required but band_suffix_base
 ** and band_suffix_multiplier are optional.
 **
 ** History data can only be two level deep, i.e. history.data
 ** should contain groups and each group an have at most one
 ** sub-structure within in called parameters.
 **
 ** Keywords that appear in the structure at various levels
 ** are copied out verbatim, unless they are overridden by
 ** the data/suffix_data specifications.
 **
 ** Data types do not have to exactly match. For example, integer
 ** data is appropriately converted to shorts if the output format
 ** specifies shorts. Note that only numeric data is handled
 ** right now.
 **
 ** Objects vs. Groups are determined by using the Object field
 ** in a structure. If it is present, it is taken at its face
 ** value. If it is not present, the structure is assumed to be
 ** a group.
 **
 ** The output file is generated via the ISIS writer. It
 ** has been designed using ISIS v2 writer.
 **
 **/
int
dv_WriteISISStruct(Var *s, char *fname, int force)
{
    INT4   fid = 0;                /* fileid for ISIS calls */
    Var   *qube = NULL;            /* struct.qube */
    int    qube_index = -1;
    Var   *qube_data = NULL;       /* struct.qube.data */
    int    qube_data_index = -1;
    Var   *suffix_data = NULL;     /* struct.qube.suffix_data */
    int    suffix_data_index = -1;
    INT4   order;                  /* ISIS-org of the output ISIS data */
    int    max_axis_name_len = 0;  /* max(length(axis_names)) */
    CHAR  *axname = NULL;          /* dim(axis_names) * max_axis_name_len */
    Var   *v = NULL;
    INT4   rc = 0;
    int    i, core_axes;
    char  *suffix_buff[3] = {NULL, NULL, NULL}; /* suffix buffers for q_pio_qube */
    int    suffix_buff_sizes[3] = {0, 0, 0}; /* sizes (#items) of suffix_buff above */
    char  *core_data = NULL;
    int    core_data_size_items;   /* total core data size in items */
    char  *core_item_type_str = NULL;
    CoreDataSpecs *a = NULL;
    char  *side_data = NULL, *bot_data = NULL;
    int    core_plane_size_items, side_plane_size_items, bot_plane_size_items;
    int    band_no;
    Var   *h, *hist_data;          /* variables for processing of history */
    char  *notprop = "";           /* list of groups not to propagate forward -
                                      for some reason if one passes q_open() a
                                      NULL in this parameter's place, it sets */


    /* get pointer to the qube structure */
    if ((qube_index = find_struct(s, "qube", &qube)) < 0){
        if ((qube_index = find_struct(s, "spectral_qube", &qube)) < 0){
            parse_error("Unable to find qube or spectral_qube in the input struct.\n");
            return 0;
        }
    }
    
    /* get qube.axes */
    if (find_struct(qube, "axes", &v) < 0 || (core_axes = extract_int(v, 0)) != 3){
        parse_error("Expecting qube.axes = 3\n"); return 0;
    }
    
    /* get pointer to the qube data */
    if ((qube_data_index = find_struct(qube, "data", &qube_data)) < 0){
        parse_error("Expecting qube.data\n"); return 0;
    }
    
    /* get pointer to the qube suffixes data - this element is optional though */
    suffix_data_index = find_struct(qube, "suffix_data", &suffix_data);

    /* get common data attributes from keywords in the qube structure */
    a = get_data_attrs_from_lbl(qube);
    if (a == NULL){
        parse_error("Error while getting attributes from the structure. "
                    "See previous messages.\n");
        return 0;
    }

    /* translate core_org to its equivalent ISIS-order */
    if (!get_isis_order_for_org(a->org, &order)){
        parse_error("Unable to get ISIS order for org %s (%d)\n",
                    get_org_string(a->org), a->org);
        free_CoreDataSpecs(&a);
        return 0;
    }

    /* build the axname string as per the core_org */
    max_axis_name_len = get_max_strlen(a->axis_name, a->axes);
    axname = (char *)calloc((max_axis_name_len+1)*a->axes, sizeof(char));
    for(i = 0; i < 3; i++){
        sprintf(&axname[i*(max_axis_name_len+1)], "%s", a->axis_name[i]);
    }

    /* set skeletal size of the core and suffixes */
    core_item_type_str = get_generic_item_type(a->core_item_type_str);
    q_set_sys_keys(&fid, a->axes, order,
                   a->core_items, a->suffix_items,
                   axname, max_axis_name_len+1,
                   a->core_item_bytes, core_item_type_str,
                   a->core_scale, &rc);
    free(core_item_type_str);
    if (rc != 0){
        parse_error("q_set_sys_keys failed\n");
        free_CoreDataSpecs(&a); return 0;
    }

    /* if the user has asked to overwrite the file, then delete
       it first, since q_open() fails on an existing file. */
    if (force && access(fname, F_OK) == 0){ unlink(fname); }
    
    /* open qube */
    q_open(&fid, 0, fname, CREATE_NEW, 0, notprop, 0, 0, 3, 0, &rc);
    if (rc != 0){
        parse_error("q_open failed\n");
        free_CoreDataSpecs(&a); return 0;
    }

    /**
     ** Alloc memory for a core plane along with the associated
     ** side and bottom suffixes. Back-suffix plane is handled
     ** sparately.
     **/
    core_plane_size_items = a->core_items[iom_orders[a->org][0]] *
        a->core_items[iom_orders[a->org][1]];
    side_plane_size_items = a->suffix_items[iom_orders[a->org][0]] *
        (a->core_items[iom_orders[a->org][1]] +
         a->suffix_items[iom_orders[a->org][1]]);
    bot_plane_size_items = a->suffix_items[iom_orders[a->org][1]] *
        a->core_items[iom_orders[a->org][0]];

    core_data = (char *)calloc(core_plane_size_items, a->core_item_bytes);
    side_data = (char *)calloc(side_plane_size_items, a->suffix_bytes);
    bot_data  = (char *)calloc(bot_plane_size_items, a->suffix_bytes);

    if (core_data == NULL || side_data == NULL || bot_data == NULL){
        parse_error("Mem alloc error %d bytes.\n",
                    core_plane_size_items * a->core_item_bytes +
                    (side_plane_size_items + bot_plane_size_items) *
                    a->suffix_bytes);
        free_CoreDataSpecs(&a); q_close(fid, DELETE_FILE, &rc);
        return 0;
    }


    /* copy data to the output, one plane at a time */
    for(band_no = 0; band_no < a->core_items[iom_orders[a->org][2]]; band_no++){
        extract_core_data_for_band(a, band_no, core_data);
        extract_suffix_data_for_band(a, band_no, side_data, bot_data);
        q_lio_image(fid, WRITE, band_no+1, core_data, side_data, bot_data, &rc);
    }

    /* realloc an image plane of the size of a back-plane */
    free(core_data);
    core_data = (char *)calloc(core_plane_size_items, a->suffix_bytes);
    if (core_data == NULL){
        parse_error("Mem alloc error %d bytes.\n",
                    core_plane_size_items * a->suffix_bytes);
        free_CoreDataSpecs(&a); q_close(fid, DELETE_FILE, &rc);
        return 0;
    }

    /* clear out side_data and bot_data */
    memset(side_data, 0, side_plane_size_items * a->suffix_bytes);
    memset(bot_data,  0, bot_plane_size_items  * a->suffix_bytes);

    /* copy band-suffixes to the output, one plane at a time */
    for(band_no = 0; band_no < a->suffix_items[iom_orders[a->org][2]]; band_no++){
        extract_backplane_data_for_band(a, band_no, core_data);
        q_lio_backplane(fid, WRITE, band_no+1, core_data, side_data, bot_data, &rc);
    }

    free(core_data);
    free(side_data);
    free(bot_data);

    /* set the suffix names appropriately */
    if (!set_suffix_names(fid, a)){
        parse_error("Failed to set the suffix names\n");
        free_CoreDataSpecs(&a); q_close(fid, DELETE_FILE, &rc);
        return 0;
    }

    /* add misc keywords to the qube */
    if (!propagate_keywords(fid, s)){
        parse_error("Failed to propagate keywords to the qube\n");
        free_CoreDataSpecs(&a); q_close(fid, DELETE_FILE, &rc);
        return 0;
    }

    if (find_struct(s, "history", &h) >= 0){
        if (find_struct(h, "data", &hist_data) >= 0){
            /* add history to the qube */
            if (!propagate_history(fid, hist_data)){
                parse_error("Failed to propagate history to the qube\n");
                free_CoreDataSpecs(&a); q_close(fid, DELETE_FILE, &rc);
                return 0;
            }
        }
    }

    /* close qube */
    q_close(fid, KEEP_FILE, &rc);
    if (rc != 0){
        parse_error("q_close failed\n");
        return 0;
    }

    /* return success */
    return 1;
}

#endif /* HAVE_LIBISIS */

/***********************************************************************/
/* End of ISIS Cube Writer Code                                        */
/***********************************************************************/


