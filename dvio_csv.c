#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <float.h>
#include <unistd.h>

#ifndef _WIN32
#include <sys/mman.h>
#else
#endif /* _WIN32 */

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include <fcntl.h>
#include <ctype.h>
#include "dvio.h"
#include "csv.h"

typedef unsigned char uchar;


#define MAX(a,b) (((a) > (b))? (a): (b))
#define MIN(a,b) (((a) < (b))? (a): (b))


//#define FF_LOADCSV_DEBUG 1

/** datatype used in coldef*/
// TODO(rswinkle) why separate macros?  why not just the stardard ones in parser.h?
typedef enum {
    TBYTE,
    TSHORT,
    TINT,
    TFLOAT,
    TDOUBLE,
    TSTR
} coltype;

struct _coldef;

/** Contains column information */
typedef struct _coldef {
    /* column type extraction data */
    char     *name;            /**< probable name */
    char     *text;            /**< text as read from the file */
    int      sno;              /**< serial number */
    coltype  data_type;        /**< data type */
    int      max_len;          /**< max text string length */
    int      size;             /**< size (in bytes) for the type */
    int      naflag;           /**< data for this field contains an N/A */

    /* cached data */
    int      dav_type;         /**< davinci type of this column */

    /* array specific data, for structure extraction */
    struct   _coldef  *prev;   /**< chain for array processing */
    struct   _coldef  *next;   /**< chain for array processing */
    int      chain_len;        /**< chain-length at the "root"-level only */
} coldef;                     /* column definition */



/**
    Contains meta data.
    Passed to callback functions to be updated as data is parsed.
*/
typedef struct _calldata {
    int      cur_field;     ///<current field being parsed (0 based)
    coldef*  coldefs;       ///<point to array of coldefs so I can access/modify type/size etc. in callback functions
    coldef** fields;        ///<point to root-level fields used in pass 2
    int      nfields;       ///<number of root-level fields use in pass 2
    char**   vals;          ///<fields copied into here for conversion in pass2
    int      colsize;       ///<number of columns allocated in coldefs (reallocation happens in cb1 if needed)

    int      header;        ///<whether 1st row is column names
    int      collapse;      ///<whether to collapse consecutive field separators
    int      nrecs;         ///<current record (0 based)
    int      ncols;         ///<max number of columns seen. Used to resize coldefs.

    void     **data_ptrs;   ///<used in pass2 to copy data into (pointers are set up previously)

    int      error;         ///<set when error occurs in callback functions
} calldata;



//used for write
static int print_headers(Var** data, char** keys, int count, FILE* file, char* separator);



static int make_coldefs(char **colnames, coldef **coldefs, int n);
static void free_coldefs(coldef *cols, int n);
static void init_coldefs(coldef* cols, int num);



/**
** Fix the "size" field of coldef structure for each of the column
** definitions based on the types recognized by davinci.
    @param cols [in|out] coldef array.
    @param n [in] number of columns.
*/
static void
fill_col_min_size(
    coldef   *cols,
    int      n
    )
{
    int      i;

    for(i=0; i<n; i++){
        switch (cols[i].data_type) {
        case TBYTE:     cols[i].size = sizeof(char);    break;
        case TSHORT:    cols[i].size = sizeof(short);   break;
        case TINT:      cols[i].size = sizeof(int);     break;
        case TFLOAT:    cols[i].size = sizeof(float);   break;
        case TDOUBLE:   cols[i].size = sizeof(double);  break;

        default:
            /* well in this case, the length is the maximum length */
            cols[i].size = cols[i].max_len;
            break;
        }

    }
}




/**
** Return the davinci type based on the column type and size pair.
    @param c [in] column type.
*/
static int
davinci_type(
    coltype  c       /* column type: TINT, TFLOAT, ... */
    )
{
    int      dav_type;

    switch(c) {
        case TBYTE:     dav_type = BYTE;        break;
        case TSHORT:    dav_type = DV_INT16;       break;
        case TINT:      dav_type = INT;         break;
        case TFLOAT:    dav_type = FLOAT;       break;
        case TDOUBLE:   dav_type = DOUBLE;       break;

        case TSTR:
        default:
            /* does not apply otherwise */
            dav_type = -1;
            break;
    }

    return dav_type;
}


