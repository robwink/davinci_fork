#include "parser.h"
#include "ff_modules.h"
#include <math.h>
#include <string.h>
#include "api.h"

static Var *foo_dispatch(vfuncptr, Var *);

static dvModuleFuncDesc exported_list[] = {
  { "sqrt", (void *) foo_dispatch },
  { "ceil", (void *) foo_dispatch },
  { "floor", (void *) foo_dispatch },
}; 

static dvModuleInitStuff is = {
  exported_list, sizeof(exported_list)/sizeof(dvModuleFuncDesc),
  NULL, 0
};

dv_module_init(
    const char *name,
    dvModuleInitStuff *init_stuff
    )
{
    *init_stuff = is;
    
    parse_error("Loaded module foo.");
	parse_error("This module uses the function wrapper api to show how external ");
	parse_error("libraries can be wrapped to make a davinci module.");

    return 1; /* return initialization success */

}

/* API CLI definitions */
APIARGS sqrt_args[]={
	{ "sqrt",DOUBLE },
	{ "val",DOUBLE }
};

void
sqrt_wrapper(int ac,APIARGS *av){
	double *dblval = malloc(sizeof(double));
	*dblval = sqrt(*((double *)av[1].argval));	/* double val */
	av[0].argval = dblval;
}

APIARGS ceil_args[]={
	{ "ceil",DOUBLE },
	{ "val",DOUBLE }
};

void
ceil_wrapper(int ac,APIARGS *av){
	av[0].argval = malloc(sizeof(double));
	*((double *)av[0].argval) = ceil(*((double *)av[1].argval));	/* double val */
}

APIARGS floor_args[]={
	{ "floor",DOUBLE },
	{ "val",DOUBLE }
};

void
floor_wrapper(int ac,APIARGS *av){
	av[0].argval = malloc(sizeof(double));
	*((double *)av[0].argval) = floor(*((double *)av[1].argval));	/* double val */
}

/* APIDEFS */
static const APIDEFS foo_apidefs[] = {
	{ "sqrt",sqrt_args,2,sqrt_wrapper }, // #args is = #actual-args + 1 for non-void funcs
	{ "ceil",ceil_args,2,ceil_wrapper },
	{ "floor",floor_args,2,floor_wrapper },
};

void
dv_module_fini(const char *name)
{
  parse_error("Unloaded module foo.");
}


static Var *
foo_dispatch(vfuncptr func, Var *args)
{
	Var *result = NULL;
	int i;

	parse_error("foo_dispatch called with %s\n", func->name);
	for(i=0; i<3; i++){
		if (strcmp(foo_apidefs[i].apiname, func->name) == 0){
			parse_error("Calling function %s\n", func->name);
			result = dispatch_api(&foo_apidefs[i], args);
			parse_error("Function %s returned\n", func->name);
			return result;
		}
	}
	parse_error("%s not found\n", func->name);
	return result;
}
