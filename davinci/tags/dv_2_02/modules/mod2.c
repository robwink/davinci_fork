#include "parser.h"
#include "ff_modules.h"

static Var *foo(vfuncptr f, Var *args);
static Var *bar(vfuncptr f, Var *args);

static dvModuleFuncDesc exported_list[] = {
    {"foo", (void *)foo},
    {"bar", (void *)bar}
};

static dvDepAttr dependencies[] = {
	{"mod1", NULL}
};

static dvModuleInitStuff is = {
    exported_list, 2,
    dependencies, 1
};

DV_MOD_EXPORT int
dv_module_init(
    const char *name,
    dvModuleInitStuff *init_stuff
    )
{
    fprintf(stderr, "mod2.c::dv_module_init() called with %s\n", name);

    *init_stuff = is;
    
    return 1; /* return initialization success */
}

static Var *
foo(vfuncptr f, Var *args)
{
    fprintf(stderr, "mod2.c::%s::foo() called with %p\n", f->name, args);
    return newInt(1);
}

static Var *
bar(vfuncptr f, Var *args)
{
    fprintf(stderr, "mod2.c::%s::bar() called with %p\n", f->name, args);
    return NULL;
}

