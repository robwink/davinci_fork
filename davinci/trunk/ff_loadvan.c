#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#else
#include <sys/mman.h>
#include <libgen.h>
#endif /* _WIN32 */

#include <fcntl.h>
#include <ctype.h>
#include "parser.h"
#include "system.h"


#define MAX(a,b) (((a) > (b))? (a): (b))
#define MIN(a,b) (((a) < (b))? (a): (b))

/* error codes */
#define ENCOLS  -2 /* number of data values <> required number of columns */
#define EMEM    -1 /* memory allocation/reallocation error */
#define ETOOBIG -3 /* required buffer size is too big */
#define EINPUT  -4 /* structural inconsistency */
#define ETYPE   -5 /* invalid type encountered */
#define EFILE   -6 /* unable to open file -OR- input file problem */



typedef enum {
    TINT,
    TREAL,
    TSTR,
    TNA
} coltype;

struct _coldef;

typedef struct _coldef {
    /* column type extraction data */
    char     *name;            /* probable name */
    char     *text;            /* text as read from the file */
    int      sno;              /* serial number */
    coltype  data_type;        /* data type 0=int 1=real 2=string */
    int      max_len;          /* max text string length */
    double   max_num_val;      /* max value if treated as real */
    int      neg_flag;         /* data has -ve values */
    int      size;             /* size (in bytes) for the type */
    int      naflag;           /* data for this field contains an N/A */

    /* cached data */
    int      dav_type;         /* davinci type of this column */

    /* array specific data, for structure extraction */
    struct   _coldef  *prev;   /* chain for array processing */
    struct   _coldef  *next;   /* chain for array processing */
    int      chain_len;        /* chain-length at the "root"-level only */
} coldef;                     /* column definition */


typedef enum {
    INVAL = 0,
    FS    = 1<<0,
    RS    = 1<<1,
    SIGN  = 1<<2,
    DIGIT = 1<<3,
    DOT   = 1<<4,
    EXP   = 1<<5,
    NA_N  = 1<<6,
    NA_SL = 1<<7,
    NA_A  = 1<<8,
    PRINT = 1<<9,
    SEOF  = 1<<10
} psym_cat;       /* pseudo symbols | categories */






static int make_coldefs(char **colnames, coldef **coldefs, int n);
static void free_coldefs(coldef *cols, int n);



/*
** Fix the "size" field of coldef structure for each of the column
** definitions based on the types recognized by davinci.
*/
static void
fill_col_min_size(
    coldef   *cols,
    int      n
    )
{
    int      i;

    for(i=0; i<n; i++){
        switch(cols[i].data_type){
        case TINT:
            cols[i].size = sizeof(int);

#if 0
            if((int)cols[i].max_num_val > SHRT_MAX){
                cols[i].size = sizeof(int);
            }
            else if(((int)cols[i].max_num_val > CHAR_MAX) && !cols[i].neg_flag){
                cols[i].size = sizeof(short);
            }
            else {
                /* davinci char-type values cannot be -ve */
                cols[i].size = sizeof(char);
            }
#endif
            break;
      
        case TREAL:
            cols[i].size = sizeof(double);

#if 0
            if(cols[i].max_num_val > FLT_MAX){
                cols[i].size = sizeof(double);
            }
            else {
                cols[i].size = sizeof(float);
            }
#endif
            break;

        case TSTR:
            /* well in this case, the length is the maximum length */
            cols[i].size = cols[i].max_len;
            break;
        }
    }
}


static void
fill_pseudo_symbols(
    const char  *fdelim,
    const char  *rdelim,
    int   psyms[257]
    )
{
    uint     i, n;
    psym_cat c;

    for (i = 0; i < 256; i++){
        c = INVAL;
        if (strchr(fdelim, (uchar)i))                c |= FS;
        if (strchr(rdelim, (uchar)i))                c |= RS;
        if (((uchar)i) == '+' || ((uchar)i) == '-')  c |= SIGN;
        if (isdigit((uchar)i))                       c |= DIGIT;
        if (((uchar)i) == '.')                       c |= DOT;
        if (toupper((char)i) == 'E')                 c |= EXP;
        if (toupper((char)i) == 'N')                 c |= NA_N;
        if (((char)i) == '/')                        c |= NA_SL;
        if (toupper((char)i) == 'A')                 c |= NA_A;
        if (isprint((uchar)i))                       c |= PRINT;

        psyms[i] = c;
    }
    psyms[256] = SEOF;
}

