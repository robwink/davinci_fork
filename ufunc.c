#include "parser.h"

/**
 ** Load function from file.
 ** Find and verify name.
 ** Find split and verify args.
 **/

UFUNC **ufunc_list = NULL;
int nufunc = 0;
int ufsize = 16;
extern int pp_line;
extern char save_file[];

void list_funcs();
UFUNC * make_ufunc(Var *id, Var *args, Var *body);

UFUNC *
locate_ufunc(char *name) 
{
    int i;
    for (i = 0 ; i < nufunc ; i++) {
        if (!strcmp(ufunc_list[i]->name, name)) return(ufunc_list[i]);
    }
    return(NULL);
}

int
destroy_ufunc(char *name)
{
    int i;
    for (i = 0 ; i < nufunc ; i++) {
        if (!strcmp(ufunc_list[i]->name, name)) {
            free_ufunc(ufunc_list[i]);
            ufunc_list[i] = ufunc_list[nufunc-1];
            nufunc--;
            return(1);
        }
    }
    return(0);
}

void
store_ufunc(UFUNC *f) 
{
    if (ufunc_list == NULL) {
        ufunc_list = (UFUNC **)calloc(ufsize, sizeof(UFUNC *));
    } else {
        if (nufunc == ufsize) {
            ufsize *= 2;
            ufunc_list = (UFUNC **)my_realloc(ufunc_list, ufsize * sizeof(UFUNC *));
        }
    }
    ufunc_list[nufunc++] = f;
}

void
free_ufunc(UFUNC *f) 
{
	int i;
    if (f->text) free(f->text);
    if (f->name) free(f->name);
    if (f->argbuf) free(f->argbuf);
    if (f->args) {
		for (i = 0 ; i < f->nargs ; i++) free(f->args[i]);
		free(f->args);
	}
    if (f->tree) free_tree(f->tree);
    free(f);
}

void
save_ufunc(Var *id, Var *args, Var *body)
{
    UFUNC *f;

    f = make_ufunc(id, args, body);
    if (f == NULL) return;
    /**
     ** If a ufunc with this name exists, destroy it
     **/
    if (destroy_ufunc(f->name)) {
        if (VERBOSE) fprintf(stderr, "Replacing function %s\n", f->name);
    } else {
        if (VERBOSE) 
            fprintf(stderr, "Loaded function %s\n", f->name);
    }
    store_ufunc(f);
}


/**
 ** dispatch_ufunc - dispatch a ufunc.
 **
 ** Start a new scope.
 ** Initialize all the functions named args to NOT_PRESENT.
 ** Scan args for keyword pairs.  Pull em off and store 'em in scope
 ** Init $N variables.
 ** Send function text to parser.
 **
 **
 ** Scope should include a symtab pointer to hold memory allocated
 ** while in this scope.  To be deallocated on exit.
 **/
Var *
dispatch_ufunc(UFUNC *f, Var *arg)
{
    Scope *scope = new_scope();
    int i, argc;
    Var *v, *p, *e;
    int insert = 0;

    /**
     ** Create identifiers for all the named arguments.  These dont
     ** yet have pointers to data, indicating they're NOT_PRESENT.
     **/
    for (i = 0 ; i < f->nargs ; i++) {
        dd_put(scope, f->args[i], NULL);
    }
    /**
     ** Parse through args looking for keyword pairs, and storing their
     ** values.  While we are at it, if we encounter a value without a 
     ** keyword, store it in ARGV
     **
     ** Total number of args is stored in $0
     **/
    for ( p = arg ; p != NULL ; p=p->next) {
        if (V_TYPE(p) == ID_KEYWORD) {
            if (dd_find(scope, V_NAME(p)) == 0) {
                sprintf(error_buf, "Unknown keyword to ufunc: %s(... %s= ...)",
                        f->name, V_NAME(p));
                parse_error(NULL);
                free_scope(scope);
                return(NULL);
            } else {
                v = V_KEYVAL(p);
                if ((e = eval(v)) != NULL) v = e;
                dd_put(scope, V_NAME(p), v);
            }
        } else {
            v = p;
            if ((e = eval(v)) != NULL) v = e;
            argc = dd_put_argv(scope, v);

            if (f->max_args >= 0 && argc > f->max_args) {
                sprintf(error_buf, 
                        "Too many arguments to ufunc: %s().  Only expecting %d",
                        f->name, f->max_args);
                parse_error(NULL);
				dd_unput_argv(scope);
                free_scope(scope);
                return(NULL);
            }
        }
    }
    if (f->min_args && (argc = dd_argc(scope)) < f->min_args) {
        sprintf(error_buf, 
                "Not enough arguments to ufunc: %s().  Expecting %d.", 
                f->name, f->min_args);
        parse_error(NULL);
        free_scope(scope);
		dd_unput_argv(scope);
        return(NULL);
    }
    /**
     ** Okay, now we have dealt with all the args.
     ** Push this scope into the scope stack, and run the function.
     **/
    scope->ufunc = f;
    scope_push(scope);
    evaluate(f->tree);

    /**
     **  Additionally, we need to transfer this value OUT of this scope's
     **  symtab, since that memory will go away when the scope is cleaned.
     **/
    dd_unput_argv(scope);
        
    /**
     ** options the user may exercise with the RETURN statement:
     **
     ** return($1)  - scope doesn't own this memory.
     ** return(arg) - scope doesn't own this memory either.
     ** return(sym) - must get this value out of the local symtab, to keep
     **                   it from being free'd when the scope exits.  Stuff
     **                   it into the parent's tmptab to be claimed or free'd
     ** return(tmp) - must get this value out of the local tmptab, to keep
     **                   it from being free'd when the scope exits.  Stuff
     **                   it into the parent't tmptab to be claimed or free'd
	 **
	 ** when we promote a value up to the parent's scope, if the value had
	 ** a name, we need to get rid of it.
     **/
    if ((v = scope->rval) != NULL) {
        if (mem_claim(v) != NULL || rm_symtab(v) != NULL) {
            insert = 1;
        }
    }
    clean_scope(scope_pop());
        
    if (insert) {
        Symtable *sym;
        Scope *scope = scope_tos();

		if (V_NAME(v) != NULL) {
			free(V_NAME(v));
			V_NAME(v) = NULL;
		}

        sym = (Symtable *)calloc(1, sizeof(Symtable));
        sym->value = v;
        sym->next = scope->tmp;
        scope->tmp = sym;
    }
    return(v);
}

