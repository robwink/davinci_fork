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

void list_funcs();

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
    free(f->text);
    free(f->name);
    if (f->argbuf) free(f->argbuf);
    if (f->args) free(f->args);
    if (f->tree) free_tree(f->tree);
    free(f);
}

void
save_ufunc(char *filename)
{
    UFUNC *f;

    f = load_function(filename);
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

UFUNC *
load_function(char *filename)
{
    /**
     ** locate and verify important portions of function definition
     **/
    int i,j;
    struct stat sbuf;
    char *buf, *str, *p;
    int fd;
    int nlen = 0;
    char name[256];
	char line[2048];
	char *q;

    UFUNC *f = NULL, *f2 = NULL;
    Scope *scope;
    void *handle;
	extern char *yytext;
	extern Var *curnode;
	FILE *fp;
	void *parent_buffer;
	extern int local_line;
	extern char *pp_str;

    /**
     ** Get text from file
     **/
    if (stat(filename, &sbuf) != 0) {
        fprintf(stderr, "Internal error: load_function, no file\n");
        return(NULL);
    }
    buf = (char *)calloc(1, sbuf.st_size+1);
	fp = fopen(filename, "r");
    fread(buf, sbuf.st_size, 1, fp);
	fclose(fp);

	unlink(filename);

    buf[sbuf.st_size] = '\0';

    str = buf;

    while(isspace(*str)) str++;
    if (strncasecmp(str, "define", 6)) {
        free(buf);
        fprintf(stderr, "Internal error: load_function, no 'define' in file\n");
        return(NULL);
    }
    str += 6;
    while(*str && isspace(*str)) str++;
    while(*str && (isalnum(*str) || *str == '_')) name[nlen++] = *str++;
    if (nlen == 0) {
        /**
         ** If we want to be nice to the user, we could spit out all
         ** the ufunc's here.
         **/
        free(buf);

		list_funcs();

        return(NULL);
    }
    name[nlen] = '\0';
    if (!(isalpha(*name) || *name == '_')) {
        sprintf(error_buf,"Function name must begin with a letter: %s", name);
        parse_error(NULL);
        free(buf);
        return(NULL);
    }

    f = (UFUNC *)calloc(1, sizeof(UFUNC));
    f->name = strdup(name);
    f->text = buf;
    f->ready = 0;

	/*
	** Added 09/29/00, 
	**
	** See if the function we are replacing is exactly the same.
	** If so, do nothing.
	*/

	if ((f2 = locate_ufunc(f->name)) != NULL) {
		if (!strcmp(f->text, f2->text)) {
			/*
			** Functions are identical, move on.
			*/
			free(f->name);
			free(f->text);
			free(f);
			return(NULL);
		}
	}

    /**
     ** should now find '( args )'.
     ** args should be limited to ids, space and numbers
     **/
    while(*str && isspace(*str)) str++;

    if (*str && *str == '(') {
        str++;
        p = str;
        while(*str && (isalnum(*str) || isspace(*str) || strchr(",_", *str))) 
            str++;
        if (*str && *str == ')') {
            /**
             ** Believe we found a complete args section.  Parse 'em up.
             ** Duplicate the string for cutting on.
             **/
            if (p != str) {
                f->argbuf = strndup((char *)p, str-p);
                split_string(f->argbuf, &f->nargs, &f->args, ",");
            }
            /**
             ** verify arg limits
             **/
            f->min_args = -1;
            f->max_args = -1;
            if (f->nargs) {
                if (strspn(f->args[f->nargs-1], "0123456789")) {
                    f->min_args = atoi(f->args[f->nargs-1]);
                    f->nargs--;
                    if (f->nargs && strspn(f->args[f->nargs-1], "0123456789")) {
                        f->max_args = f->min_args;
                        f->min_args = atoi(f->args[f->nargs-1]);
                        f->nargs--;
                    }
                }
                if (f->max_args != -1 && f->min_args > f->max_args) {
                    parse_error("min_args > max_args.\n");
                    return(f);
                }
            }
            /**
             ** Verify arg names.
             **/
            for (i = 0 ; i < f->nargs ; i++) {
                if (!(isalpha(f->args[i][0]))) {
                    sprintf(error_buf, "Illegal argument name: %s", f->args[i]);
                    parse_error(NULL);
                    return(f);
                }
                for (j = 0 ; j < strlen(f->args[i]) ; j++) {
                    if (!(isalnum(f->args[i][j]) || f->args[i][j] == '_')) {
                        sprintf(error_buf, "Illegal argument name: %s", f->args[i]);
                        parse_error(NULL);
                        return(f);
                    }
                }
            }
        } else {
            /**
             ** Next character is not ')'.  Panic.
             **/
            sprintf(error_buf, "function %s, bad args.", f->name);
            parse_error(NULL);
            return(f);
        }
    } else {
        /**
         ** If we wanted to be nice to the user, we could spit out the
         ** contents of the function named in f->name here.
         **/
        sprintf(error_buf, "loading function %s, no args.", f->name);
        parse_error(NULL);
        return(f);
    }
    str++;

    /**
     ** Presumably, everything else is body.
     **/
    if (*str) f->body = str;

    /**
     ** Shove the function into the stream for evaluation.
     **/
	if (debug) {
		p = f->text;
		q = f->body;
		memcpy(line, p, q-p+1);
		line[q-p+1] = '\0';
		printf("%s", line);
	}
	
	pp_line = local_line;
	p = str;

	parent_buffer = (void *)get_current_buffer();

	while (p && *p) {
		q = strchr(p, '\n');
		if (q == NULL) q = p+strlen(p)-1;
		memcpy(line, p, q-p+1);
		line[q-p+1] = '\0';
		if (debug) printf("%s", line);
		handle = yy_scan_string(line);
		pp_str = line;
		pp_line++;
		while(i = yylex()) {
			j = yyparse(i, (Var *)yytext);

			if (j == 1) {
				f->tree = curnode;
				break;
			}
		}
		yy_delete_buffer((struct yy_buffer_state *)handle);
		p = q+1;
	}
	yy_switch_to_buffer((struct yy_buffer_state *)parent_buffer);
	f->ready = 1;

	/*
	** Take one off  because save_ufunc put one on.
	*/
	pp_line--;

    return(f);
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
			for (i = 0 ; i < f->nargs ; i++) {
				if (!strcmp(f->args[i], V_NAME(p))) {
					break;
				}
			}
			if (i == f->nargs) {
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
        char *zero = (char *)calloc(1,1);
        dd_put(scope_tos(), V_NAME(arg), newVal(BSQ, 1,1,1, BYTE, zero));
	}
	return(NULL);
}