typedef struct {
    char  *data;      /* data segment pointer */
    off_t len;        /* length of data segment */
    char  *dend;      /* end of data pointer data + len */
    int   psyms[257]; /* pseudo symbols */
    char  *q;         /* current position of lexer within data */
    int   zflag;      /* if set: zero out field and record terminators */
} vlexc;             /* vanilla lexer context */

static vlexc*
new_vlexc(
    char  *data,
    off_t len,
    const char  *fs,
    const char  *rs,
    int   zflag
    )
{
    vlexc *t;
    int   i;

    if ((t = (vlexc *)calloc(sizeof(vlexc), 1)) == NULL){
        return NULL;
    }

    t->data = data;
    t->len = len;
    t->dend = t->data + t->len;
    fill_pseudo_symbols(fs, rs, t->psyms);
    t->q = t->data;
    t->zflag = zflag;

    return t;
}

static void
free_vlexc(
    vlexc *h
    )
{
    free(h);
}

static void
reset_vlex(
    vlexc *h
    )
{
    h->q = h->data;
}


typedef enum {
    VTOK_ERR,         /* ERROR */
    VTOK_INT,         /* INT value encountered */
    VTOK_REAL,        /* REAL value encountered */
    VTOK_STR,         /* STRING encountered */
    VTOK_NA,          /* N/A encountered */
    VTOK_EOR,         /* end of record */
    VTOK_EOD          /* end of data */
} vlextokt;          /* vanilla lexer token types */


