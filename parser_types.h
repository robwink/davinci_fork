#ifndef PARSER_TYPES_H
#define PARSER_TYPES_H

#include "darray.h"
#include "ff_modules.h"

//for size_t
#include <stdlib.h>

#include <stdint.h>

typedef struct _var Var;
typedef Var* Vptr;
/* dvModule is defined in ff_modules.h */
typedef struct _vfuncptr* vfuncptr;
typedef Var* (*vfunc)(struct _vfuncptr*, Var*); /* function caller */

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;



typedef struct Range {
	int dim; /* dimension of data */
	u64 lo[3];
	u64 hi[3];
	u64 step[3];
} Range;

typedef struct Sym {
	int format;   /* format of data */
	size_t dsize; /* total size of data */
	size_t size[3];  /* size of each axis */
	int order;    /* axis application order */
	void* data;
	void* null; /* null value */
	char* title;
} Sym;

typedef struct Node {
	Var* left;
	Var* right;
	Var* parent;

	// An identifier of what type of value this is:
	// Possibilities: Constant, Temporary
	int type;

	int token_number; /* Where in the value table is this puppy located? */
} Node;

typedef struct Vstruct {
	int x_count;
	char** x_names;
	Var** x_data;
} Vstruct;

typedef struct TextArray {
	int Row;
	char** text;
} TextArray;

struct _var {
	int type;
	char* name;
	union {
		Node node;
		Sym sym;
		Range range;
		char* string;
		Var* keyval; /* used by $Keyword */
		Narray* vstruct;
		Narray* args; /* an array of function arguments (same as struct) */
		TextArray textarray;
		dvModule mod;      /* a dynamically loaded module */
		vfuncptr function; /* most likely a module function dereference */
	} value;
};


struct _vfuncptr {
	const char* name;
	vfunc fptr;
	void* fdata;
	void* fdata2;
};


// This structure used by parse_args.
// perhaps it should be in misc.h with it?
typedef struct Alist {
	const char* name;
	int type;
	void* limits;
	void* value;
	int filled;
} Alist;


#define V_NAME(v) (v)->name /* NAME of SYMbol in union */
#define V_TYPE(v) (v)->type /* type of var */

#define V_KEYVAL(v) (v)->value.keyval /* keyword value */
#define V_STRING(v) (v)->value.string
#define V_RANGE(v) (&((v)->value.range)) /* range value */
#define V_NODE(v) (&((v)->value.node))
#define V_SYM(v) (&((v)->value.sym)) /* SYMbol value in union */

#define V_DATA(v) V_SYM(v)->data      /* pointer to data */
#define V_INT(v) (*((int*)V_DATA(v))) /* derefernce as a single int */
/* #define V_INT64(v)  (*((int64 *)V_DATA(v))) / * derefernce as a single int64 */
#define V_FLOAT(v) (*((float*)V_DATA(v)))   /* derefernce as a single float */
#define V_DOUBLE(v) (*((double*)V_DATA(v))) /* derefernce as a single dbl */
#define V_FORMAT(v) V_SYM(v)->format
#define V_DSIZE(v) V_SYM(v)->dsize
#define V_SIZE(v) V_SYM(v)->size
#define V_ORDER(v) V_SYM(v)->order
#define V_ORG(v) V_SYM(v)->order

#define V_HISTORY(v) V_SYM(v)->history
#define V_TITLE(v) V_SYM(v)->title

#define V_TEXT(v) (v)->value.textarray

#define V_STRUCT(v) (v)->value.vstruct
#define V_ARGS(v) (v)->value.args
#define V_MODULE(v) (v)->value.mod
#define V_FUNC(v) (v)->value.function

#define newVar (Var*)mem_malloc

// The following define the various types for Var->type
enum {
	ID_NONE  = 0, /* a non value */
	ID_ERROR = 99,
	ID_BASE  = 100, /* in case of conflicts */
	ID_UNK,         /* Unknown type - also used as a generic type */
	ID_STRING,      /* NULL terminated character string */
	ID_KEYWORD,     /* keyword argument */
	ID_VAL,         /* everything with dim != 0 */
	ID_STRUCT,      /* Structure */
	ID_TEXT,        /*1-D Array of Strings*/

	ID_IVAL, /* Integer value */
	ID_RVAL, /* real value */
	ID_ID,   /* Identifier */

	ID_LIST,   /* Statement list */
	ID_IF,     /* if statement */
	ID_ELSE,   /* else statement */
	ID_WHILE,  /* while statement */
	ID_CONT,   /* continue statement */
	ID_BREAK,  /* break statement */
	ID_RETURN, /* return statement */