/**
 ** find ufunc.  
 ** Spit to file.  
 ** Call editor.
 ** Check file time, and reload if newer.
 **/

Var *
ufunc_edit(vfuncptr func , Var *arg)
{
    UFUNC *ufunc;
    char *name;
    struct stat sbuf;
    time_t time = 0; 
    FILE *fp;
    char buf[256];
    char *fname, *filename, *editor;
    int temp = 0;

    if (arg == NULL) return(NULL);
    if (V_TYPE(arg) == ID_STRING) {
        filename = V_STRING(arg);
        if ((fname = locate_file(filename)) == NULL) {
            fname = filename;
        }
    } else if (V_TYPE(arg) == ID_VAL) {
		/**
		 ** Numeric arg, call hedit to do history editing()
		 **/
		 return(ff_hedit(func,arg));
	} else {
        if ((name  = V_NAME(arg)) == NULL) {
            return(NULL);
        }

        if ((ufunc = locate_ufunc(name)) == NULL) return(NULL);
		/*
		parse_error("Unable to edit function: %s", name);
		return(NULL);
		*/

        fname = tempnam(NULL,NULL);
        fp = fopen(fname, "w");
        fputs(ufunc->text, fp);
        fclose(fp);
        temp=1;
    }

    if (stat(fname, &sbuf) == 0)  {
        time = sbuf.st_mtime;
    }

    if ((editor = getenv("EDITOR")) == NULL) 
        editor = "/bin/vi";

    sprintf(buf, "%s %s", editor, fname);
    system(buf);

    if (stat(fname, &sbuf) == 0) {
        if (time != sbuf.st_mtime) {
            fp = fopen(fname, "r");
            push_input_stream(fp);
        } else {
            fprintf(stderr, "File not changed.\n");
        }
    }
    if (temp) {
		unlink(fname);
		free(fname);
	} else if (filename && fname != filename) free(fname);

    return(NULL);
}

void
list_funcs()
{
	int i;
    for (i = 0 ; i < nufunc ; i++) {
		printf("%s\n", ufunc_list[i]->name);
	}
}

Var *
ff_global(vfuncptr func, Var * arg)
{
	Var *e = get_global_sym(V_NAME(arg));
	if (e != NULL) {
        dd_put(scope_tos(), V_NAME(e), e);
	} else {
		parse_error("%s: variable %s not found", func->name, V_NAME(arg));
	}
	return(NULL);
}

UFUNC *
make_ufunc(Var *id, Var *args, Var *body)
{
	UFUNC *f;
	int ac;
	Var **av;
	int i;

	struct stat sbuf;
	FILE *fp;

    f = (UFUNC *)calloc(1, sizeof(UFUNC));
    f->name = strdup(V_NAME(id));
    f->text = NULL;
    f->ready = 0;

    /**
     ** Get text from file
     **/
    if (stat(save_file, &sbuf) != 0) {
		perror(save_file);
        return(NULL);
    }
    f->text = (char *)calloc(1, sbuf.st_size+1);
	fp = fopen(save_file, "r");
    fread(f->text, sbuf.st_size, 1, fp);
	fclose(fp);

	unlink(save_file);

    /**
     ** should now find '( args )'.
     ** args should be limited to ids, space and numbers
     **/

	f->min_args = -1;
	f->max_args = -1;

	if (args != NULL) {
		make_args(&ac, &av, NULL, args);
		if (av[ac-1] == ID_VAL) {
			f->min_args = V_INT(av[ac-1]);
			ac--;
			if (av[ac-1] == ID_VAL) {
				f->max_args = f->min_args;
				f->min_args = V_INT(av[ac-1]);
				ac--;
			}
		}
		if (f->max_args != -1 && f->min_args > f->max_args) {
			parse_error("min_args > max_args.\n");
			return(f);
		}

		f->nargs = 0;
		f->args = calloc(ac, sizeof(char *));
		for (i = 0 ; i < av ; i++) {
			if (V_TYPE(av[i]) != ID_ID) {
				parse_error("Illegal argument to function: %s", f->name);
				free_ufunc(f);
				return(NULL);
			}
			f->args[i] = V_NAME(av[i]);
			f->nargs++;
		}
	}

	f->ready = 1;
	f->tree = body;

    return(f);
}
