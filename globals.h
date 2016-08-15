#ifndef GLOBALS_H
#define GLOBALS_H

#include "parser_types.h"
#include <stdio.h>

extern char error_buf[16384];

//only used in main.c so maybe move it there and make it static?
extern char pp_input_buf[8192];

extern int orders[3][3];

extern char* ORG2STR[];
extern char* FORMAT2STR[];



extern int interactive;
extern int continuation;
extern int in_comment;
extern int in_string;

extern int debug;
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