/*
** Returns the next token from input data (as given by lexer context "h").
** Lexeme start and end+1 are returned in *ls and *le.
*/
static vlextokt
vlex_next_tok(
    vlexc    *h,      /* vanilla lexer context */
    char     **ls,    /* lexeme start */
    char     **le     /* lexeme end */
    )
{
    int      state;   /* current state */
    uint     l;       /* current input symbol */
    uint     lc;      /* input symbol's category pattern of ORed "psym_cat"s */
    int      zflag;   /* zero out field- and record- separators? */
    char     *p, *q;  /* p = start of lexeme, q = end of lexeme + 1 */
    char     *dend;   /* end of data pointer */

    zflag = h->zflag;

    for(state = 10, q = h->q, dend = h->dend; q <= dend; q++){
        /*
        ** At end of data, assume a sentinel value of 256
        */

        l = (q < dend)? *q: 256;
        lc = h->psyms[l];    /* get ORed pseudo-symbols */

        /*
        ** NOTE (1): The text following each state case gives the 
        ** pattern of input seen thus far.
        **
        ** NOTE (2): q-- serves as a back-track mechanism, since,
        ** at each iteration, q is incremented automatically.
        */ 

        switch(state){
        case 10:  /* start state */
            p = q; /* mark start of lexeme */

            /*
            ** If zero-flag (zflag) is set, zero out field and record
            ** separators.
            **
            ** Skip consecutive field separators. Set the new start of
            ** lexeme.
            */
            if      (lc & FS){          state = 10;  p = q+1; if (zflag) *q = 0; }
            else if (lc & RS){          state = 130;          if (zflag) *q = 0; }
            else if (lc & SEOF)         state = 960, q--;
            else if (lc & SIGN)         state = 20;
            else if (lc & DIGIT)        state = 30;
            else if (lc & NA_N)         state = 100;
            else if (lc & PRINT)        state = 90;
            else                        state = 999, q--;
            break;

        case 20: /* [+-] */
            if      (lc & (SEOF|RS|FS)) state = 920, q--;
            else if (lc & DIGIT)        state = 30;
            else if (lc & PRINT)        state = 90;
            else                        state = 920, q--;
            break;

        case 30: /* [+-]{0,1}[0-9]+ */
            if      (lc & (SEOF|RS|FS)) state = 910, q--;
            else if (lc & DIGIT)        state = 30;
            else if (lc & DOT)          state = 40;
            else if (lc & EXP)          state = 60;
            else if (lc & PRINT)        state = 90;
            else                        state = 910, q--;
            break;

        case 40: /* [+-]{0,1}[0-9]+[.] */
            if      (lc & (SEOF|RS|FS)) state = 900, q--;
            else if (lc & DIGIT)        state = 50;
            else if (lc & EXP)          state = 60;
            else if (lc & PRINT)        state = 90;
            else                        state = 900, q--;
            break;

        case 50: /* [+-]{0,1}[0-9]+[.][0-9]+ */
            if      (lc & (SEOF|RS|FS)) state = 900, q--;
            else if (lc & DIGIT)        state = 50;
            else if (lc & EXP)          state = 60;
            else if (lc & PRINT)        state = 90;
            else                        state = 900, q--;
            break;

        case 60: /* [+-]{0,1}[0-9]+([.][0-9]*){0,1}[eE] */
            if      (lc & (SEOF|RS|FS)) state = 920, q--;
            else if (lc & DIGIT)        state = 80;
            else if (lc & SIGN)         state = 70;
            else if (lc & PRINT)        state = 90;
            else                        state = 920, q--;
            break;

        case 70: /* [+-]{0,1}[0-9]+([.][0-9]*){0,1}[eE][+-] */
            if      (lc & (SEOF|RS|FS)) state = 920, q--;
            else if (lc & DIGIT)        state = 80;
            else if (lc & PRINT)        state = 90;
            else                        state = 920, q--;
            break;

        case 80: /* [+-]{0,1}[0-9]+([.][0-9]*){0,1}[eE][+-]{0,1}[0-9]+ */
            if      (lc & (SEOF|RS|FS)) state = 900, q--;
            else if (lc & DIGIT)        state = 80;
            else if (lc & PRINT)        state = 90;
            else                        state = 900, q--;
            break;

        case 90: /* [:print:]+ */
            if      (lc & (SEOF|RS|FS)) state = 920, q--;
            else if (lc & PRINT)        state = 90;
            else                        state = 920, q--;
            break;

        case 100: /* [nN] */
            if      (lc & (SEOF|RS|FS)) state = 920, q--;
            else if (lc & NA_SL)        state = 110;
            else if (lc & PRINT)        state = 90;
            else                        state = 920, q--;
            break;
      
        case 110: /* [nN][/] */
            if      (lc & (SEOF|RS|FS)) state = 920, q--;
            else if (lc & NA_A)         state = 120;
            else if (lc & PRINT)        state = 90;
            else                        state = 920, q--;
            break;
      
        case 120: /* [nN][/][aA] */
            if      (lc & (SEOF|RS|FS)) state = 930, q--;
            else if (lc & PRINT)        state = 90;
            else                        state = 930, q--;
            break;

        case 130:   /* End of Record */
            if      (lc & RS){          state = 130;          if (zflag) *q = 0; }
            else                        state = 951, q--;
            break;

            /* >> remove back-track from states above that call accepting states << */
            /* >>>>>> MOVE all accepting states into a separate switch <<<<<< */
        case 900:   /* Accept REAL */
            /* [+-]{0,1}[0-9]+([.][0-9]*){0,1}([eE][0-9]+){0,1} */
            *ls = p;
            h->q = *le = q;
            return VTOK_REAL;

        case 910:   /* Accept INT */
            /* [+-]{0,1}[0-9]+ */
            *ls = p;
            h->q = *le = q;
            return VTOK_INT;

        case 920:   /* Accept STRING */
            /* [:print:]+ */
            *ls = p;
            h->q = *le = q;
            return VTOK_STR;

        case 930:   /* Accept N/A */
            /* [nN][/][aA] */
            *ls = p;
            h->q = *le = q;
            return VTOK_NA;

        case 951:   /* Accept End of Record */
            /* [:eor:]+ */
            *ls = p;
            h->q = *le = q;
            return VTOK_EOR;

        case 960:   /* End of Data */
            *ls = p;
            h->q = *le = q;
            return VTOK_EOD;

        default:    /* 999 - Catch all - Error! */
            *ls = p;
            h->q = *le = q;
            return VTOK_ERR;
        }
    }

    /*
    ** If the following line is ever reached, there is an error above
    */
    fprintf(stderr, "%s:%d unreachable code reached.\n", __FILE__, __LINE__);

    return VTOK_ERR;
}

