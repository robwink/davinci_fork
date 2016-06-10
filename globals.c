#include <setjmp.h>
#include <stdio.h>

#include "parser_types.h"

// These symbols were moved from main.c in order to create
// a windows dll easier.
//
// TODO(rswinkle) move all globals here

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


// from init.c
const char *FORMAT2STR[] = {
	0,
	"byte",
	"short",
	"int",
	"float",
	"vax float",
	"",
	"",
	"double",
	"unsigned short" // drd Bug 2208 Loading a particular hdf5 file kills davinci
};

const char *ORG2STR[] = {
	"bsq",
	"bil",
	"bip"
};


// from array.c
/**
 ** These convert from BSQ to something else
 ** Which is why orders[BIP] looks funny.
 **
 ** This is the "location" of axis N.
 **/

int orders[3][3] = {{0, 1, 2}, {0, 2, 1}, {1, 2, 0}};





