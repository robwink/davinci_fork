#ifdef HAVE_CONFIG_H
#include <iom_config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#include <string.h>
#include "iomedley.h"
#include "io_lablib3.h"


iom_edf iom_ConvertISISType(char *type, char * bits, char *bytes);


int
iom_isISIS(FILE *fp)
{
    char        buf[256];

    rewind(fp);

    if (fgets(buf, 256, fp) == NULL) {
        return(0);
    }
    if (strncmp(buf, "CCSD", 4) &&
        strncmp(buf, "NJPL", 4) &&
        strncmp(buf,"PDS",3)) {
        return(0);
    }

        /**
         ** Best guess is yes, it is.
         **/
    return(1);
}


int
iom_GetISISHeader(
    FILE *fp, 
    char *filename, 
    struct iom_iheader *h,
    char *msg_file,     /* file to which the parse-messages should go */
    OBJDESC **r_obj     /* NULL allowed */
    )
{
    OBJDESC *ob = NULL;
    OBJDESC *qube = NULL;
    OBJDESC *image = NULL;
    KEYWORD * key, *key1, *key2;
    int rlen;
    int scope = ODL_THIS_OBJECT;
    char        *ptr, p1[16], p2[16],p3[16];
    int org, format, size[3];
    int suffix[3], suffix_size[3];
    int i;
    float       offset, gain;
    unsigned short start_type;
    size_t start;
    int suffix_bytes = 0;
    char *err_file = NULL;
    char *ddfname = NULL; /* Detached Data-File Name */
    int line_prefix = 0;
    int t;
    char *tempString = NULL;

    size[0] = size[1] = size[2] = 0;
    suffix[0] = suffix[1] = suffix[2] = 0;
    suffix_size[0] = suffix_size[1] = suffix_size[2] = 0;

    if (!iom_isISIS(fp)){
        return 0;
    }
    
        /**
         ** Parse the label
         **/

    rewind(fp);
        /* memset(h, '\0', sizeof(struct _iheader)); */
    iom_init_iheader(h);

        /* err_file = msg_file; */
    if (iom_is_ok2print_details()){
        err_file = msg_file;
    }
    else {
#ifdef _WIN32
        err_file = (char *)"nul:";
#else
        err_file = (char *)"/dev/null";
#endif /* _WIN32 */
    }

    ob = (OBJDESC *)OdlParseLabelFile(filename, err_file,
                                      ODL_EXPAND_STRUCTURE, iom_VERBOSITY <= 3);

    if (!ob || (key = OdlFindKwd(ob, "RECORD_BYTES", NULL, 0, 0)) == NULL) {
        OdlFreeTree(ob);
        ob = NULL;
        if (iom_is_ok2print_errors()){
            fprintf(stderr, "%s is not a PDS file.", filename);
        }
        return(0);
    } else {
        rlen = atoi(key->value);
    }

        /**
         ** Check if this is an ISIS QUBE first.  If not, check for an IMAGE
         ** Fri Mar 29 15:28:44 MST 2002: We're now dealing with SPECTAL_QUBE
         **		in addition.
         **/
    qube = NULL;
    qube = OdlFindObjDesc(ob, (char *)"QUBE", NULL, 0, 0, 0);
    if (qube == NULL)
        qube = OdlFindObjDesc(ob, (char *)"SPECTRAL_QUBE", NULL, 0, 0, 0);
    
    if (qube != NULL) {
        
            /**
             ** Get data organization
             **/
        
        org = -1;
        if ((key = OdlFindKwd(qube, "AXES_NAME", NULL, 0, scope)) ||
            (key = OdlFindKwd(qube, "AXIS_NAME", NULL, 0, scope))) {
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
        
        key2 = OdlFindKwd(qube, "CORE_ITEM_BYTES", NULL, 0, scope);
        
            /**
             ** This tells us if we happen to be using float vs int
             **/
        
        key1 = OdlFindKwd(qube, "CORE_ITEM_TYPE", NULL, 0, scope);
        
        format = iom_ConvertISISType(key1 ? key1->value : NULL,
                                     NULL,
                                     key2 ? key2->value : NULL);

        if (format == iom_EDF_INVALID){
            if (iom_is_ok2print_unsupp_errors()){
                fprintf(stderr,
                        "%s has unsupported/illegal SIZE+TYPE combination.\n",
                        filename);
            }
            OdlFreeTree(ob);
            ob = NULL;
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

        if (iom_is_ok2print_details()){
            fprintf(stderr, "ISIS Qube: ");
            fprintf(stderr, "%dx%dx%d %s %s\t\n",
                    iom_GetSamples(size, org),
                    iom_GetLines(size, org),
                    iom_GetBands(size, org),
                    iom_Org2Str(org), iom_EFormat2Str(format));
            
                /*************************************************************/
                /* THE FOLLOWING INFO MUST BE PLACED INTO THE HEADER SOMEHOW */
                /*************************************************************/
            
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
        h->eformat = (iom_edf)format;
        h->offset = offset;
        h->gain = gain;
        h->prefix[0] = h->prefix[1] = h->prefix[2] = 0;
        h->suffix[0] = suffix_size[0];
        h->suffix[1] = suffix_size[1];
        h->suffix[2] = suffix_size[2];
        h->corner = (size_t)suffix[0]*(size_t)suffix[1]*(size_t)suffix_bytes;

            /*
            ** Fri Mar 29 15:44:18 MST 2002: We've added ^SPECTRAL_QUBE as
            ** as object pointer
            */

        key = NULL;
        key = OdlFindKwd(ob, "^QUBE", NULL, 0, 0);
        if (key == NULL)
            key = OdlFindKwd(ob, "^SPECTRAL_QUBE", NULL, 0, 0);

        
        if (key != NULL) {
            tempString = OdlGetFileName(key, &start, &start_type);
            if(NULL != tempString) {
                free(tempString);
            }
            if (start_type == ODL_RECORD_LOCATION) {
                start = (start-1)*rlen;
            }
            h->dptr = start;
        }
        if (r_obj != NULL) {
            *r_obj = ob;
        } else {
            OdlFreeTree(ob);
            ob = NULL;
        }
        return(1);
    } else {
        if ((image = OdlFindObjDesc(ob, (char *)"IMAGE", NULL, 0, 0, 0)) != NULL) {
        
            if ((key = OdlFindKwd(image, (char *)"LINE_SAMPLES", NULL, 0, scope))) {
                size[0] = atoi(key->value);
            }
            if ((key = OdlFindKwd(image, (char *)"LINES", NULL, 0, scope))) {
                size[1] = atoi(key->value);
            }
            if ((key = OdlFindKwd(image, (char *)"BANDS", NULL, 0, scope))) {
                size[2] = atoi(key->value);
            }
		/*
		** Thu Dec  1 14:43:00 MST 2005 - NsG
		** 
		** This is all new code because we finally encountered images with
		** multiple bands.
		**
		** In the case of non-BSQ images, we need to manually reorganize the
		** indices to match the organization.
		**/
            if ((key = OdlFindKwd(image, "BAND_STORAGE_TYPE", NULL, 0, scope))) {
                if (!strcasecmp(key->value, "BAND_SEQUENTIAL")) {
                    org = iom_BSQ;
                        /* 0 = x, 1 = y, 2 = z */
                        /* No switching */
                } else if (!strcasecmp(key->value, "LINE_INTERLEAVED")) {
                    org = iom_BIL;
                        /* 0 = x, 1 = z, 2 = y */
                    t = size[1];
                    size[1] = size[2];
                    size[2] = t;
                } else if (!strcasecmp(key->value, "SAMPLE_INTERLEAVED")) {
                    org = iom_BIP;
                        /* 0 = z, 1 = x, 2 = y */
                    t = size[0];
                    size[0] = size[2];
                    size[2] = size[1];
                    size[1] = t;
                }
            } else {
                    /* probable fix here.  Org wasn't getting set at all */
                org = iom_BSQ;
            }
        
            key2 = OdlFindKwd(image, "SAMPLE_BITS", NULL, 0, scope);
            key1 = OdlFindKwd(image, "SAMPLE_TYPE", NULL, 0, scope);
        
            format = iom_ConvertISISType( key1 ? key1->value : NULL,
                                          key2 ? key2->value : NULL, 
                                          NULL);
        
            if (format == iom_EDF_INVALID){
                if (iom_is_ok2print_unsupp_errors()){
                    fprintf(stderr,
                            "%s has unsupported/illegal SIZE+TYPE combination.\n",
                            filename);
                }
                OdlFreeTree(ob);
                ob = NULL;

                return 0;
	    }


            if ((key = OdlFindKwd(image, "LINE_PREFIX_BYTES", NULL, 0, scope))) {
                line_prefix = atoi(key->value);
            }
        
                /**
                 ** Load important IMAGE values
                 **/
            if ((key = OdlFindKwd(ob, "^IMAGE", NULL, 0, 0)) != NULL) {
          
                    /*
                    ** If the actual image data is located in a detached
                    ** data file, get that file name. Save this name in 
                    ** the header for future retrieval of data.
                    */
          
		  
                ddfname = OdlGetFileName(key, &start, &start_type);
                if (strcmp(ddfname, filename)) {
                    char *p;
		  	/*
			** If the filename is different, then it's a detached label
			*/
                    if ((p = strrchr(filename, '/')) != NULL) {
                        char *q;
                
                        q = calloc(strlen(ddfname)+(p-filename+1)+2, sizeof(char));
                        strncpy(q, filename, (p-filename+1));
                        strcat(q, "/"); strcat(q, ddfname);
                
                        if(NULL != ddfname) {
                            free(ddfname);
                            ddfname = NULL;
                        }
                        ddfname = q;
                    } else {
                            /* no directory specified in filename, just use ddfname */
                    }
                }
          
                if (start_type == ODL_RECORD_LOCATION) {
                    start = (start-1)*rlen;
                    h->dptr = start;
                } 
            }
            if (r_obj != NULL) {
                *r_obj = ob;
            } else {
                OdlFreeTree(ob);
                ob = NULL;
            }
        
            h->org = org;
            h->size[0] = size[0];
            h->size[1] = size[1];
            h->size[2] = size[2];
            h->eformat = (iom_edf)format;
            h->format = iom_Eformat2Iformat(h->eformat);
            h->offset = 0;
            h->gain = 0;
            h->prefix[0] = line_prefix;
            h->prefix[1] = h->prefix[2] = 0;
            h->suffix[0] = h->suffix[1] = h->suffix[2] = 0;
            h->ddfname = ddfname;
            return(1);
        }
    }
    if((NULL != ob) && (*r_obj != ob)) {
        OdlFreeTree(ob);
        ob = NULL;
    }
    return(0);
}


int
iom_WriteISIS(
    char *fname,
    void *data,
    struct iom_iheader *h,
    int force_write,
    char *title
    )
{
    size_t dsize;
    size_t fsize;
    char buf[1025];
    FILE *fp = NULL;

    if (!force_write && access(fname, F_OK) == 0){
        if (iom_is_ok2print_errors()){
            fprintf(stderr, "File %s exists already.\n", fname);
        }
        return 0;
    }

    if ((fp = fopen(fname, "wb")) == NULL){
        if (iom_is_ok2print_sys_errors()){
            fprintf(stderr, "Unable to write file %s. Reason: %s.\n",
                    fname, strerror(errno));
        }
        return 0;
    }
    
    dsize = iom_iheaderDataSize(h); /* >>> I GUESS <<< */
    fsize = dsize/512+1;

        /*
        ** FOLLOWING CODE NEEDS TO BE AUGMENTED TO INCLUDE FLOATING POINT
        ** DATA FILE I/O.
        */

    sprintf(buf, "PDS_VERSION = 3.0\r\n");
    sprintf(buf+strlen(buf), "RECORD_TYPE = FIXED_LENGTH\r\n");
    sprintf(buf+strlen(buf), "RECORD_BYTES = 512\r\n");
    sprintf(buf+strlen(buf), "FILE_RECORDS = %ld\r\n",fsize+2);
    sprintf(buf+strlen(buf), "LABEL_RECORDS = 2\r\n");
    sprintf(buf+strlen(buf), "FILE_STATE = CLEAN\r\n");
    sprintf(buf+strlen(buf), "^QUBE = 3\r\n");
    sprintf(buf+strlen(buf), "OBJECT = QUBE\r\n");
    sprintf(buf+strlen(buf), "    AXES = 3\r\n");
    if (h->org == iom_BIL) {
        sprintf(buf+strlen(buf), "    AXES_NAME = (SAMPLE,BAND,LINE)\r\n");
    } else if (h->org == iom_BSQ) {
        sprintf(buf+strlen(buf), "    AXES_NAME = (SAMPLE,LINE,BAND)\r\n");
    } else {
        sprintf(buf+strlen(buf), "    AXES_NAME = (BAND,SAMPLE,LINE)\r\n");
    }
    sprintf(buf+strlen(buf), "    CORE_ITEMS = (%d,%d,%d)\r\n",
	    h->size[0], h->size[1], h->size[2]);
    sprintf(buf+strlen(buf), "    CORE_ITEM_BYTES = %d\r\n", 
            iom_NBYTESI(h->format));

        /* Always output native-endian data. */

#ifdef WORDS_BIGENDIAN
    switch (h->format) {
        case iom_BYTE:
        case iom_SHORT:
        case iom_INT:
            sprintf(buf+strlen(buf), "    CORE_ITEM_TYPE = SUN_INTEGER\r\n");
            break;
        case iom_FLOAT:
        case iom_DOUBLE:
            sprintf(buf+strlen(buf), "    CORE_ITEM_TYPE = SUN_REAL\r\n");
            break;
    }

#else

    switch (h->format) {
        case iom_BYTE:
        case iom_SHORT:
        case iom_INT:
            sprintf(buf+strlen(buf), "    CORE_ITEM_TYPE = PC_INTEGER\r\n");
            break;

        case iom_FLOAT:
        case iom_DOUBLE:
            sprintf(buf+strlen(buf), "    CORE_ITEM_TYPE = PC_REAL\r\n");
            break;
    }
#endif /* WORDS_BIGENDIAN */
    
    sprintf(buf+strlen(buf), "    CORE_BASE = 0.0\r\n");
    sprintf(buf+strlen(buf), "    CORE_MULTIPLIER = 1.0\r\n");
    sprintf(buf+strlen(buf), "    CORE_NAME = RAW_DATA_NUMBERS\r\n");
    sprintf(buf+strlen(buf), "    CORE_UNIT = DN\r\n");
    sprintf(buf+strlen(buf), "    OBSERVATION_NOTE = \"%s\"\r\n",
            (title ? title : "BLANK"));
    sprintf(buf+strlen(buf), "END_OBJECT = QUBE\r\n");
    sprintf(buf+strlen(buf), "END\r\n");
    memset(buf+strlen(buf), ' ', 1024-strlen(buf));

        /* write header to file */
    fwrite(buf, 1, 1024, fp);

        /*
        ** Write data to file.
        ** We don't need to byte-swap data because it is written in
        ** the machine's native format.
        */
    fwrite(data, iom_NBYTESI(h->format), dsize, fp);

    if (ferror(fp)){
        if (iom_is_ok2print_sys_errors()){
            fprintf(stderr, "Error writing to file %s. Reason: %s.\n",
                    fname, strerror(errno));
        }
        fclose(fp);
        unlink(fname);
        return 0;
    }
    
    fclose(fp);

    return(1); /* success */
}




/*
** ConvertISISType()
**
** Returns one of the formats given in dvio_edf as per
** the specified "type", "bits", and "bytes." If both
** "bits" and "bytes" are specified, "bits" value is
** given preference over the "bytes" value.
**
** Returns EDF_INVALID on invalid format.
*/
iom_edf
iom_ConvertISISType(char *type, char * bits, char *bytes)
{
    int item_bytes = 0;
    
    if (bits){
        switch(atoi(bits)){
            case 8: item_bytes = 1; break;
            case 16: item_bytes = 2; break;
            case 32: item_bytes = 4; break;
            case 64: item_bytes = 8; break;
        }
    }
    else if (bytes){
        item_bytes = atoi(bytes);
    }

	return iomConvertISISType(type, item_bytes);
}


iom_edf
iomConvertISISType(char *type, int item_bytes)
{
    int format = iom_EDF_INVALID; /* Assume invalid format to start with */
    char *q;
    
    q = type;
    
    if ((!strcmp(q, "INT")) || 
        (!strcmp(q,"UNSIGNED_INTEGER")) || 
        (!strcmp(q,"INTEGER"))){
        switch(item_bytes){
#ifdef WORDS_BIGENDIAN
            case 1: format = iom_MSB_INT_1; break;
            case 2: format = iom_MSB_INT_2; break;
            case 4: format = iom_MSB_INT_4; break;
#else /* little endian */
            case 1: format = iom_LSB_INT_1; break;
            case 2: format = iom_LSB_INT_2; break;
            case 4: format = iom_LSB_INT_4; break;
#endif /* WORDS_BIGENDIAN */
        }
    }
    else if (!strcmp(q, "SUN_INTEGER") ||
             (!strcmp(q,"SUN_UNSIGNED_INTEGER")) || 
             (!strcmp(q,"MSB_UNSIGNED_INTEGER")) || 
             (!strcmp(q,"MSB_SIGNED_INTEGER")) ||
             (!strcmp(q,"MSB_INTEGER"))) {
        switch(item_bytes){
            case 1: format = iom_MSB_INT_1; break;
            case 2: format = iom_MSB_INT_2; break;
            case 4: format = iom_MSB_INT_4; break;
        }
    }
    else if (!strcmp(q, "PC_INTEGER") ||
             (!strcmp(q,"PC_UNSIGNED_INTEGER")) || 
             (!strcmp(q,"LSB_UNSIGNED_INTEGER")) || 
             (!strcmp(q,"LSB_SIGNED_INTEGER")) ||
             (!strcmp(q,"LSB_INTEGER")) ||
             (!strcmp(q, "VAX_INT")) ||
             (!strcmp(q,"VAX_INTEGER"))) {
        switch(item_bytes){
            case 1: format = iom_LSB_INT_1; break;
            case 2: format = iom_LSB_INT_2; break;
            case 4: format = iom_LSB_INT_4; break;
        }
    }
    else if (!strcmp(q, "REAL")){
        switch(item_bytes){
#ifdef WORDS_BIGENDIAN
            case 4: format = iom_MSB_IEEE_REAL_4; break;
            case 8: format = iom_MSB_IEEE_REAL_8; break;
#else /* little endian */
            case 4: format = iom_LSB_IEEE_REAL_4; break;
            case 8: format = iom_LSB_IEEE_REAL_8; break;
#endif /* WORDS_BIGENDIAN */
        }
    }
    else if (!strcmp(q, "SUN_REAL") || !strcmp(q, "IEEE_REAL")){
        switch(item_bytes){
            case 4: format = iom_MSB_IEEE_REAL_4; break;
            case 8: format = iom_MSB_IEEE_REAL_8; break;
        }
    }
    else if (!strcmp(q, "PC_REAL") || !strcmp(q, "RIEEE_REAL")){
        switch(item_bytes){
            case 4: format = iom_LSB_IEEE_REAL_4; break;
            case 8: format = iom_LSB_IEEE_REAL_8; break;
        }
    }
    else if (!strcmp(q, "VAX_REAL")) {
        switch(item_bytes){
            case 4: format = iom_VAX_REAL_4; break;
        }
    }
    return(format);
}
