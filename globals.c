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


const char* FORMAT2STR[] = {
    0,           "byte",    "uint16", "uint32", "uint64",

    "int8",
    "short", // really int16
    "int",   // really int32
    "int64",

    "float",     "double",

    "vax float", "vax int",
};

const char* ORG2STR[] = {"bsq", "bil", "bip"};
