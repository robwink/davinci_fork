#include "parser.h"

/**
 **/


/**
 ** ff_pause() - Get a line of input from the user
 **/

Var *
ff_pause(vfuncptr func, Var *arg)
{
    Var *v, *s;
	char buf[256];

	if ((v = verify_single_string(func, arg)) != NULL) {
		printf("%s", V_STRING(v));
		fflush(stdout);
	}
	fgets(buf, 256, stdin);
    /**
     ** Create the output object.
     **/
    s = newVar();
    V_TYPE(s) = ID_STRING;
	V_STRING(s) = strdup(buf);

    return(s);
}
