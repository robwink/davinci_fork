#include "parser.h"
#include "dvio.h"


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

    if (iom_isISIS(fp) == 0) return(NULL);

    if (iom_GetISISHeader(fp, filename, &h, NULL) == 0) {
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
        if ((fd = open(datafile, O_RDONLY)) < 0) {
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
            iom_EFormat2Str(h.format), iom_NBYTESI(h.format)*8);

    if (VERBOSE > 1) { parse_error(hbuf); }
    V_TITLE(v) = strdup(hbuf);

    iom_cleanup_iheader(&h);
    
    return(v);
}


int 
dv_WriteISIS(Var *s, FILE *fp, char *filename, char *title )
{
    struct iom_iheader h;

    /*
    ** Build an _iheader structure from the "Var *s" that
    ** is suitable for writing an ISIS output file.
    */

    iom_var2iheader(s, &h);
    iom_WriteISIS(fp, filename, V_DATA(s), &h, V_TITLE(s) == NULL? "(none)": V_TITLE(s));
    return(1);
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
#ifdef _WIN32
    else err_file = "nul:";
#else
    else err_file = "/dev/null";
#endif /* _WIN32 */

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
                 int s_bytes,
                 int plane_number,
                 int s_item_byte_e, /* external data type as stored in file */
                 int *s_item_byte,  /* internal representation in memory */
                 int ordinate,
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


    /*
    ** byte_swap_data() returns the machine dependent type of
    ** data corresponding to the input data type passed in
    ** "s_item_byte_e".
    */
    format = iom_byte_swap_data(data, dsize/s_bytes, s_item_byte_e);
    *s_item_byte = format;

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
						*type=iom_orders[h->org][i];
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
ff_read_suffix_plane2(vfuncptr func, Var * arg)
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

    if ((fname = iom_locate_file(isisfile)) == NULL ||
        (fp = fopen(fname, "r")) == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", isisfile);
        return (NULL);
    }

    strcpy(fname2, fname);
    free(fname);
    fname = fname2;

    if (iom_GetISISHeader(fp, fname, &s, &object) == 0) {
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
                    iom_edf format = iom_ConvertISISType(type_list[i], NULL, size_list[i]);
                    fprintf(stderr, "%s Plane %d: '%s' (%s byte %s)\n",
                            str, j, name_list[j], 
                            size_list[j], 
                            iom_EFormat2Str(format));
                }
            }
        }
        return(NULL);
        
    } else if (V_TYPE(Suffix)==ID_STRING) {
        name=V_STRING(Suffix);
        if(dv_LookupSuffix(qube, name, &plane, &type, &s, suffix)) {
			parse_error("Unable to find plane: %s", name);
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
        return(NULL);
    }


    if ((plane)>= suffix[iom_orders[s.org][type]]){ 
        fprintf(stderr, "The cube only has %d %s-Suffix plane%s\n",
                suffix[iom_orders[s.org][type]],
                ((type==0) ? ("Sample"): ((type==1) ? ("Line"):("Band"))),
                (suffix[iom_orders[s.org][type]] > 1 ? ("s"):("")));
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

    s.s_hi[iom_orders[s.org][type]]=1;

    if (suffix_bytes!=iom_NBYTESI(format))
        data=dv_RePackData(data,suffix_bytes,format,chunk);
    
    return(newVal(s.org,
                  s.s_hi[0],
                  s.s_hi[1],
                  s.s_hi[2],
                  ihfmt2vfmt(format),
                  data));
}

