/******************************* parser .h *******************************/
#ifndef PARSER_H
#define PARSER_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef __MSDOS__
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/time.h>
#include "config.h"
#include <values.h>

#else 

#include "dos.h"

#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>


#include "system.h"

#define memdup(p, l)	memcpy(malloc(l), (p), l)

typedef struct _var Var;
typedef Var * Vptr;
typedef struct _symbol Sym;
typedef struct _range Range;
typedef struct _tagnode Node;
typedef struct _tagVstruct Vstruct;
typedef struct _text TextArray;

struct _range {
	int dim;			/* dimension of data */
	int lo[3];
	int hi[3];
	int step[3];
};

struct _symbol {
    int format;			/* format of data */
    int dsize;			/* total size of data */
    int size[3];        /* size of each axis */
    int order;			/* axis application order */
    void *data;

	void *null;			/* null value */

    char *title;
};

struct _tagnode {
	Var *left;
	Var *right;
	Var *parent;
};

struct _tagVstruct {
	int count;
	char **names;
	Var **data;
};

struct _text {
	int Row;
	unsigned char **text;
};


struct _var {
	int type;
    char *name;
    union {
        Node node;
		Sym sym;
		Range range;
		char *string;
		Var *keyval;		/* used by $Keyword */
		Vstruct vstruct;
		TextArray textarray;
	} value; 
	Var *next;
};

#define V_NEXT(v)	(v)->next				/* pointer to next value in table */
#define V_NAME(v)	(v)->name				/* NAME of SYMbol in union */
#define V_TYPE(v)	(v)->type				/* type of var */

#define V_KEYVAL(v)	(v)->value.keyval		/* keyword value */
#define V_STRING(v)	(v)->value.string
#define V_RANGE(v)	(&((v)->value.range))	/* range value */
#define V_NODE(v)	(&((v)->value.node))
#define	V_SYM(v)	(&((v)->value.sym))		/* SYMbol value in union */

#define V_DATA(v)	V_SYM(v)->data			/* pointer to data */
#define V_INT(v)	(*((int *)V_DATA(v)))	/* derefernce as a single int */
#define V_FLOAT(v)	(*((float *)V_DATA(v)))	/* derefernce as a single float */
#define V_FORMAT(v)	V_SYM(v)->format
#define V_DSIZE(v)	V_SYM(v)->dsize
#define V_SIZE(v)	V_SYM(v)->size
#define V_ORDER(v)	V_SYM(v)->order
#define V_ORG(v)	V_SYM(v)->order

#define V_HISTORY(v) V_SYM(v)->history
#define V_TITLE(v)   V_SYM(v)->title

#define V_STRUCT(v)  (v)->value.vstruct
#define V_TEXT(v)		(v)->value.textarray

#define newVar	(Var *)mem_malloc


/**
 ** The following define the various types for Var->type
 **/

#define ID_BASE			100				/* in case of conflicts */
#define ID_NONE         0				/* a non value */
#define ID_ERROR		ID_BASE-1		
#define ID_UNK  		ID_BASE+1       /* Unknown type */
#define ID_STRING       ID_BASE+2       /* NULL terminated character string */
#define	ID_KEYWORD		ID_BASE+3		/* keyword argument */
#define ID_VAL			ID_BASE+5		/* everything with dim != 0 */
#define ID_STRUCT      ID_BASE+6		/* Structure */
#define ID_TEXT		   ID_BASE+8	   /*1-D Array of Strings*/

#define ID_IVAL         ID_BASE+10		/* Integer value */
#define ID_RVAL         ID_BASE+11		/* real value */
#define ID_ID           ID_BASE+12		/* Identifier */

#define ID_LIST         ID_BASE+14		/* Statement list */
#define ID_IF           ID_BASE+15		/* if statement */
#define ID_ELSE         ID_BASE+16		/* else statement */
#define ID_WHILE        ID_BASE+17		/* while statement */
#define ID_CONT         ID_BASE+18		/* continue statement */
#define ID_BREAK        ID_BASE+19		/* break statement */
#define ID_RETURN       ID_BASE+20		/* return statement */

#define ID_RANGES       ID_BASE+21		/* list of ranges */
#define ID_RSTEP        ID_BASE+22		/* list of ranges */
#define ID_RANGE        ID_BASE+23		/* single range value */
#define ID_SET          ID_BASE+24		/* equivalence expression */
#define ID_OR           ID_BASE+25		/* logical (||) or */
#define ID_AND          ID_BASE+26		/* logical (&&) and */
#define ID_EQ           ID_BASE+27		/* logical (==) equals */
#define ID_NE           ID_BASE+28		/* logical (!=) not equals */
#define ID_LT           ID_BASE+29		/* logical (<) less than */
#define ID_GT           ID_BASE+30		/* logical (>) greater than */
#define ID_LE           ID_BASE+31		/* logical (<=) less than or equal */
#define ID_GE           ID_BASE+32		/* logical (>=) greater than or equal */
#define ID_ADD          ID_BASE+33		/* addition */
#define ID_SUB          ID_BASE+34		/* subtraction */
#define ID_MULT         ID_BASE+35		/* multiplcation */
#define ID_DIV          ID_BASE+36		/* division */
#define ID_MOD          ID_BASE+37		/* modulo divison */
#define ID_UMINUS       ID_BASE+38		/* unary minus */
#define ID_FUNCT        ID_BASE+39		/* function */
#define ID_ARRAY		ID_BASE+40		/* application of ranges to array */
#define ID_ARG          ID_BASE+41		/* list of arguments */
#define ID_ARGS         ID_BASE+42		/* single argument */

