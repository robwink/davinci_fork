/******************************* ff_source.c **********************************/
#include "parser.h"

/**
 ** This file contains routines to handle pushing and popping of input files.
 **/

FILE *ftos;
static FILE **fstack=NULL;
int nfstack=0;
static int fsize=16;
int sourced=0;

void
push_input_stream(FILE *fptr)
{
    if (fstack == NULL)  {
        fstack = (FILE **)calloc(fsize, sizeof(FILE *));
    } else if (nfstack == fsize) {
        fsize *= 2;
        fstack = (FILE **)my_realloc(fstack, fsize * sizeof(FILE *));
    }

    fstack[nfstack] = fptr;
    ftos = fstack[nfstack];
    nfstack++;

    sourced++;
}

void
push_input_file(char *name)
{
    FILE *fptr;
    char *fname;

    if (name == NULL) {
        fptr = stdin;
    } else if ((fptr = fopen(name, "r")) == NULL) {
        if ((fname = locate_file(name)) != NULL) {
            fptr = fopen(fname, "r");
            free(fname);
        }
        if (fptr == NULL)  {
            sprintf(error_buf, "Could not source file: %s", name);
            parse_error(NULL);
            return;
        }
    }

    push_input_stream(fptr);
}

/**
 ** Close (pop) currently opened file, and return next one on stack.
 **/

void
pop_input_file()
{
    FILE *fp;

    ftos = NULL;

    fp = fstack[--nfstack];
    if (fileno(fp) != 0) {
        fclose(fp);
    }
    sourced--;
    if (sourced < 0) sourced = 0;

    if (nfstack == 0) 
        return;

    ftos = fstack[nfstack-1];
}

int
is_file(char *name) 
{
    struct stat sbuf;

    if ((stat(name, &sbuf)) != 0) {
        return(0);
    }
    return(1);
}
/**
 ** ff_source() - Source a script file
 **
 ** This function pushes another file onto the file stack.  It takes
 ** a single STRING arg.
 **/

Var *
ff_source(vfuncptr func, Var *arg)
{
    char *p;

    if (arg == NULL) {
        return(NULL);
    } else if (arg->next != NULL) {
        sprintf(error_buf, "Too many arguments to function: %s()", func->name);
        parse_error(NULL);
        return(NULL);
    }
    if (V_TYPE(arg) != ID_STRING) {
        sprintf(error_buf, "Invalid argument to function: %s()", func->name);
        parse_error(NULL);
        return(NULL);
    }
    /**
     ** remove extra quotes from string.
     **/
    p = V_STRING(arg);
    if (*p == '"') p++;
    if (strchr(p, '"')) *(strchr(p,'"')) = '\0';

    push_input_file(p);

    return(NULL);
}