/*
** vpass1() does the first pass through the input data mapped into
** the memory. It processes input on a token basis. Tokens are
** retrieved using calls to vlex_next_tok(). vpass1() assumes that
** the lexer (in this case vlex_next_tok()) will return separate
** tokens for N/As, end-of-records, and end-of-data.
**
** First input record is assumed to contain a list of field names
** and does not contribute towards field-type and -length 
** determination. A list of columns is generated from this line
** and returned in *cols, with number of columns found returned
** in *ncols.
**
** Initially all columns are assumed to be of integer type, unless
** the input data says otherwise.
** 
** 2nd record onward are treated as data records. Field data types,
** and lengths are determined using these.
**
** NOTE 1:
** A valid input data file should contain the same number of fields
** as there are columns in the first record.
**
** NOTE 2:
** Code that checks for minimum size required by a field is not being
** used right now, but it can be activated pretty easily.
*/
static int
vpass1(
    vlexc    *h,
    coldef   **cols,
    int      *ncols
    )
{
    vlextokt t;
    char     *p, *q;
    int      hflag = 1;  /* we are processing the header */

    char     **tokl = NULL;
    int      ntok = 0, toka = 0;

    int      rc;

    int      nrecs = 0;
    int      ncpr = 0;


    *cols = NULL;
    *ncols = 0;

    do{
        /* get next input token */
        t = vlex_next_tok(h, &p, &q);
        if (t == VTOK_EOR) nrecs++;

        if (hflag){

            /*
            ** If hflag is set, we are reading the header line from the
            ** file. Each token on this line is put into a list of column
            ** names. At end of record, the number of columns is available
            ** in "ncols".
            */

            if ((t == VTOK_EOR) || (t == VTOK_EOD)){
                hflag = 0;

                if ((rc = make_coldefs(tokl, cols, *ncols)) < 0){
                    if (tokl) free(tokl);
                    return rc;
                }
            }
            else {
                (*ncols)++;

                /* keep "toka" multiple of "2" as we use two slots per field */
                if (ntok >= toka){
                    toka = (toka == 0)? 256: toka * 2;
                    tokl = (char **)realloc(tokl, toka * sizeof(char *));
                }

                /* add start and end of current field to the list of tokens */
                tokl[ntok++] = p;
                tokl[ntok++] = q;
            }
        }
        else {

            /* if input is jagged, catch it */

            if ((t != VTOK_EOR) && (t != VTOK_EOD) && ((ncpr+1) > *ncols)){
                fprintf(stderr, "Jagged input record #%d.\n", nrecs);
                if (tokl) free(tokl);
                if (*cols) free_coldefs(*cols, *ncols);
                return EINPUT;
            }

            switch(t){
            case VTOK_EOR:
            case VTOK_EOD:
                if (ncpr > 0 && ncpr != *ncols){
                    fprintf(stderr, "Jagged input record #%d.\n", nrecs);
                    if (tokl) free(tokl);
                    if (*cols) free_coldefs(*cols, *ncols);
                    return EINPUT;
                }

                ncpr = 0;
                break;

                /* update column specs */
            case VTOK_INT:
                (*cols)[ncpr].data_type = MAX((*cols)[ncpr].data_type, TINT);
                (*cols)[ncpr].max_len = MAX((*cols)[ncpr].max_len, (int)(q-p));
                /*
                  if (v < 0) (*cols)[ncpr].negflag = 1;
                  (*cols)[ncpr].max_num_val = MAX((*cols)[ncpr].max_num_val, fabs(v));
                  */
                ncpr ++;
                break;

            case VTOK_REAL:
                (*cols)[ncpr].data_type = MAX((*cols)[ncpr].data_type, TREAL);
                (*cols)[ncpr].max_len = MAX((*cols)[ncpr].max_len, (int)(q-p));
                /*
                  if (v < 0) (*cols)[ncpr].negflag = 1;
                  (*cols)[ncpr].max_num_val = MAX((*cols)[ncpr].max_num_val, fabs(v));
                  */
                ncpr ++;
                break;

            case VTOK_STR:
                (*cols)[ncpr].data_type = MAX((*cols)[ncpr].data_type, TSTR);
                (*cols)[ncpr].max_len = MAX((*cols)[ncpr].max_len, (int)(q-p));
                ncpr ++;
                break;

            case VTOK_NA:
                (*cols)[ncpr].naflag = 1;
                (*cols)[ncpr].max_len = MAX((*cols)[ncpr].max_len, (int)(q-p));
                ncpr ++;
                break;

            case VTOK_ERR:
                fprintf(stderr, "Aborting first pass of data.\n");
                if (tokl) free(tokl);
                if (*cols) free_coldefs(*cols, *ncols);
                return EINPUT;

            default:
                fprintf(stderr, "Unhandled instance in %s:%d.\n",
                        __FILE__, __LINE__);
                if (tokl) free(tokl);
                if (*cols) free_coldefs(*cols, *ncols);
                return EINPUT;
            }
        }
    } while (t != VTOK_EOD);

    /* establish minimum size (in bytes) for the field */
    fill_col_min_size(*cols, *ncols);

    if (tokl) free(tokl);
    return nrecs;
}