/**
** Guess the structure based on the column definitions passed in.
**
** Columns with same name are considered as members of the same
** array with their order preserved from left to right.
**
** Array element columns are chained off of the first array element
** column, called the "root"-element or "root"-column.
**
** A list of "root"-columns is returned in LIST "l" and their
** number is returned as the function return value. If the function
** return value is less than zero then an error has occurred.

   Example:
     If the input columns were:

               "sclk_time temps[1] temps[2] det temps[3]"

     then the LIST *l should be arranged as follows:

     l[0] = coldef<"sclk_time">

     l[1] = coldef<"temps">
            + l[1]->next points to chain of coldef<"temps">
            each of l[1]->next's is an element of the array "temps"

     l[2] = coldef<"det">



    @param cols [in|out] array of all columns
    @param n [in] number of columns in cols.
    @param l [out] root columns returned here.

*/
static int
guess_struct(
    coldef   *cols,
    int      n,
    coldef   ***l
    )
{
    int      i, j;
    int      nfields;
    coldef   *t;

    /* Chain array fields (ie coldefs) together based on whether the titles match.
        for example data[1] data[5] data[6] would be linked together with next/prev
        pointer members of coldef.  data[1] would be head of chain.
        No field type promotion or column length determination is done here. */
    for(i = 0; i < n; i++){

        for(j = i+1; j < n; j++){

            #ifdef FF_LOADCSV_DEBUG
            fprintf(stderr, "%s\t%s\n", cols[i].name, cols[j].name);
            #endif

            if (strcmp(cols[i].name, cols[j].name) == 0){

                cols[i].next = &cols[j];
                cols[j].prev = &cols[i];

                /*
                ** One name match is found, stop processing this i'th column and
                ** move to the next. This break is necessary for preserving
                ** chains of array columns.
                */
                break;
            }
        }
    }

    *l = (coldef **)calloc(n, sizeof(coldef *));
    if (*l == NULL){
        return 0;
    }

    /* count number of root level fields and determine their davinci type and size.*/
    nfields = 0;
    for(i = 0; i < n; i++){
        /*
        ** if prev pointer set, then this record belongs to an
        ** array chain, so skip it, as it has already been considered
        */
        if (cols[i].prev) continue;

        /*
        ** Fill chain-length in the column definition
        ** this value is the array dimension for the field
        ** under consideration.
        **
        ** Also determine final field size and type based on
        ** the elements of the field (if it is an array field)
        */

        cols[i].chain_len = 1;

        for (t = cols[i].next; t; t = t->next){
            cols[i].chain_len++;
            cols[i].data_type = MAX(t->data_type, cols[i].data_type);
            cols[i].size = MAX(t->size, cols[i].size);
        }

        for (t = cols[i].next; t; t = t->next){
            t->data_type = cols[i].data_type;
            t->size = cols[i].size;
        }

        /* save davinci type of the column for future use */
        cols[i].dav_type = davinci_type(cols[i].data_type);

        /* add this field to the list of "root"-level fields */
        (*l)[nfields++] = &cols[i];
    }

    /* pack the structure list */
    *l = (coldef **)realloc(*l, nfields * sizeof(coldef *));

    return nfields;
}


/**
    Preallocates storage space for the davinci struct and its
    the element data to be returned.
    @param fields [in] array of root level coldefs
    @param nfields [in] number of fields (columns).
    @param nrecs [in] number of rows.
    @return pointer to data structure or NULL on error.
*/
static Var*
alloc_davinci_data_space(
    coldef   **fields,      /* list of "root"-"coldef"s */
    int      nfields,
    int      nrecs       /* total number of input records */
    )
{

    int      i, j;
    int      cl;         /* chain length: number of array elements */
    int      dt;         /* davinci type corresponding to (Ttype, byte-size) */
    Var      *v_return;  /* Returned davinci variable */
    Var      *v_t;
    void     *tdata;
    size_t   mem_size;

    /* create a blank structure */
    v_return = new_struct(0);



    for(i=0; i<nfields; i++){
        cl = fields[i]->chain_len;

        switch(fields[i]->data_type) {
        case TBYTE:
        case TSHORT:
        case TINT:
        case TFLOAT:
        case TDOUBLE:
            mem_size = (size_t)nrecs* (size_t)cl * (size_t)fields[i]->size;

            tdata = malloc(mem_size);
            if( tdata == NULL ) {
                memory_error(errno, mem_size);
                return NULL;
            }

            dt = fields[i]->dav_type;
            v_t = newVal(BSQ, cl, nrecs, 1, dt, tdata);
            break;

        case TSTR:
            /*
            ** since davinci's text data-type can only handle
            ** one dimension, i.e. lines (of text), we will end
            ** up concatenating the text array elements together
            ** with spaces in-between.
            */
            mem_size = (size_t)nrecs*(size_t)sizeof(char*);
            tdata = malloc(mem_size);
            if( tdata == NULL ) {
                memory_error(errno, mem_size);
                return NULL;
            }

            v_t = newText(nrecs, tdata);
            for(j=0; j<nrecs; j++){
                /* data = cl*fields[i]->size + spaces = (cl - 1) + '\0' = 1 */
                mem_size = ((size_t)cl*(size_t)fields[i]->size)+cl;
                ((char **)tdata)[j] = calloc(1, mem_size);

                if( ((char**)tdata)[j] == NULL ) {
                    memory_error(errno, mem_size);
                    return NULL;
                }

            }
            break;

        default:
            fprintf(stderr,
                    "Unhandled type %d encontered encountered. Fix %s:%d.\n",
                    fields[i]->data_type, __FILE__, __LINE__);
            return NULL;
        }
        add_struct(v_return, fields[i]->name, v_t);
    }

    return v_return;
}



/** Returns the number of base-10 digits in a positive integer. */
static int
num_digits(int number)
{
    int len = 1;

    while( number /= 10 )
        len++;

    return len;
}



static int
is_overflow_double(double d){
    return d == HUGE_VAL || d == -HUGE_VAL;
}

