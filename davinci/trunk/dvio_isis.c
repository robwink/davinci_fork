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
    char *msg_file = NULL;

    if (iom_isISIS(fp) == 0) return(NULL);

    if (VERBOSE){
        msg_file = NULL;
    }
    else {
#ifdef _WIN32
        msg_file = "nul:";
#else
        msg_file = "/dev/null";
#endif /* _WIN32 */
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
        } else {
            for (y=h->s_lo[1];y<h->s_hi[1];y+=h->s_skip[1]){ 
                if (ordinate==1 && h->s_skip[0]==1 && h->s_skip[2]==1){
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
        (fp = fopen(fname, "r")) == NULL) {
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
#ifdef _WIN32
        msg_file = "nul:";
#else
        msg_file = "/dev/null";
#endif /* _WIN32 */
    }

    if (iom_GetISISHeader(fp, fname, &s, msg_file, &object) == 0) {
        parse_error("%s: not an ISIS file", fname);
        free(fname);
        return (NULL);
    }

    if ((qube = OdlFindObjDesc(object, "QUBE", NULL, 0, 0, 0)) == NULL) {
        parse_error("%s: Not a qube object.", isisfile);
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


    Alist alist[6];
    alist[0] = make_alist( "core",    	ID_VAL,    	NULL,    &core);
    alist[1] = make_alist( "side", 	ID_STRUCT,  NULL,    &suffix[0]);
    alist[2] = make_alist( "bottom",	ID_STRUCT,  NULL,    &suffix[1]);
    alist[3] = make_alist( "back",  	ID_STRUCT,  NULL,    &suffix[2]);
    alist[4] = make_alist( "filename",  ID_STRING,  NULL,    &filename);
    alist[5].name = NULL;

    if (parse_args(func, arg, alist) == 0) return(NULL);

    if (core == NULL) {
        parse_error("%s: No core object specified", func->name);
        return(NULL);
    }
    if (V_ORG(core) != BSQ) {
        parse_error("%s: only able to handle BSQ cores", func->name);
        return(NULL);
    }

    if (filename == NULL) {
        parse_error("%s: No filename specified\n", func->name);
        return(NULL);
    }

    fp = fopen(filename, "w");

    size[0] = GetX(core);
    size[1] = GetY(core);
    size[2] = GetZ(core);

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
    sprintf(buf+strlen(buf), "RECORD_BYTES = 1\n");
    sprintf(buf+strlen(buf), "^QUBE =        \n");
    sprintf(buf+strlen(buf), "OBJECT = QUBE\n");
    sprintf(buf+strlen(buf), "    AXES = 3\n");
    sprintf(buf+strlen(buf), "    AXIS_NAME = (SAMPLE,LINE,BAND)\n");
    sprintf(buf+strlen(buf), "    CORE_ITEMS = (%d,%d,%d)\n", size[0], size[1], size[2]);
    sprintf(buf+strlen(buf), "    CORE_ITEM_BYTES = %d\n", GetNbytes(core));
    sprintf(buf+strlen(buf), "    CORE_ITEM_TYPE = %s\n", iformat_to_eformat(core));
    sprintf(buf+strlen(buf), "    SUFFIX_BYTES = 4\n");
    sprintf(buf+strlen(buf), "    SUFFIX_ITEMS = (");

    for (i = 0 ; i < 3 ; i++) {
        if (i) sprintf(buf+strlen(buf),",");
        if (suffix[i] == NULL)  sprintf(buf+strlen(buf), "0");
        else sprintf(buf+strlen(buf), "%d", get_struct_count(suffix[i]));
    }
    sprintf(buf+strlen(buf), ")\n");

    for (i = 0 ; i < 3 ; i++) {
        char *suf_prefix;
        if (suffix[i]) {
            switch (i) {
            case 0: suf_prefix = "    SAMPLE_SUFFIX"; break;
            case 1: suf_prefix = "    LINE_SUFFIX"; break;
            case 2: suf_prefix = "    BAND_SUFFIX"; break;
            }

            /* names */
            sprintf(buf+strlen(buf), "%s_NAME = (", suf_prefix);
            for (j = 0 ; j < get_struct_count(suffix[i]) ; j++) {
                if (j) sprintf(buf+strlen(buf), ",");
                get_struct_element(suffix[i], j, &name, &v);		
                sprintf(buf+strlen(buf), "%s", name);
            }
            sprintf(buf+strlen(buf), ")\n");

            /* size */
            sprintf(buf+strlen(buf), "%s_ITEM_BYTES = (", suf_prefix);
            for (j = 0 ; j < get_struct_count(suffix[i]) ; j++) {
                if (j) sprintf(buf+strlen(buf), ",");
                get_struct_element(suffix[i], j, &name, &v);		
                sprintf(buf+strlen(buf), "%d", GetNbytes(v));
            }
            sprintf(buf+strlen(buf), ")\n");

            /* type */
            sprintf(buf+strlen(buf), "%s_ITEM_TYPE = (", suf_prefix);
            for (j = 0 ; j < get_struct_count(suffix[i]) ; j++) {
                if (j) sprintf(buf+strlen(buf), ",");
                get_struct_element(suffix[i], j, &name, &v);		
                sprintf(buf+strlen(buf), "%s", iformat_to_eformat(v));
            }
            sprintf(buf+strlen(buf), ")\n");
        }
    }

    sprintf(buf+strlen(buf), "END_OBJECT = QUBE\n");
    sprintf(buf+strlen(buf), "END\n");

    p = strstr(buf, "^QUBE =");
    p += 8;
    sprintf(lenstr, "%d", strlen(buf)+1);
    strncpy(p, lenstr, strlen(lenstr));

	fwrite(buf, strlen(buf), 1, fp);

    nbytes = GetNbytes(core);

    if (V_ORG(core) == BSQ) {
        for (k = 0 ; k < size[2] ; k++) {
            for (j = 0 ; j < size[1] ; j++) {
                pos = (k*size[1]+j)*size[0] * nbytes;
                fwrite(V_DATA(core) + pos, size[0], nbytes, fp);

                if (suffix[0]) {
                    for (n = 0 ; n < get_struct_count(suffix[0]) ; n++) {
                        get_struct_element(suffix[0], n, NULL, &v);
                        write_one(v, 0, j, k, fp);
                    }
                }
            }
            if (suffix[1]) {
                for (n = 0 ; n < get_struct_count(suffix[1]) ; n++) {
                    get_struct_element(suffix[1], n, NULL, &v);
                    write_row_x(v, 0, k, fp);
                }
            }
        }
        if (suffix[2]) {
            for (n = 0 ; n < get_struct_count(suffix[2]) ; n++) {
                get_struct_element(suffix[2], n, NULL, &v);
                write_plane(v, V_ORG(core), 2, fp);
            }
        }
    }
    fclose(fp);
    return(NULL);
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
    fwrite(V_DATA(v) + pos*nbytes, 1, nbytes, fp);
} 

write_row_x(Var *v, int y, int z, FILE *fp)
{
    int i;
    int x = GetX(v);

    for (i = 0 ; i < x ; i++) {
        write_one(v, i, y, z, fp);
    }
}
write_row_y(Var *v, int x, int z, FILE *fp)
{
    int i;
    int y = GetY(v);

    for (i = 0 ; i < y ; i++) {
        write_one(v, x, i, z, fp);
    }
}

write_row_z(Var *v, int x, int y, FILE *fp)
{
    int i;
    int z = GetZ(v);

    for (i = 0 ; i < z ; i++) {
        write_one(v, x, y, i, fp);
    }
}

write_plane(Var *v, int org, int plane, FILE *fp) 
{
    int i;
    int x = GetX(v);
    int y = GetY(v);
    int z = GetZ(v);
    
    if (plane == 0) {
        if (org == BIL) {		/* write rows of Y */
            for (i = 0 ; i < z ; i++) {
                write_row_y(v, 0, i, fp);
            }
        } else if (org == BIP) {	/* write rows of Z */
            for (i = 0 ; i < y ; i++) {
                write_row_z(v, 0, i, fp);
            }
        }
    }
    if (plane == 1) {
        if (org == BIL) {		/* write rows of x */
            for (i = 0 ; i < z ; i++) {
                write_row_x(v, 0, i, fp);
            }
        } else if (org == BIP) {	/* write rows of z */
            for (i = 0 ; i < x ; i++) {
                write_row_z(v, i, 0, fp);
            }
        }
    }
    if (plane == 2) {
        for (i = 0 ; i <y ; i++) {	/* write rows of X */
            write_row_x(v, i, 0, fp);
        }
    }
}

char *
iformat_to_eformat(Var *obj) 
{
    static char buf[256];
    switch (V_FORMAT(obj)) {
    case BYTE: sprintf(buf,"MSB_UNSIGNED_INTEGER"); break;
    case SHORT: sprintf(buf,"MSB_INTEGER"); break;
    case INT: sprintf(buf,"MSB_INTEGER"); break;
    case FLOAT: sprintf(buf,"IEEE_REAL"); break;
    case DOUBLE: sprintf(buf,"IEEE_REAL"); break;
    }
    return(buf);
}
