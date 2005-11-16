#include "parser.h"
#include "ff_modules.h"

static Var *foo(vfuncptr f, Var *args);
static Var *bar(vfuncptr f, Var *args);

static dvModuleFuncDesc exported_list[] = {
    {"jack", (void *)foo},
    {"sam", (void *)bar}
};

static dvModuleInitStuff is = {
    exported_list, 2,
    NULL, 0
};

DV_MOD_EXPORT int
dv_module_init(
    const char *name,
    dvModuleInitStuff *init_stuff
    )
{
	parse_error("*******************************************************");
	parse_error("*******   mod1.c initialized with %-10s   ********", name);
	parse_error("*******************************************************");

    *init_stuff = is;
    
    return 1; /* return initialization success */
}

static Var *
foo(vfuncptr f, Var *args)
{
    parse_error("%s:%s::foo() called with %p\n", __FILE__, f->name, args);
    return NULL;
}

static Var *
bar(vfuncptr f, Var *args)
{
    parse_error("%s:%s::bar() called with %p\n", __FILE__, f->name, args);
    return NULL;
}

DV_MOD_EXPORT void
dv_module_fini(
    const char *name
)
{
	parse_error("*******************************************************");
	parse_error("********** mod1.c finalized                ************");
	parse_error("*******************************************************");
}