/**
    Field separator callback function used in first pass through data.
    It determines data type of current field and update
    column info (length, type etc.) as necessary during the first pass through the data.
    @param s [in] field data as read from file, and preprocessed by libcsv to remove
             special characters.
    @param len [in] length of field s.
    @param call_data [in|out] data structure with meta data.
    @see csv.h
    @see libcsv.c
    @see calldata
*/
static void
pass1_cb_fs(void *s, size_t len, void *call_data)
{
    int i;
    long itemp = 0;
    double dtemp = 0;
    float ftemp = 0;
    char *end = NULL;
    int type_found = 0;

    char temp_char;
    char* ctemp = NULL;
    coldef* coltemp = NULL;
    size_t mem_size;

    calldata* data = (calldata*)call_data;


     char* nan = "N/A";

#ifdef FF_LOADCSV_DEBUG
    printf("\"");
    for(i=0; i<len; i++)
        printf("%c", ((char*)s)[i]);
    printf("\"_ ");
#endif

    //skip blanks if collapse is 1 and always collapse blanks in header if header==1
    if( len==0 && (data->collapse || (data->nrecs==0 && data->header) ) )
        return;

    //if this is one more column than we've seen before
    if( data->cur_field == data->ncols ) {
        data->ncols++;

        //handles case of more columns than headers seen or created
        if( data->nrecs != 0 ) {
            mem_size = (num_digits(data->ncols)+2)*sizeof(char);
            if( (ctemp = (char*)malloc(mem_size))==NULL) {
                memory_error(errno, mem_size);
                data->error = 1;
                return;
            }

            snprintf(ctemp, num_digits(data->ncols)+2, "c%d", data->ncols);
            data->coldefs[data->cur_field].name = ctemp;
            #ifdef FF_LOADCSV_DEBUG
                fprintf(stderr, "*%s\n", data->coldefs[data->cur_field].name);
            #endif
        }

        /* expand coldef array if necessary */
        if( data->cur_field >= data->colsize ) {

            i=0;
            #ifdef FF_LOADCSV_DEBUG
                while( data->coldefs[i].name != NULL && i<=data->cur_field )
                    fprintf(stderr, "%s\t", data->coldefs[i++].name);
            #endif

            data->colsize *= 2;
            mem_size = data->colsize*sizeof(coldef);
            if( (data->coldefs = (coldef*)realloc(data->coldefs, mem_size)) == NULL ) {
                memory_error(errno, mem_size);
                data->error = 1;
                return;
            }

            init_coldefs(&data->coldefs[data->colsize/2], data->colsize/2);

            i=0;
            #ifdef FF_LOADCSV_DEBUG
                while( data->coldefs[i].name != NULL && i<=data->cur_field )
                    fprintf(stderr, "%s\t", data->coldefs[i++].name);
            #endif
        }
    }

    //if len=0 the type and length don't change . . .
    //but what if there's a blank field in the header row? not sure.
    //Maybe I'll just always collapse blanks in header row (if header=1 of course)
    if( len==0 && data->nrecs!=0 ) {
        data->cur_field++;
        return;
    }

    /* first line may be headers */
    if( data->nrecs==0 ) {
        if( data->header ) {
            if( (ctemp = (char*)calloc(len+1, sizeof(char)) )==NULL) {
                memory_error(errno, sizeof(char)*(len+1));
                data->error = 1;
                return;
            }

            memcpy(ctemp, s, len);
            data->coldefs[data->cur_field].name = ctemp;
            #ifdef FF_LOADCSV_DEBUG
                fprintf(stderr, "*%s\n", data->coldefs[data->cur_field].name);
            #endif
            data->cur_field++;

            return;
        } else {
            if( (ctemp = (char*)calloc(num_digits(data->ncols)+2, sizeof(char)) )==NULL) {
                memory_error(errno, sizeof(char)*(num_digits(data->ncols)+2) );
                data->error = 1;
                return;
            }

            snprintf(ctemp, num_digits(data->ncols)+2, "c%d", data->ncols);
            data->coldefs[data->cur_field].name = ctemp;
            #ifdef FF_LOADCSV_DEBUG
                fprintf(stderr, "*%s\n", data->coldefs[data->cur_field].name);
            #endif
        }
    }

    /* update max_len of field (used if field turns out to be string type) */
    data->coldefs[data->cur_field].max_len = MAX(data->coldefs[data->cur_field].max_len, len);


    /* determine field type and update column type */
    char* schar = (char*)s;
    temp_char = schar[len];
    schar[len] = '\0';
    itemp = strtol(s, &end, 10);

    if(itemp<=INT_MAX && itemp >= INT_MIN && end>=schar+len) {
        if( itemp>=0 && itemp<=UCHAR_MAX ) {
            ;
        } else if( itemp<=SHRT_MAX && itemp>=SHRT_MIN ) {
            data->coldefs[data->cur_field].data_type = MAX(data->coldefs[data->cur_field].data_type, TSHORT);
        } else {
            data->coldefs[data->cur_field].data_type = MAX(data->coldefs[data->cur_field].data_type, TINT);
        }

        type_found = 1;
    }


/*
 * drd
 * [Bug 2169] load_csv() does not promote floating point data to doubles
 *
 * Saadat suggests the use of floats where possible.
 * If the float can represent the number to within a tolerance, then
 * use float, if not, use a double.
 * Where we often see large numbers is with sclk times.
 * These we like to keep within 1 second.  Thus I am picking 0.5
 * as the tolerance.  If the float and double differ more than this,
 * I make the value a double. If the float and the double are within 0.5,
 * I make the value a float.
 *
 *
 */

#undef DOUBLEFLOATDEBUG

    if( !type_found ) {
#ifdef DOUBLEFLOATDEBUG
    	static int countf = 0;
    	static int countd = 0;
#endif
        ftemp = strtof(s, &end);
        if( ftemp!=HUGE_VAL && ftemp!=-HUGE_VAL && end>=schar+len) {
        	type_found = 1;
#ifdef DOUBLEFLOATDEBUG
        	countf++;
#endif
        	data->coldefs[data->cur_field].data_type = MAX(data->coldefs[data->cur_field].data_type, TFLOAT);
        	dtemp = strtod(s, &end);
        	if( !is_overflow_double(dtemp) && end>=schar+len) {
     			 if(fabs( dtemp - ftemp) > 0.5) {  // drd here is the 0.5--note that fabs() promotes to and then compares as double
     				                               // Again, 0.5 picked to keep sclk vals within 1, or +0.5/-0.5
     				 data->coldefs[data->cur_field].data_type = MAX(data->coldefs[data->cur_field].data_type, TDOUBLE);
#ifdef DOUBLEFLOATDEBUG
     				 countd++;
        			 printf("The diff is %f\n", fabs(dtemp-ftemp));
        			 printf("countf = %d, countd = %d\n", countf, countd);
#endif
        		 }
        	}
        }
    }

/*
 * There is a small chance that the number is too large for a float just above here.
 * Then the double would never be made.  So we (possibly) test one more time here
 * for double, and if this fails, we promote to TSTR, string
 */

    if( !type_found ) {
        dtemp = strtod(s, &end);

        if( !is_overflow_double(dtemp) && end>=schar+len) {
            data->coldefs[data->cur_field].data_type = MAX(data->coldefs[data->cur_field].data_type, TDOUBLE);
            type_found = 1;
        } else {
            data->coldefs[data->cur_field].data_type = TSTR;
        }
    }


    schar[len] = temp_char;
    /* update calldata */
    data->cur_field++;
}

