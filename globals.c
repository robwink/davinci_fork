#include "func.h"
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


// TODO (rswinkle) move the functions to more appropriate place(s)
// and add similar small functions from main.c to those places

void quit(int return_code)
{
	char* path = getenv("TMPDIR");

	if (interactive) {
		printf("\n");
#if defined(USE_X11_EVENTS) && defined(HAVE_LIBREADLINE)
		/* JAS FIX */
		rl_callback_handler_remove();
#endif
	}

	clean_scope(scope_tos());

	// clean up temporary directory
	rmrf(path);
	exit(return_code);
}

void make_sym(Var* v, int format, char* str)
{
	V_FORMAT(v)  = format;
	V_DSIZE(v)   = 1;
	V_SIZE(v)[0] = V_SIZE(v)[1] = V_SIZE(v)[2] = 1;
	V_ORG(v)                                   = BSQ;
	V_DATA(v)                                  = calloc(1, NBYTES(format));

	switch (format) {
	case INT: *((int*)(V_DATA(v))) = strtol(str, NULL, 10); break;
	case FLOAT: {
		double d;
		d = strtod(str, NULL);
		// NOTE(rswinkle) this only works because we apply unary minus separately later
		// otherwise you'd have to check against -FLT_MAX and -FLT_MIN too
		// also right here is an easy fix to the float intermediate bug/design choice
		if (((d > FLT_MAX) || (d < FLT_MIN)) && (d != 0)) {
			free(V_DATA(v));
			V_DATA(v)               = calloc(1, NBYTES(DOUBLE));
			V_FORMAT(v)             = DOUBLE;
			*((double*)(V_DATA(v))) = d;
		} else {
			*((float*)(V_DATA(v))) = d;
		}
	}
	}
}



void yyerror(char* s)
{
	extern int pp_count;
	extern int pp_line;

	printf("***%*s^ ", pp_count, " ");
	printf("%s, line %d\n", s, pp_line);
}

int yywrap()
{
	return 1;
}

char* unquote(char* name)
{
	char* p = name;

	if (name == NULL) return NULL;
	if (*name == 0) return name;
	if (*name == '"') {
		p++;
		name++;
		while (*name) {
			if (*name == '"' && *(name - 1) != '\\') break;
			name++;
		}
		*name = '\0';
	} else if (*name == '\'') {
		p++;
		name++;
		while (*name) {
			if (*name == '\'' && *(name - 1) != '\\') break;
			name++;
		}
		*name = '\0';
	}
	return p;
}

char* unescape(char* str)
{
	char* p = str;
	char* q = str;

	if (str && *str) {
		while (*p) {
			if (*p == '\\') {
				if (*(p + 1) == 't')
					*q = '\t';
				else if (*(p + 1) == 'n')
					*q = '\n';
				else
					*q = *(p + 1);
				p++;
			} else {
				*q = *p;
			}
			p++;
			q++;
		}
		*q = *p;
	}
	return str;
}
