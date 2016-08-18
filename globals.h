#ifndef GLOBALS_H
#define GLOBALS_H

#include "parser_types.h"
#include <stdio.h>

//in error.c
extern char error_buf[16384];

// in array.c
extern int orders[3][3];


//below here, all declared in globals.c
extern char* ORG2STR[];

#define NUM_TYPE_STRS 13
extern const char* FORMAT_STRINGS[NUM_TYPE_STRS];
extern const int STR_TO_FORMAT[NUM_TYPE_STRS];
#undef NUM_TYPE_STRS


extern int interactive;
extern int continuation;
extern int in_comment;
extern int in_string;

extern int debug;
//only used in main.c so maybe move it there and make it static?
extern char pp_input_buf[8192];
extern FILE* lfile;
extern FILE* pfp;

extern int SCALE;
extern int VERBOSE;
extern int DEPTH;

extern int allocs;
extern Var* VZERO;

// This is stuff from the old input.c
extern int pp_count;
extern int pp_line;
extern int indent;

extern Var* curnode;




#endif