/**
    Record separator call back function used during the first pass through data.
    It initializes data for next record.
    @param c [in] character caused callback (not used here).
    @param call_data [in|out] data structure of meta data.
*/
static void
pass1_cb_rs(int c, void *call_data)
{
    calldata* data = (calldata*)call_data;

    #ifdef FF_LOADCSV_DEBUG
        printf("\n");
    #endif

    data->nrecs++;
    data->cur_field = 0;
}


/**
    Initialize coldef structures.
    @param cols [in|out] array of coldefs.
    @param num [in] number of cols.
*/
static void
init_coldefs(coldef* cols, int num)
{
    int i;

    for(i=0; i<num; i++) {
        cols[i].name = NULL;
        cols[i].data_type = TBYTE; /* start as "BYTE", promote if necessary */
        cols[i].max_len = 0;
        cols[i].text = NULL;
        cols[i].next = NULL;
        cols[i].prev = NULL;
        //cols[i].max_num_val = 0;
        cols[i].size = 0;
    }
}



/**
    First pass through data: figures out column data types and preps data to be copied.
    @param file [in] file data.
    @param file_size [in] size of file.
    @param p [in] csv_parser used to parse data.
    @param header [in] whether first row is column names.
    @param collapse [in] whether to collapse consecutive field separators.
    @param columns [out] array of coldef structures returned (contain column info).
    @param numcols [out] size of columns returned.

*/
static int
pass1_types(
    char*               file,
    size_t              file_size,
    struct csv_parser*  p,
    int                 header,
    int                 collapse,
    coldef**            columns,
    int*                numcols
    )
{


    int ncols = 30;
    coldef* cols = NULL;
    int i;

    if( (cols = (coldef*)calloc(ncols, sizeof(coldef))) == NULL ) {
        memory_error(errno, sizeof(coldef)*ncols);
        return -1;
    }
    init_coldefs(cols, ncols);

    calldata pdata;

    pdata.cur_field = 0;
    pdata.coldefs = cols;
    pdata.colsize = ncols;
    pdata.header = header;
    pdata.collapse = collapse;
    pdata.nrecs = 0;
    pdata.ncols = 0;
    pdata.error = 0;



    if (csv_parse(p, file, file_size, pass1_cb_fs, pass1_cb_rs, &pdata) != file_size) {
        fprintf(stderr, "Error parsing file: %s\n", csv_strerror(csv_error(p)));
        return -1;
    }

    if( pdata.error ) {
        free(pdata.coldefs);
        return -1;
    }


    csv_fini(p, pass1_cb_fs, pass1_cb_rs, &pdata);

    for(i=0; i<pdata.ncols; i++)
    	pdata.coldefs[i].sno = i;


    *columns = pdata.coldefs;
    *numcols = pdata.ncols;

    #ifdef FF_LOADCSV_DEBUG
        fprintf(stderr,"\n\n=====\n\n");
        for(i=0; i<ncols; i++)
            fprintf(stderr, "%s\t", (*columns)[i].name);
        printf("pdata.ncols = %d\n", pdata.ncols);
    #endif

    return (header) ? (pdata.nrecs-1) : pdata.nrecs;
}




/**
    Field separator call back function used in second pass through data.
    Copies fields into vals to be used in pass2_cb_rs function to copy into
    davinci data structure.
    @param s [in] field just finished and converted/copied in this function.
    @param len [in] length of field s.
    @param call_data [in|out] data structure with meta data.
    @see csv.h
    @see libcsv.c
    @see calldata
*/
static void
pass2_cb_fs(void *s, size_t len, void *call_data)
{
    calldata* data = (calldata*)call_data;


    if( (data->nrecs==0 && data->header) || (data->collapse && len==0) )
        return;

    int i;
#ifdef FF_LOADCSV_DEBUG
    fprintf(stderr, "%d\"",len);
    for(i=0; i<len; i++)
        fprintf(stderr, "%c", ((char*)s)[i]);
    fprintf(stderr, "\"_ ");
#endif


//     unsigned char temp = ((unsigned char*)s)[len];
     ((char*)s)[len] = '\0';
//     if( !(data->vals[data->cur_field++] = strdup((char*)s)) ) {
//     	fprintf(stderr, "strdup failed!\n");
//     }

    memcpy(data->vals[data->cur_field++], (char*)s, len+1);
    //strcpy(data->vals[data->cur_field++], (char*)s);

//     ((unsigned char*)s)[len] = temp;
}



