#include "parser_types.h"
#include <setjmp.h>
#include <stdio.h>

// These symbols were moved from main.c in order to create
// a windows dll easier.

int interactive  = 1;
int continuation = 0;
int in_comment   = 0;
int in_string    = 0;

int debug = 0;
char pp_input_buf[8192];
FILE* lfile = NULL;
FILE* pfp   = NULL;

int SCALE   = 6;
int VERBOSE = 2;
int DEPTH   = 2;

int allocs = 0;
Var* VZERO;

// This is stuff from the old input.c
int pp_count = 0;
int pp_line  = 0;
int indent   = 0;

Var* curnode = NULL;


#define NUM_TYPE_STRS 13

const char* FORMAT_STRINGS[NUM_TYPE_STRS+1] = {
	"byte",
	"short",
	"int",

	"uint8",
	"uint16",
	"uint32",
	"uint64",

	"int8",
	"int16",
	"int32",
	"int64",

	"float",
	"double",

	NULL

//	,"vax float", "vax int"
};

const int STR_TO_FORMAT[NUM_TYPE_STRS] = {
	DV_UINT8,
	DV_INT16,
	DV_INT32,

	DV_UINT8,
	DV_UINT16,
	DV_UINT32,
	DV_UINT64,

	DV_INT8,
	DV_INT16,
	DV_INT32,
	DV_INT64,

	DV_FLOAT,
	DV_DOUBLE

//	,VAX_FLOAT,
//	VAX_INTEGER
};

#undef NUM_TYPE_STRS

const char* ORG2STR[] = {"bsq", "bil", "bip"};


