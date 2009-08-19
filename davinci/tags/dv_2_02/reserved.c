#include "parser.h"

Var *RV_set_order(Var *,Var *,Var *);
Var *RV_set_int(Var *,Var *,Var *);

/**
 ** Search the list of reserved variable names.
 ** If this is one of them, check the input to make
 ** sure it is of the proper type, and do the appropriate equivalence.
 **/

struct _rlist {
    const char *name;
    Var *(*func)(Var *,Var *,Var *);
} rlist[] = {
    { "scale", RV_set_int },
    { "verbose", RV_set_int },
    { "debug", RV_set_int },
    { "depth", RV_set_int },
    { NULL, NULL }
};

int
is_reserved_var(char *name)
{
    struct _rlist *r;
    for (r = rlist ; r->name != NULL ; r++) {
	if (!strcasecmp(name, r->name)) {
	    return(1);
	}
    }
    /**
     ** variable not found.
     **/
    return(0);
}

Var *
set_reserved_var(Var *id, Var *range, Var *exp)
{
    struct _rlist *r;
    for (r = rlist ; r->name != NULL ; r++) {
	if (!strcasecmp(V_NAME(id), r->name)) {
	    return(r->func(id, range, exp));
	}
    }
    /**
     ** variable not found.
     **/
    return(NULL);
}

/**
 ** functions to specifically set reserved variables.
 **/


Var * 
RV_set_int(Var *id,Var *range,Var *exp) 
{
    if (exp == NULL) return(NULL);

    if (V_TYPE(exp) != ID_VAL || V_FORMAT(exp) != INT || V_DSIZE(exp) != 1) {
	sprintf(error_buf, "Improper value for reserved variable: %s", 
		V_NAME(id));
	parse_error(NULL);
	return(NULL);
    }
    V_NAME(exp) = strdup(V_NAME(id));
    put_sym(exp);

    if (!strcmp(V_NAME(id), "verbose")) VERBOSE = V_INT(exp);
    if (!strcmp(V_NAME(id), "SCALE")) SCALE = V_INT(exp);
    if (!strcmp(V_NAME(id), "debug")) debug = V_INT(exp);
    if (!strcmp(V_NAME(id), "DEPTH")) DEPTH = V_INT(exp);
		
    return(id);
}