/**
    Record separator call back function used in second pass through data.
    Goes through root level fields converting the field strings in vals to
    appropriate types and copying to correct positions in davinci structure.
    @param c [in] the character that caused the call.
    @param call_data [in|out] data structure containing meta data.
*/
static void
pass2_cb_rs(int c, void *call_data)
{
    calldata* data = (calldata*)call_data;
    int i, j, idx;
    char* s = NULL;
    int recs = (data->header) ? data->nrecs-1 : data->nrecs;
    coldef* t = NULL;
    char** vals = data->vals;
    char *tmps = "";



    unsigned char     **byte_data = (unsigned char **)data->data_ptrs;
    short    **short_data = (short **)data->data_ptrs;
    int      **int_data = (int **)data->data_ptrs;
    float    **float_data = (float **)data->data_ptrs;
    double   **double_data = (double **)data->data_ptrs;
    char     ***str_data = (char ***)data->data_ptrs;




    //make sure empty fields at the end are blank strings
    for(i=data->cur_field; i<data->ncols; i++)
    	vals[i][0] = '\0';//tmps;

    if(data->nrecs > 0 && (data->nrecs%1000) == 0)
        fprintf(stderr, ".");

#ifdef FF_LOADCSV_DEBUG
    fprintf(stderr, "\n\n");
    for(i=0; i<data->ncols; i++)
    	fprintf(stderr, "%s\n", vals[i]);
    fprintf(stderr, "\n\n");
#endif

    if( data->nrecs==0 && data->header ) {
    	data->nrecs++;
        return;
    }

    for(i=0; i<data->nfields; i++) {

    	for(j=0, t=data->fields[i]; t; t=t->next, j++) {
    		idx = recs * data->fields[i]->chain_len + j;

			switch(t->data_type)
			{

			case TBYTE:
				byte_data[i][idx] = atoi(vals[t->sno]);
				break;
			case TSHORT:
				short_data[i][idx] = atoi(vals[t->sno]);
				break;
			case TINT:
				int_data[i][idx] = atoi(vals[t->sno]);
				break;
			case TFLOAT:
				float_data[i][idx] = atof(vals[t->sno]);
				break;
			case TDOUBLE:
				double_data[i][idx] = atof(vals[t->sno]);
				break;

			case TSTR:
                s = str_data[i][recs];

                strcat(s, vals[t->sno]);
                if (t->next != NULL){ strcat(s, " "); }
				//strcpy( ((char***)data->data_ptrs)[i][recs], (char*)s);
				break;

			default:
				fprintf(stderr, "Unknown type\n");
				data->error = 1;
				break;
			}
    	}
    }

//     for(i=0; i<data->cur_field; i++)
//     	free(vals[i]);

    #ifdef FF_LOADCSV_DEBUG
        printf("\n");
    #endif

    data->nrecs++;
    data->cur_field = 0;
}



/**
    Second pass through data: Goes through file converting fields to appropriate
    types and copying them into davinci data structure.
    @param file [in] file data as a char array.
    @param file_size [in] size of file obviously.
    @param p [in] csv_parser object used to parse data.
    @param columns [in] array of coldefs.
    @param data_ptrs [in|out] array of void*'s pointed at previously allocated space in Davinci structure.
    @param header [in] whether 1st row is treated as column names.
    @param collapse [in] whether to collapse consecutive field separators.
    @return returns 1 on success, 0 for failure.
*/
static int
pass2(
    char*   file,
    size_t  file_size,
    struct  csv_parser *p,
    coldef  **columns,
    coldef  *coldefs,
    void    **data_ptrs,
    int     numcols,
    int     numfields,
    int     header,
    int     collapse
)
{
    calldata pdata;
    int i;

    pdata.cur_field = 0;
    pdata.fields = columns;
    pdata.header = header;
    pdata.collapse = collapse;
    pdata.nrecs = 0;
    pdata.error = 0;
    pdata.data_ptrs = data_ptrs;
    pdata.ncols = numcols;
    pdata.nfields = numfields;

    if( !(pdata.vals = (char**)calloc(numcols, sizeof(char*))) ) {
    	memory_error(errno, numcols*sizeof(char*));
    	return 0;
    }

    for(i=0; i<numcols; i++) {
        if( !(pdata.vals[i] = calloc(coldefs[i].max_len+1, sizeof(char))) ) {
            memory_error(errno, (coldefs[i].max_len+1)*sizeof(char));
            return 0;
        }
    }


    if (csv_parse(p, file, file_size, pass2_cb_fs, pass2_cb_rs, &pdata) != file_size) {
        fprintf(stderr, "Error parsing file: %s\n", csv_strerror(csv_error(p)));
        return 0;
    }

    if( pdata.error ) {
        //fprintf(stderr, "Some error in call back function.\n");
        for(i=0; i<numcols; i++)
            free(pdata.vals[i]);
        free(pdata.vals);
        return 0;
    }

    csv_fini(p, pass2_cb_fs, pass2_cb_rs, &pdata);
    for(i=0; i<numcols; i++)
        free(pdata.vals[i]);
    free(pdata.vals);

    return 1;
}