	ID_RANGES, /* list of ranges */
	ID_RSTEP,  /* list of ranges */
	ID_RANGE,  /* single range value */
	ID_SET,    /* equivalence expression */
	ID_OR,     /* logical (||) or */
	ID_AND,    /* logical (&&) and */
	ID_EQ,     /* logical (==) equals */
	ID_NE,     /* logical (!=) not equals */
	ID_LT,     /* logical (<) less than */
	ID_GT,     /* logical (>) greater than */
	ID_LE,     /* logical (<=) less than or equal */
	ID_GE,     /* logical (>=) greater than or equal */
	ID_ADD,    /* addition */
	ID_SUB,    /* subtraction */
	ID_MULT,   /* multiplcation */
	ID_DIV,    /* division */
	ID_MOD,    /* modulo divison */
	ID_UMINUS, /* unary minus */
	ID_LSHIFT, /* left shift */
	ID_RSHIFT, /* right shift */
	ID_FUNCT,  /* function */
	ID_ARRAY,  /* application of ranges to array */
	ID_ARG,    /* list of arguments */
	ID_ARGS,   /* single argument */

	ID_FOR,     /* for loop */
	ID_FOREACH, /* foreach loop */
	ID_EACH,    /* foreach val */
	ID_ARGV,    /* $VALUE argument. Evalue at run */

	ID_INC,    /* increment value */
	ID_DEC,    /* decrement value */
	ID_INCSET, /* increment value */
	ID_DECSET, /* decrement value */
	ID_MULSET, /* *= value */
	ID_DIVSET, /* /= value */

	ID_POW,   /* exponent */
	ID_CAT,   /* concatenate */
	ID_ENUM,  /* enumerated argument, not parsed */
	ID_DECL,  /* Declaration */
	ID_WHERE, /* Where */
	ID_DEREF, /* Structure dereference */

	ID_CONSTRUCT,   /* Structure constructor */
	ID_DECONSTRUCT, /* Structure deconstructor */

	ONE_AXIS, /* argument options */
	ANY_AXIS, /* argument options */

	ID_LINE, /* A lexical token */

	ID_MODULE,   /* davinci module variable ID */
	ID_FUNCTION, /* davinci module function variable ID */
	ID_PARALLEL, /* parallelization */
	ID_VARARGS,  /* varargs arguments */

	ID_FPTR /* a function pointer */
};

// TODO(rswinkle) make these an enum?

enum {
	DV_UINT8 = 1,
	DV_UINT16,
	DV_UINT32,
	DV_UINT64,

	DV_INT8,
	DV_INT16,
	DV_INT32,
	DV_INT64,

	DV_FLOAT,
	DV_DOUBLE,

// deprecated
	VAX_FLOAT,
	VAX_INTEGER
};

#define NBYTES(a) dv_format_size(a)

int dv_format_size(int type);

/**
 ** Data axis order
 ** Var->value.Sym->order
 **
 ** !!! CAUTION: these values must be 0 based.  They are used as array
 **              indices below.
 **/

#define BSQ 0
#define BIL 1
#define BIP 2

#define Format2Str(i) dv_format_to_str(i)

const char* dv_format_to_str(int type);


#define Org2Str(i) (((i) >= BSQ && (i) <= BIP) ? ORG2STR[(i)] : "undef")

#define GetSamples(s, org) (s)[((org) == BIP ? 1 : 0)]
#define GetLines(s, org) (s)[((org) == BSQ ? 1 : 2)]
#define GetBands(s, org) (s)[((org) == BIP ? 0 : ((org) == BIL ? 1 : 2))]

#define GetX(s) GetSamples(V_SIZE(s), V_ORG(s))
#define GetY(s) GetLines(V_SIZE(s), V_ORG(s))
#define GetZ(s) GetBands(V_SIZE(s), V_ORG(s))
#define GetNbytes(s) NBYTES(V_FORMAT(s))

#define clamp(v, lo, hi) ((v) > (lo) ? ((v) < (hi) ? (v) : (hi)) : (lo))
#define clamp_byte(v) clamp(v, 0, UINT8_MAX)
#define clamp_u8(v) clamp(v, 0, UINT8_MAX)
#define clamp_u16(v) clamp(v, 0, UINT16_MAX)
#define clamp_u32(v) clamp(v, 0, UINT32_MAX)
#define clamp_u64(v) clamp(v, 0, UINT64_MAX)

#define clamp_i8(v) clamp(v, INT8_MIN, INT8_MAX)

#define clamp_i16(v) clamp(v, (INT16_MIN), (INT16_MAX))
#define clamp_short(v) clamp_i16(v)

#define clamp_i32(v) clamp(v, (INT32_MIN), (INT32_MAX))
#define clamp_int(v) clamp_i32(v)

#define clamp_i64(v) clamp(v, (INT64_MIN), (INT64_MAX))

#define clamp_float(v) v
#define clamp_double(v) v


#endif
