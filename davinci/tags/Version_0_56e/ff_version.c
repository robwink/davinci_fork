#include "parser.h"
#include "version.h"

Var *
ff_version(vfuncptr func, Var *arg)
{
	Var *s;
	extern char *version;

	s = newVar();
	V_TYPE(s) = ID_STRING;
	V_STRING(s) = strdup(version+5);
	return(s);
}