/** Defines what constitues a space character for the parser.
    Only used when user sets fdelim to TAB otherwise the libcsv
    default of treating tabs and spaces as space characters is fine.
    @param c [in]
    @return whether c is the space character.
*/
static int
isaspace(unsigned char c) { return c==' '; }



/**
    Actually loads the csv file.
    @param filename [in] name of file to write to.
    @param fdelim [in] character to uses for field separator.
    @param header [in] whether 1st line is names of columns and not part of data.
    @param collapse [in] whether to collapse consecutive field separators or treat as empty fields.
    @param v_return [out] Var data structure that data is read into and returned in.
    @return 1 on success, 0 on failure.
*/
static int
load_csv(
    char* filename,
    char fdelim,
    int header,
    int collapse,
    Var     **v_return
    )
{
    char* file = NULL;
    char* temp_str = NULL;
    int fd;
    struct   stat sbuf;

    struct csv_parser p;
    void** data_ptrs = NULL;
    void** data = NULL;     //pseudo davinci Var* struct for stand alone testing

    coldef *columns = NULL;
    coldef **fields = NULL;
    int numcols = 0;
    int nrecs = 0;
    int nfields = 0;
    int i;
    Var *d = NULL;
    float gb;
    int mb;
    int bytesInMb = pow(2, 20);

    char * fname = NULL;
    FILE *fp;
    int iscompressed=0;

    *v_return = NULL;

    //check for empty file, return empty struct here

   fname = dv_locate_file(filename);
   if (!file_exists(fname)) {
        parse_error("Unable to expand filename %s\n", filename);
        return 0;
    }

    /*uncompress the file if necessary */
    if (fname && (fp = fopen(fname, "rb")) != NULL) {
      if (iom_is_compressed(fp)) {
        fprintf(stderr, "is compressed\n"); /* FIX: remove */
        fclose(fp);
        fname = iom_uncompress_with_name(fname);
        iscompressed=1;
      }
    }

    if (stat(fname, &sbuf) < 0){
      if(iscompressed) {
        unlink(fname);
      }
      free(fname);
      return 0;
    }

    /* if the file is empty just return a new empty struct */
    if( sbuf.st_size == 0 ) {
        *v_return = new_struct(0);
        if(iscompressed) {
            unlink(fname);
        }
        free(fname);
        return 1;
    }

    if((fd = open(fname, O_RDONLY)) < 0){
      if(iscompressed) {
        unlink(fname);
      }

      free(fname);
      return 0;
    }

    data = mmap(NULL, sbuf.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (data == NULL || data == MAP_FAILED){
    	fprintf(stderr, "\nDavinci cannot allocate sufficient memory to load the file\n\"%s\".\n\n", fname);

    	// Calculate the number of mega bytes...
    	mb = sbuf.st_size / bytesInMb;

    	// Calculate the number of giga bytes...
    	gb = mb >= 1024 ? ((float)mb/1024.0) : 0;

    	if((int)gb > 0)
    	{
    		// If file was found to be greater than 1 GB.

    		fprintf(stderr, "Reason: The size of the file you tried to load is %.1f GB which is greater than the virtual memory of your system.\n\n", gb);
    		fprintf(stderr, "Hint: Use a system with a virtual memory (RAM + SWAP) configuration greater than\n%.1f GB.\n\n", gb);
    	}
    	else
    	{
    		fprintf(stderr, "Reason: The size of the file you tried to load is %d MB which is greater than the virtual memory of your system.\n\n", mb);
    		fprintf(stderr, "Hint: Use a system with a virtual memory (RAM + SWAP) configuration greater than\n%d MB.\n\n", mb);
    	}
      close(fd);
      if(iscompressed) {
        unlink(fname);
      }
      free(fname);
      return 0;
    }

    close(fd);

    csv_init(&p, 0);
    csv_set_delim(&p, fdelim);
    if( fdelim!=',')
        csv_set_space_func(&p, isaspace);



    if( (nrecs = pass1_types((char*)data, sbuf.st_size, &p, header, collapse, &columns, &numcols))<0 ) {
        munmap(data, sbuf.st_size);
        if(iscompressed) {
          unlink(fname);
        }
        free(fname);
        csv_free(&p);
        return 0;
    }

    /* cut off at [ so linking works. */
    for(i=0; i<numcols; i++)
        if ((temp_str = strchr(columns[i].name, '['))) { *temp_str = 0; }

    fill_col_min_size(columns, numcols);

    if( !(nfields = guess_struct(columns, numcols, &fields)) ) {
        free_coldefs(columns, nfields);
        columns = NULL;
        munmap(data, sbuf.st_size);
        if(iscompressed) {
          unlink(fname);
        }
        free(fname);
        csv_free(&p);
        return 0;
        //clean up if necessary (probably inside)
    }


    data_ptrs = (void **)calloc(nfields, sizeof(void *));
    if (data_ptrs == NULL) {
        free_coldefs(columns, numcols);
        columns = NULL;
        free(fields);
        munmap(data, sbuf.st_size);
        if(iscompressed) {
          unlink(fname);
        }
        free(fname);
        csv_free(&p);
        return 0;
    }


    //fix names for davinci
    for(i=0; i<numcols; i++) {
        columns[i].name = fix_name(columns[i].name);
    }


    if( (*v_return = alloc_davinci_data_space(fields, nfields, nrecs)) == NULL ) {
        free_coldefs(columns, numcols);
        columns = NULL;
        free(fields);
        free(data_ptrs);
        munmap(data, sbuf.st_size);
        if(iscompressed) {
          unlink(fname);
        }
        free(fname);
        csv_free(&p);
        return 0;
    }

    //set data_ptrs to point at data correctly here
    for (i = 0; i < nfields; i++){
        get_struct_element(*v_return, i, NULL, &d);
        switch(fields[i]->data_type){
        case TSTR:
            data_ptrs[i] = (void *)V_TEXT(d).text;
            break;

        default:
            data_ptrs[i] = (void *)V_DATA(d);
            break;
        }
    }

    //csv_set_opts(&p, CSV_APPEND_NULL);
    if( !pass2((char*)data, sbuf.st_size, &p, fields, columns, data_ptrs, numcols, nfields, header, collapse) ) {
        free_coldefs(columns, numcols);
        columns = NULL;
        free(fields);
        free(data_ptrs);
        munmap(data, sbuf.st_size);
        if(iscompressed) {
          unlink(fname);
        }
        free(fname);
        csv_free(&p);
        return 0;
    }


    free_coldefs(columns, numcols);
    free(fields);
    free(data_ptrs);
    munmap(data, sbuf.st_size);
    if(iscompressed) {
        unlink(fname);
    }
    free(fname);
    csv_free(&p);

    return 1;
}




/**
    Davinci function called for load_csv.
    @return a davinci structure containing data from csv file.
*/
Var *
ff_loadcsv(
    vfuncptr func,
    Var      *args
    )
{
    char     *filename = NULL;
    Var      *v_return;
    int      rc, header = 1, collapse_fdelim = 0;
    char     *field_delim = "\t";

    Alist    alist[6];
    alist[0] = make_alist( "filename",      ID_STRING,    NULL,        &filename);
    alist[1] = make_alist( "separator",     ID_STRING,    NULL,        &field_delim);
    alist[2] = make_alist( "delimiter",     ID_STRING,    NULL,        &field_delim);
    alist[3] = make_alist( "header",        INT,          NULL,        &header);
    alist[4] = make_alist( "collapse",      INT,          NULL,        &collapse_fdelim);
    alist[5].name = NULL;

    if (parse_args(func, args, alist) == 0) return(NULL);

    if (filename == NULL){
        parse_error("%s(): \"%s\" must be specified.\n",
                    func->name, alist[0].name);
        return NULL;
    }


    if( !load_csv(filename, *field_delim, header, collapse_fdelim, &v_return) )
        return NULL;

    return v_return;
}



/**
 *  Free the coldef array.
    @param cols [in] array of coldef structures to free.
    @param n [in] size of cols (needed to free internal dynamic memory).
 */
static void
free_coldefs(
    coldef   *cols,
    int      n
    )
{
    int   i;

    for(i = 0; i < n; i++){
        if (cols[i].name) free(cols[i].name);
    }

    free(cols);
}



/**
 *  Writes data out as csv file.
    @param the_data [in] object to write out passed from ff_write in ff_write.c.
    @param filename [in] name of file to write to.
    @param field_delim [in] string to use as field separator.
    @param force [in] whether to overwrite an existing file of same name if found.
    @return 1 on success, 0 on failure.
 */
int
dv_WriteCSV(Var* the_data, char* filename, char* field_delim, int header, int force)
{
	int i, j, count, row, max, columns, error;
	Var** data = NULL;
	char** keys = NULL;
	FILE* file = NULL;
	struct stat filestats;
    char record_delim[] = "\n";

	if( field_delim == NULL ) {
		field_delim = "\t";
	}

	i = stat(filename, &filestats);

	if ( i == -1 && errno != ENOENT) {
		parse_error("stat error: %s", strerror(errno) );
		return 0;
	} else if( i == 0 && force == 0) {
		parse_error("File already exists: %s\n", filename);
		return 0;
	}

	error = 0;

	if( V_TYPE(the_data) == ID_STRUCT ) {		//data is a structure so get sub structs
		count = get_struct_count(the_data);
		data = (Var**)calloc(count, sizeof(Var*));
		keys = (char**)calloc(count, sizeof(char*));
		if( data == NULL || keys == NULL) {
			memory_error(errno, (size_t)count*(size_t)(sizeof(Var*)+sizeof(char*)) );
			error = 1;
		}

		if( !error ) {
			for(i=0; i<count; i++) {
				if( get_struct_element(the_data, i, &keys[i], &data[i] ) == -1 ) {
					parse_error("Error retrieving structure elements at index %d\n", i);
					error = 1;
					break;
				}

				if( data[i] != NULL && V_SIZE(data[i])[2] > 1 ) {
					parse_error("Multiple bands (z>1) not supported.\nColumn %d has z dimension %d\n",
							i, V_SIZE(data[i])[2] );
					error = 1;
					break;
				}
			}
		}
	} else {	//else data is a basic type so no sub structures
		count = 1;
		keys = (char**)calloc(1, sizeof(char*));
		data = (Var**)calloc(1, sizeof(Var*));
		if(keys == NULL || data == NULL) {
			memory_error(errno, sizeof(char*)+sizeof(Var*) );
			error = 1;
		}

		if( !error ) {
			keys[0] = V_NAME(the_data);
			data[0] = the_data;
		}
	}

	if( error ) {
		free(keys);
		free(data);
	}

	for(i=0; i<count; i++) {
		if( V_ORG(data[i]) != BSQ ) {
			parse_error("Only BSQ data format supported\nColumn %s is not BSQ\n", keys[i] );
			free(keys);
			free(data);
			return 0;
		}
	}

    /* just opening in text output mode because text output OS will handle differences */
	file = fopen(filename, "w");
	if( file == NULL ) {
		parse_error("Error opening file \"%s\" : %s", filename, strerror(errno) );
		free(data);
		free(keys);
		return 0;
	}

/* print out column headers if requested */
	if(header)
		print_headers(data, keys, count, file, field_delim);


/* calculate max number of rows */
	max = 0;
	for(i=0; i<count; i++)
		if( max < V_SIZE(data[i])[1] ) max = V_SIZE(data[i])[1];


	row = 0;
	for(row = 0; row<max; row++) {
		for(i=0; i<count; i++) {

			if( data[i] == NULL ) {		//combine
				if( i < count-1 )
					fprintf(file, "%s", field_delim);
				continue;
			}

			//if a column has fewer rows than others
			if( (V_TYPE(data[i]) == ID_VAL && row >= V_SIZE(data[i])[1]) ||
				( V_TYPE(data[i]) == ID_TEXT && V_TEXT(data[i]).Row <= row) ) {

				if( i < count-1 ) {
					for(j=0; j<V_SIZE(data[i])[0]; j++)
					fprintf(file, "%s", field_delim);
				}
				continue;
			}
			/*
			 * drd
			 * [Bug 2169] load_csv() does not promote floating point data to doubles
			 *
			 * Saadat reports that the output was now coming out as E format with
			 * little resolution.
			 * The changes here are to make the output match the inputs
			 * The FLOAT inputs hand 4 decimal places, the DOUBLE inputs 1 decimal place
			 *
			 * The change is to %.4f from %G for FLOAT,
			 *                  %.1f from %G for DOUBLE
			 *
			 */

			if( V_TYPE(data[i]) == ID_VAL ) {
				columns = V_SIZE(data[i])[0];
				for(j=0; j<columns; j++) {
					switch(V_FORMAT(data[i]))
					{
						case BYTE:		fprintf(file, "%u", ((unsigned char*)V_DATA(data[i]))[row*columns+j] );	break;
						case DV_INT16:		fprintf(file, "%d", ((short*)V_DATA(data[i]))[row*columns+j] );			break;
						case INT:		fprintf(file, "%d", ((int*)V_DATA(data[i]))[row*columns+j] );			break;
						case FLOAT:		fprintf(file, "%.4f", ((float*)V_DATA(data[i]))[row*columns+j] );		break;
					  //case FLOAT:		fprintf(file, "%G", ((float*)V_DATA(data[i]))[row*columns+j] );			break;
						case DOUBLE:	fprintf(file, "%.1f", ((double*)V_DATA(data[i]))[row*columns+j] );		break;
					 // case DOUBLE:	fprintf(file, "%G", ((double*)V_DATA(data[i]))[row*columns+j] );		break;
						default:
							parse_error("unknown format for ID_VAL: %d\n", V_FORMAT(data[i]));
							free(keys);
							free(data);
							fclose(file);
							return 0;

					}
					if( j+1 < V_SIZE(data[i])[0] ) fprintf(file, "%s", field_delim);
				}
			}

			/*
			 * drd
			 * Added 'else if' for ID_STRING, a NULL terminated character string.
			 * This handler was simply missing.
			 * Already had ID_TEXT, a 1-D Array of Strings
			 *
			 * See enum definitions in parser.h
			 *
			 * todo: is ID_TEXT ever used by a call to dv_WriteCSV()?
			 *
			 */
			else if(V_TYPE(data[i]) == ID_STRING ) {
				fprintf(file, "%s", V_STRING(data[i]));
				}


			else if( V_TYPE(data[i]) == ID_TEXT ) {

				if( V_TEXT(data[i]).text[row] != NULL )
					csv_fwrite(file, V_TEXT(data[i]).text[row], strlen( V_TEXT(data[i]).text[row] ) );

			} else {
				keys[i] = keys[i]==NULL ? "(null)" : keys[i];
				parse_error("Unknown type: %d for column %d: %s\n", V_TYPE(data[i]), i, keys[i] );
				free(keys);
				free(data);
				fclose(file);
				return 0;
			}

			if( i<count-1 ) fprintf(file, "%s", field_delim);

		}	// end for (columns)

		fprintf(file, "%s", record_delim);
	}	//end while (rows)

	fclose(file);
	free(data);
	free(keys);

	return 1;
}



/**
    Output column headers to the specified file.
    @param data [in] the data structure to be printed as csv.
    @param keys [in] contains array of data structure member names (these are the headers).
    @param count [in] number of columns.
    @param file [in] file pointer to print to.
    @param fdelim [in] field separator.
    @return 1
*/
static int
print_headers(Var** data, char** keys, int count, FILE* file, char* fdelim)
{
	int i = 0, j = 0;
	char *key = NULL;

	for(i=0; i<count; i++) {

		key = keys[i] == NULL? "": keys[i];

		if( data[i] != NULL && V_SIZE(data[i])[0] > 1) {
			for(j=0; j<V_SIZE(data[i])[0]; j++) {
				if (strlen(key) > 0)
					fprintf(file, "%s[%d]", keys[i], j );
				if( j < V_SIZE(data[i])[0] - 1 ) fprintf(file, "%s", fdelim);
			}
		} else {
			fprintf(file, "%s", key);
		}

		if( i < count-1 ) fprintf(file, "%s", fdelim);
	}
	fprintf(file, "\n");

	return 1;
}



