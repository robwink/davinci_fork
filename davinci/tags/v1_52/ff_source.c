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
int *flinenos;
extern int pp_line;

void
push_input_stream(FILE *fptr)
{
    if (fstack == NULL)  {
        fstack = (FILE **)calloc(fsize, sizeof(FILE *));
		flinenos = (int *)calloc(fsize, sizeof(int));
    } else if (nfstack == fsize) {
        fsize *= 2;
        fstack = (FILE **)my_realloc(fstack, fsize * sizeof(FILE *));
		flinenos = (int *)my_realloc(flinenos, fsize * sizeof(int));
    }

    fstack[nfstack] = fptr;
	flinenos[nfstack] = pp_line;

    ftos = fstack[nfstack];
    pp_line = 0;
    nfstack++;

    sourced++;
}

void
push_input_file(char *name)
{
    FILE *fptr = NULL;
    char *fname;

    if (name == NULL) {
        fptr = stdin;
    } else if ((fptr = fopen(name, "r")) == NULL) {
        if ((fname = dv_locate_file(name)) != NULL) {
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
	pp_line = flinenos[nfstack];

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
	char *filename = NULL;
	char *fname = NULL;

	Alist alist[2];
	alist[0] = make_alist( "filename",    ID_STRING,    NULL,    &filename);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (filename == NULL) {
		parse_error("%s: No filename specified.", func->name);
		return(NULL);
	}
    /**
     ** remove extra quotes from string.
     **/
    p = filename;
    if (*p == '"') p++;
    if (strchr(p, '"')) *(strchr(p,'"')) = '\0';

	if ((fname = dv_locate_file(filename)) == NULL) {
		parse_error("Cannot find file: %s\n", filename);
		return(NULL);
	}
    push_input_file(fname);

    return(NULL);
}