/*
** return the davinci type based on the column type and size pair
*/
static int
davinci_type(
    coltype  c,       /* column type: TINT, TREAL, ... */
    int      csize    /* coldef.size */
    )
{
    int      dav_type;

    switch(c){
    case TINT:
        switch(csize){
        case 1: dav_type = BYTE; break;
        case 2: dav_type = SHORT; break;
        default: dav_type = INT; break;
        }
        break;

    case TREAL:
        switch(csize){
        case 4: dav_type = FLOAT; break;
        default: dav_type = DOUBLE; break;
        }
        break;

    case TSTR:
    default:
        /* does not apply otherwise */
        dav_type = -1;
        break;
    }

    return dav_type;
}


/*
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

    for(i = 0; i < n; i++){

        for(j = i+1; j < n; j++){

            if (strcmp(cols[i].name, cols[j].name) == 0){
                /*
                ** If column name match, combine the columns into array.
                ** This array is constructed by using the "next" and "prev"
                ** pointers within coldef structure.
                */

                cols[i].next = &cols[j];
                cols[j].prev = &cols[i];

                /*
                ** we'll worry about field type promotion and column lengths
                ** later, once we have all the columns for a field linked in
                */

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
        return EMEM;
    }

    /* count number of fields */
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
        ** Also determine is final field size and type based on
        ** the elements of the field (if it is an array field)
        */

        cols[i].chain_len = 1;

        for (t = cols[i].next; t; t = t->next){
            cols[i].chain_len ++;
            cols[i].data_type = MAX(t->data_type, cols[i].data_type);
            cols[i].size = MAX(t->size, cols[i].size);
        }

        for (t = cols[i].next; t; t = t->next){
            t->data_type = cols[i].data_type;
            t->size = cols[i].size;
        }

        /* save davinci type of the column for future use */
        cols[i].dav_type = davinci_type(cols[i].data_type, cols[i].size);

        /* add this field to the list of "root"-level fields */
        (*l)[nfields++] = &cols[i];
    }

    /*
    ** If the input columns were:
    **
    **           "sclk_time temps[1] temps[2] det temps[3]"
    **
    ** then the LIST *l should be arranged as follows:
    **
    ** l[0] = coldef<"sclk_time">
    **
    ** l[1] = coldef<"temps"> 
    **        + l[1]->next points to chain of coldef<"temps">
    **        each of l[1]->next's is an element of the array "temps"
    **
    ** l[2] = coldef<"det">
    **
    */

    /* pack the structure list */
    *l = (coldef **)realloc(*l, nfields * sizeof(coldef *));

    return nfields;
}


