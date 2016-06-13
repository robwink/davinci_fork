#ifndef PARSER_TYPES_H
#define PARSER_TYPES_H

#include "darray.h"
#include "ff_modules.h"

//for size_t
#include <stdlib.h>

typedef struct _var Var;
typedef Var* Vptr;
/* dvModule is defined in ff_modules.h */
typedef struct _vfuncptr* vfuncptr;
typedef Var* (*vfunc)(struct _vfuncptr*, Var*); /* function caller */

typedef struct Range {
	int dim; /* dimension of data */
	int lo[3];
	int hi[3];
	int step[3];
} Range;

typedef struct Sym {
	int format;   /* format of data */
	size_t dsize; /* total size of data */
	int size[3];  /* size of each axis */
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

#endif