#define ID_FOR          ID_BASE+43		/* for loop */
#define ID_FOREACH      ID_BASE+44		/* foreach loop */
#define ID_EACH         ID_BASE+45		/* foreach val */
#define ID_ARGV         ID_BASE+46		/* $VALUE argument.  Evalue at run */

#define ID_INC          ID_BASE+47		/* increment value */
#define ID_DEC          ID_BASE+48		/* decrement value */
#define ID_MULSET       ID_BASE+49		/* *=  value */
#define ID_DIVSET       ID_BASE+50		/* /= value */

#define ID_POW          ID_BASE+51		/* exponent */
#define ID_CAT          ID_BASE+52		/* concatenate */

#define ID_ENUM         ID_BASE+53		/* enumerated argument, not parsed */
#define ID_DECL         ID_BASE+54		/* Declaration */
#define ID_WHERE        ID_BASE+55		/* Where */
#define ID_DEREF        ID_BASE+56		/* Structure dereference */

#define ID_CONSTRUCT    ID_BASE+57		/* Structure constructor */
#define ID_DECONSTRUCT  ID_BASE+58		/* Structure deconstructor */



/**
 ** element format.
 ** Var->value.Sym->format
 **/

#define BYTE		1
#define SHORT		2
#define INT			3
#define FLOAT		4
#define VAX_FLOAT	5
#define VAX_INTEGER 6
#define DOUBLE		8

#define NBYTES(a)	((a) == INT ? 4 : ((a) == VAX_FLOAT ? 4 : ((a) == VAX_INTEGER ? 2 : (a))))

/**
 ** Data axis order
 ** Var->value.Sym->order
 **
 ** !!! CAUTION: these values must be 0 based.  They are used as array
 **              indices below.
 **/

#define BSQ			0
#define BIL			1
#define BIP			2

#define Format2Str(i)	FORMAT2STR[(i)]
#define Org2Str(i)		ORG2STR[(i)]

#define GetSamples(s,org)	(s)[((org) == BIP ? 1 : 0)]
#define GetLines(s,org)		(s)[((org) == BSQ ? 1 : 2)]
#define GetBands(s,org)		(s)[((org) == BIP ? 0 : ((org) == BIL ? 1 : 2))]

#define saturate(v,lo,hi)	((v) > (lo) ? ((v) < (hi) ? (v) : (hi)) : (lo))
#define saturate_byte(v)	saturate(v,0,255)
#define saturate_short(v)	saturate(v,(MINSHORT),(MAXSHORT));
#define saturate_int(v)		saturate(v,(MININT), (MAXINT));
#define saturate_float(v)	v
#define saturate_double(v)	v

#define HBUFSIZE 8192
#define PATH_SEP ' '

struct keywords {
	char *name;
	Var *value;
};

typedef struct _vfuncptr *vfuncptr;
typedef Var * (*vfunc)(struct _vfuncptr *,Var *);	/* function caller */
typedef double (*dfunc)(double);
typedef double (*ddfunc)(double, double);

struct _vfuncptr {
	char *name;
	vfunc fptr;
	void *fdata;
};

struct _iheader {
    int dptr;			/* offset in bytes to first data value    */
    int prefix[3];		/* size of prefix data (bytes)            */
    int suffix[3];		/* size of suffix data (bytes)            */
    int size[3];		/* size of data (pixels)                  */
    int s_lo[3];        /* subset lower range (pixels)            */
    int s_hi[3];		/* subset upper range (pixels)            */
    int s_skip[3];		/* subset skip interval (pixels)          */
	int dim[3];			/* final dimension size */
	int corner;          /* size of 1 whole plane */

    int byte_order;		/* byteorder of data                      */
    int format;			/* data format (INT, FLOAT, etc)          */
    int org;			/* data organization                      */

    float gain, offset;	/* data multiplier and additive offset    */
};

/**
 ** This structure used by parse_args.
 **/
typedef struct {
    char *name;
    int type;
    void *limits;
    void *value;
    int filled;
} Alist;


#define YYSTYPE Vptr
#define YYDEBUG 1
extern YYSTYPE yylval;

extern char error_buf[256];
extern char pp_input_buf[8192];
extern int orders[3][3];
extern Var *VZERO;
extern char *ORG2STR[];
extern char *FORMAT2STR[];
extern int Argc;
extern char **Argv;
extern int VERBOSE;
extern int SCALE;
extern int debug;

#ifndef __MSDOS__ 
#define drand48()       ((double)lrand48()/(unsigned int)(1<<31))
#else
#define readline w_readline
#define add_history w_add_history
#define history_get_history_state w_history_get_history_state
#endif


#include "ufunc.h"
#include "scope.h"
#include "func.h"
#endif /* PARSER_H */