/*
** Preallocates storage space for the ("struct") data to be returned.
** Returns NULL on error.
*/ 
static Var *
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

    /* create a blank structure */
    v_return = new_struct(0);

    for(i=0; i<nfields; i++){
        cl = fields[i]->chain_len;

        switch(fields[i]->data_type){
        case TINT:
        case TREAL:
            tdata = calloc(nrecs, cl * fields[i]->size);
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
            tdata = calloc(nrecs, sizeof(char *));
            v_t = newText(nrecs, tdata);
            for(j=0; j<nrecs; j++){
                ((char **)tdata)[j] = calloc(1,
                                             /* data */     (cl*fields[i]->size) +
                                             /* spaces */   (cl-1) +
                                             /* '\0' */     1);

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

/*
** vpass2() reads the data from input file, translates it and
** stores in the data blocks pointed to by "data_ptrs". Each element
** of data_ptrs[] is a field in the output structure as given by
** fields[] array.
**
** Assumptions:
** -  vpass1() has zeroed out the record- and field-delimiters, resulting
**    in the input "data" being '\0' delimited (except may be for the last 
**    column of the last record - which is handled differently).
** -  All data-storage blocks (as given by "data_ptrs") are pre-allocated.
** -  Array data can be atmost two dimensional.
** -  Storage-data colums are organized in the form of parallel arrays.
**    Thus we have an structure with a bunch of columns, with each of the
**    column having "nrecs" records.
** -  Storage for Integer and Reals columns both 1D and 2D is in the 
**    form of a single dimensional data block (i.e. a linear array).
** -  Storage for Strings is in the form of rows of text, with columns
**    concatenated together with a space delimiter. Thus each line
**    of text can be accessed by indexing a double pointer "**line" as
**    line[i].
*/
static int
vpass2(
    void     *data,
    off_t    len,
    coldef   **fields,
    int      nfields,
    int      nrecs,
    void     **data_ptrs
    )
{
    int      i, j, maxlen = 0;
    char     *buff = NULL;
    char     **vals = NULL;
    int      recno = 0;
    int      idx;
    int      ncols;
    coldef   *t;
    char     *s;
    char     *tok = NULL;
    char     *dend;


    /*
    ** Make all handled typed access methods available through the
    ** following arrays:
    */
    char     **byte_data = (char **)data_ptrs;
    short    **short_data = (short **)data_ptrs;
    int      **int_data = (int **)data_ptrs;
    float    **float_data = (float **)data_ptrs;
    double   **double_data = (double **)data_ptrs;
    char     ***str_data = (char ***)data_ptrs;

    maxlen = 10;
    ncols = 0;

    for(i = 0; i < nfields; i++){
        maxlen = MAX(maxlen, fields[i]->max_len);
        ncols += fields[i]->chain_len;
    }
    maxlen ++;

    /*
    ** Values are temporary linked here according to their arrival order.
    */
    if ((vals = (char **)calloc(ncols, sizeof(char *))) == NULL){
        return EMEM;
    }

    /* allocate work area for one field worth of data */
    if ((buff = (char *)calloc(maxlen, 1)) == NULL){
        free(vals);
        return EMEM;
    }

    /* evaluate data limit */
    dend = ((char *)data) + len;

    /* Point to first token */
    tok = data;
    while(tok < dend && (*tok) == 0) tok++;

    /* skip header line */
    for (j = 0; j < ncols; j++){
        /* skip to next token */
        while(tok < dend && (*tok) != 0) tok++;
        while(tok < dend && (*tok) == 0) tok++;
    }

    /* load in data from data lines */
    for (recno=1; recno<nrecs; recno++){

        /* collect data values */
        for (j=0; j<ncols; j++){

            /* save current token */
            if (recno == (nrecs-1) && j == (ncols-1)){
                /* last column of last row is special: it could be without '\0' */
                memcpy(buff, tok, fields[nfields-1]->max_len);
                buff[fields[nfields-1]->max_len] = 0; /* '\0'-terminate */

                vals[j] = buff;
            }
            else {
                vals[j] = tok;
            }

            /* skip to next token */
            while(tok < dend && (*tok) != 0) tok++;
            while(tok < dend && (*tok) == 0) tok++;
        }

        /* place data values at their appropriate positions */
        for(i=0; i<nfields; i++){
            /*
            ** note that if the current column is an array, the
            ** array elements are linked to the "root"-coldef in
            ** a chain
            */
            for(j=0, t=fields[i]; t; t=t->next, j++){
                switch(t->data_type){
                case TINT:
                    idx = (recno-1) * fields[i]->chain_len + j;

                    switch(fields[i]->dav_type){
                    case BYTE:
                        byte_data[i][idx] = atoi(vals[t->sno]); break;
               
                    case SHORT:
                        short_data[i][idx] = atoi(vals[t->sno]); break;
               
                    default:
                    case INT:
                        int_data[i][idx] = atoi(vals[t->sno]); break;
                    }
                    break;

                case TREAL:
                    idx = (recno-1) * fields[i]->chain_len + j;

                    switch(fields[i]->dav_type){
                    case FLOAT:
                        float_data[i][idx] = atof(vals[t->sno]); break;

                    default:
                    case DOUBLE:
                        double_data[i][idx] = atof(vals[t->sno]); break;
                    }
                    break;

                case TSTR:
                    /*
                    ** assume that the string was initialized to null at its
                    ** creation
                    */
                    s = str_data[i][recno-1];

                    strcat(s, vals[t->sno]);
                    if (t->next != NULL){ strcat(s, " "); }
                    break;
            
                default:
                    fprintf(stderr, "Unhandled type %d in %s:%d.\n",
                            t->data_type, __FILE__, __LINE__);
                    return ETYPE;
                }
            }
        }
    }

    free(vals);
    free(buff);
    return recno;
}


/*
** Function that davinci-"load_vanilla" translates to ultimately.
** It returns a Var * in *v_return or NULL on erro.
*/
static int
vanread(
    const char  *filename,
    const char  *fdelims,
    const char  *rdelims,
    Var   **v_return
    )
{
    int      fd;
    char     *data = NULL;
    struct   stat sbuf;
    char     delims[256];
    coldef   *cols = NULL;
    coldef   **fields = NULL;
    int      rc, ncols, nrecs, nfields;
    int      i;
    vlexc    *h;
    void     **data_ptrs = NULL;
    Var *d;

    *v_return = NULL;

    if (stat(filename, &sbuf) < 0){
        return EFILE;
    }

    if((fd = open(filename, O_RDONLY)) < 0){
        return EFILE;
    }

    data = (char *)mmap(NULL, sbuf.st_size,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE, fd, 0);

    if (data == NULL){
        close(fd);
        return EMEM;
    }

    close(fd);

    /*
    ** Create a new lexer context. Ask the lexer to zero out fdelims
    ** and rdelims.
    */
    h = new_vlexc(data, sbuf.st_size, fdelims, rdelims, 1);

    /*
    ** Do a first scan through data and determine the number of
    ** columns and their types.
    */
    rc = nrecs = vpass1(h, &cols, &ncols);

    /* Free lexer context */
    free_vlexc(h);

    if (rc < 0){
        munmap(data, sbuf.st_size);
        if(cols) free_coldefs(cols, ncols);
        return EINPUT;
    }

    /*
    ** Guess the structure of data based on column names and
    ** data types.
    */
    rc = nfields = guess_struct(cols, ncols, &fields);
    if (rc < 0){
        free_coldefs(cols, ncols);
        munmap(data, sbuf.st_size);
        return rc;
    }

    /*
    ** Allocate pointers to data. These pointers will be used by
    ** the second pass to fill in the data values.
    */
    data_ptrs = (void **)calloc(nfields, sizeof(void *));
    if (data_ptrs == NULL){
        free_coldefs(cols, ncols);
        free(fields);
        munmap(data, sbuf.st_size);
        return EMEM;
    }

    /*
    ** Allocate memory to start off the copying operation
    ** NOTE: nrecs = no of data lines + header line
    */
    *v_return = alloc_davinci_data_space(fields, nfields, nrecs-1);
    if ((*v_return) == NULL){
        free_coldefs(cols, ncols);
        free(fields);
        free(data_ptrs);
        munmap(data, sbuf.st_size);
        return EMEM;
    }

    /*
    ** point to that memory segment using all available types
    */
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


#if 0
    for (i = 0; i < nfields; i++){
        fprintf(stdout, "%s[%d]: %s\n",
                fields[i]->name, fields[i]->chain_len,
                (fields[i]->data_type == TINT)?  "INT":
                (fields[i]->data_type == TREAL)? "REAL":
                (fields[i]->data_type == TSTR)?  "STR":
                "UNKNOWN");
    }
#endif

    /*
    ** Second pass through the data loads the data values into
    ** the memory locations pointed to by data_ptrs[]
    */
    /* reset_vlex(&h); */
    rc = vpass2(data, sbuf.st_size, fields, nfields, nrecs, data_ptrs);

    free(data_ptrs);
    if (fields) free(fields);
    free_coldefs(cols, ncols);
    munmap(data, sbuf.st_size);

    return 1;
}

/**************************************************************************
** davinci function - load_vanilla("filename" [, "delimiter"])           **
***************************************************************************/
Var *
ff_loadvan(
    vfuncptr func,
    Var      *args
    )
{
    int      ac;
    Var      **av;
    char     *file_arg = NULL, *delim_arg = NULL;
    char     *delim = NULL;
    Var      *v_return;
    int      rc;

    Alist    alist[3];
    alist[0] = make_alist( "file",   ID_STRING,    NULL,        &file_arg);
    alist[1] = make_alist( "delim",  ID_STRING,    NULL,        &delim_arg);
    alist[2].name = NULL;

	if (parse_args(func, args, alist) == 0) return(NULL);

    if (file_arg == NULL){
        parse_error("%s(): \"%s\" must be specified.\n",
                    func->name, alist[0].name);
        return NULL;
    }

    if (delim_arg == NULL){
        delim_arg = "\t "; /* default delimiter is space or tab */
    }

    rc = vanread(file_arg, delim_arg, "\r\n", &v_return);

    if (rc < 0){
        if (rc == EMEM){ fprintf(stderr, "Fatal! Memory allocation failure."); }
        parse_error("%s(): Failed! with rc=%d. See previous messages.",
                    func->name, rc);
        return NULL;
    }

    return v_return;
}




static void
free_coldefs(
    coldef   *cols,
    int      n
    )
{
    int   i;

    for(i = 0; i < n; i++){
        if (cols[i].text) free(cols[i].text);
        if (cols[i].name) free(cols[i].name);
    }

    free(cols);
}

/*
** Allocate coldef structures for each of the columns in the vanilla
** input text file. While doing so, store the column names within the
** column definition structure.
**
** Assume every field is an integer unless proven otherwise.
*/
static int
make_coldefs(
    char     **colnames,
    coldef   **coldefs,
    int      n
    )
{
    int   i, j;
    char  *p, *q;
    int   cnamelen;

    *coldefs = (coldef *)calloc(n, sizeof(coldef));

    if (*coldefs == NULL){
        return EMEM;
    }

    for(i = 0; i < n; i++){
        cnamelen = colnames[i*2+1]-colnames[i*2]+1;
        if ((q = (*coldefs)[i].name = (char *)malloc(cnamelen)) == NULL){
            free_coldefs(*coldefs, n);
            return EMEM;
        }

        sprintf(q, "%*.*s", cnamelen-1, cnamelen-1, colnames[i*2]);

        /*
        ** get rid of table name from the field, i.e.
        **         obs.sclk_time -> sclk_time
        */
        if (p = strchr(q, '.')){ strcpy((*coldefs)[i].name, p+1); }

        /*
        ** get rid of array dimensions from the field name, i.e.
        **         temps[1] -> temps
        */
        if (p = strchr(q, '[')){ *p = 0; }

        /*
        ** replace ':' with '_' for bit-fields
        */
        if (p = strchr(q, ':')){ *p = '_'; }

        if (((*coldefs)[i].text = (char *)malloc(cnamelen)) == NULL){
            free_coldefs(*coldefs, n);
            return EMEM;
        }

        sprintf((*coldefs)[i].text, "%*.*s",
                cnamelen-1, cnamelen-1, colnames[i*2]);

        (*coldefs)[i].sno = i;
        (*coldefs)[i].data_type = TINT; /* start as "int", promote if necessary */
        (*coldefs)[i].max_len = 0;
        (*coldefs)[i].max_num_val = 0;
        (*coldefs)[i].size = 0;
        (*coldefs)[i].naflag = 0;

        (*coldefs)[i].chain_len = 0;
        (*coldefs)[i].next = (*coldefs)[i].prev = NULL;
    }

    return n;
}
