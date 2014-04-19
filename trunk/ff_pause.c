#include "parser.h"

/**
 **/


/**
 ** ff_pause() - Get a line of input from the user
 **/

Var *
ff_pause(vfuncptr func, Var *arg)
{
	char buf[256];
	char *string = NULL;

	Alist alist[2];
	alist[0] = make_alist( "string",    ID_STRING,    NULL,    &string);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (string != NULL) {
		printf("%s", string);
		fflush(stdout);
	}

	fgets(buf, 256, stdin);
    return(newString(strdup(buf)));
}
