/********************************** pp.c *********************************/
#include "parser.h"

/**
 ** pp_emit_prompt()    - spit out prompt if interactive 
 **/

void commaize(char *);

/*
void 
pp_emit_prompt()
{
    extern int interactive;
    extern int sourced;
	int i;

    if (sourced <= 1 && interactive) {
		if (indent)
			printf("%2d> ",indent);
		else {
			printf("dv> ");
		}
        fflush(stdout);
    }
}
*/

Var *
V_DUP(Var *v)
{
    Var *r;
    int dsize;

    r = new(Var);
    memcpy(r, v, sizeof(Var));

    switch (V_TYPE(v)) {
    case ID_VAL:
    {
        memcpy(V_SYM(r), V_SYM(v), sizeof(Sym));
        dsize = V_DSIZE(v)*NBYTES(V_FORMAT(v));
        V_SYM(r)->data = memcpy(malloc(dsize), V_SYM(v)->data, dsize);
        if (V_TITLE(v)) V_TITLE(r) = strdup(V_TITLE(v));
    }
    break;
    case ID_STRING:
        V_STRING(r) = strdup(V_STRING(v));
        break;
    case ID_UNK:
        if (V_NAME(v)) V_NAME(r) = strdup(V_NAME(v));
        break;
    }
    return(r);
}


Var *
pp_print(Var *v)
{
    extern int SCALE;
    int i;
    Var *s;
    char bytes[32];

    if (v == NULL) return(v);
    if (VERBOSE == 0) return(v);

    /**
    ** Evaluate SCALE here.
    **/

    s = eval(v);
    if (s != NULL) {
        v = s;
    }
    switch (V_TYPE(v)) {
    case ID_STRING:
        printf("\"%s\"\n", V_STRING(v));
        break;
    case ID_VAL:
        /**
        ** This should iterate in XYZ order, always.
        **/
        sprintf(bytes, "%d", NBYTES(V_FORMAT(v))*V_DSIZE(v));
        commaize(bytes);

        if (V_DSIZE(v) > 1) {
            printf("%s:\t%dx%dx%d array of %s, %s format [%s bytes]\n",
                   V_NAME(v) ? V_NAME(v) : "", 	
                   GetSamples(V_SIZE(v),V_ORG(v)),
                   GetLines(V_SIZE(v),V_ORG(v)),
                   GetBands(V_SIZE(v),V_ORG(v)),
                   Format2Str(V_FORMAT(v)),
                   Org2Str(V_ORG(v)),
                   bytes);
        }
        if (V_DSIZE(v) < 100) {
            for (i = 0 ; i < V_DSIZE(v) ; i++) {
                if (i) {
                    if (i % V_SIZE(v)[0] == 0)
                        printf("\n");
                    if (i % (V_SIZE(v)[1]*V_SIZE(v)[0]) == 0)
                        printf("\n");
                }
                switch (V_FORMAT(v)) {
                case BYTE: printf("%d\t", ((u_char *)V_DATA(v))[i]); break;
                case SHORT: printf("%d\t", ((short *)V_DATA(v))[i]); break;
                case INT:  printf("%d\t", ((int *)V_DATA(v))[i]); break;
                case FLOAT: printf("%#.*g\t", SCALE, ((float *)V_DATA(v))[i]); break;
                case DOUBLE: printf("%#.*g\t", SCALE, ((double *)V_DATA(v))[i]); break;
                }
            }
            printf("\n");
        }
        break;
    default:
        printf("error: Unknown type.\n");
        break;
    }
    return(v);
}

/**
 ** pp_set_var() - perform variable equivalence.
 **/

Var *
pp_set_var(Var *id, Var *range, Var *exp) 
{
    Var *v, *e;
    int i,j,k;
    int size[3], s1,s2,s3,d1,d2,d3, od[3], os[3], d, s;
    Range *r, rout;

    /**
    ** If exp is named, it is a simple variable substitution.
    ** If its not named, we can use its memory directly.
    **/
    if (exp == NULL) return(NULL);
    if (range != NULL) {
        /**
        ** The user has requested an array replacement.
        **/
        v = id;
        if ((e = eval(v)) != NULL) v = e;
        if ((e = eval(exp)) != NULL) exp = e;

        /**
	 ** Verify that the src and destination pieces 
	 ** are legal values, and the same size
	 **/
        
        r = V_RANGE(range);
        if (fixup_ranges(v, r, &rout) == 0) {
            parse_error("Illegal range value.");
            return(NULL);
        }

        for (i =0 ; i < 3 ; i++) {
            size[i] = 1 + (rout.hi[i] - rout.lo[i])/rout.step[i];
            j = orders[V_ORG(exp)][i];
            if (size[i] != V_SIZE(exp)[j]) {
                parse_error("Array sizes don't match");
                return(NULL);
            }
        }
        r = &rout;
        
        for (i = 0 ; i < size[0] ; i++) {
            for (j = 0 ; j < size[1] ; j++) {
                for (k = 0 ; k < size[2] ; k++) {

                    d = cpos(i*r->step[0] + r->lo[0],
                             j*r->step[1] + r->lo[1],
                             k*r->step[2] + r->lo[2], v);

                    s = cpos(i,j,k, exp);

                    switch(V_FORMAT(v)) {
                    case BYTE:
                        ((u_char *)V_DATA(v))[d] =
                            saturate_byte(extract_int(exp, s));
                        break;
                    case SHORT:
                        ((short *)V_DATA(v))[d] =
                            saturate_short(extract_int(exp, s));
                        break;
                    case INT:
                        ((int *)V_DATA(v))[d] =
                            saturate_int(extract_int(exp, s));
                        break;
                    case FLOAT:
                        ((float *)V_DATA(v))[d] =
                            extract_float(exp, s);
                        break;
                    case DOUBLE:
                        ((double *)V_DATA(v))[d] =
                            extract_double(exp, s);
                        break;
                    }
                }
            }
        }
        /**
        ** go ahead and pull out the range to return.
        **/
        return(pp_range(id, range));
    }

    /**
    ** Check for reserved variables and verify their type.
    **/

    /**
    ** this does the actual equivalence.
    ** If the rhs is a named value, duplicate it.
    ** otherwise, use the memory directly.
    **/
    if (V_NAME(exp) != NULL) {
        v = eval(exp);
        if (v != NULL) {
            exp = V_DUP(v);
        }
    } else if (mem_claim(exp) == NULL) {
        /**
        ** if we can't claim the memory, we can't use it.
        **/
        exp = V_DUP(exp);
    }

    V_NAME(exp) = strdup(V_NAME(id));

    if (!strcmp(V_NAME(exp), "verbose")) VERBOSE = V_INT(exp);
    if (!strcmp(V_NAME(exp), "scale")) SCALE = V_INT(exp);

    put_sym(exp);
    return(id);
}

/**
 ** Ranges:
 **/

/**
 ** pp_mk_range() - make a range value, with specified from exression
 **/
Var *
pp_mk_range(Var *r1, Var *r2) 
{
    int v1=0, v2=0;
    int format;
    Var *v;

    r1 = eval(r1);
    r2 = eval(r2);

    if (r1) {
        format = V_FORMAT(r1);
        if (format != INT && format != SHORT && format != BYTE) {
            parse_error("(r1) Invalid range value.");
            return(NULL);
        }
        v1 = extract_int(r1,0);
    }

    if (r2) {
        format = V_FORMAT(r2);
        if (format != INT && format != SHORT && format != BYTE) {
            parse_error("(r2) Invalid range value");
            return(NULL);
        }
        v2 = extract_int(r2,0);
    }

    v = new(Var);

    V_RANGE(v)->lo[0] = v1;
    V_RANGE(v)->hi[0] = v2;
    V_RANGE(v)->step[0] = 0;
    V_RANGE(v)->dim++;
    V_TYPE(v) = ID_RANGE;
    V_NEXT(v) = NULL;

    return(v);
}

/**
 ** pp_mk_rstep() - make a range value, including step value.
 **/
Var *
pp_mk_rstep(Var *r1, Var *r2) 
{
    int v1=0;
    int format;
    Var *v;

    r1 = eval(r1);
    r2 = eval(r2);

    if (r1 == NULL) {
		r1 = new(Var);
    	V_RANGE(r1)->lo[0] = 0;
		V_RANGE(r1)->hi[0] = 0;
		V_RANGE(r1)->dim++;
		V_TYPE(r1) = ID_RANGE;
	}

    if (r2) {
        format = V_FORMAT(r2);
        if (format != INT && format != SHORT && format != BYTE) {
            parse_error("(r2) Invalid range value");
            return(NULL);
        }
        v1 = extract_int(r2,0);
    }

    V_RANGE(r1)->step[0] = v1;
    V_NEXT(r1) = NULL;

    return(r1);
}


Var *
pp_add_range(Var *r, Var *v) 
{
    int dim;

    if (r == NULL || v == NULL) return(NULL);

    if ((dim = V_RANGE(r)->dim) == 3) {
        parse_error("Too many range values");
        return(NULL);
    }
    V_RANGE(r)->lo[dim] = V_RANGE(v)->lo[0];
    V_RANGE(r)->hi[dim] = V_RANGE(v)->hi[0];
    V_RANGE(r)->step[dim] = V_RANGE(v)->step[0];
    V_RANGE(r)->dim++;

    return(r);
}

/**
 ** pp_range() - extract specified portion array.
 **
 **/
Var *
pp_range(Var *v, Var *r)
{
    Var *t;
    Var *out;

    /**
    ** Do some basic error detection
    **/
    if (v == NULL || r == NULL) return(NULL);

    t = eval(v);
    if (t == NULL) {
        sprintf(error_buf, "Symbol not found: %s", V_NAME(v));
        parse_error(NULL);
        return(NULL);
    }
    v = t;
    if (V_TYPE(v) != ID_VAL) {
        sprintf(error_buf, "Illegal type: %s", V_NAME(v));
        parse_error(NULL);
        return(NULL);
    }

    /**
    ** Swap array subsets to match $ORDER
    swap_ranges(r); 
    **/

    out = extract_array(v,V_RANGE(r));

    return(out);
}

/**
 ** pp_func() - call named function with arglist.
 **
 ** This needs to be exapanded to figure out which call is necessary
 ** for the named function.
 **/

Var *
pp_func(Var *function, Var *arglist)
{
    Var *p;
    char *ptr;

    if (V_NAME(function)[0] == '$') {
        p = eval(function);
        ptr = V_STRING(p);
    } else {
        ptr = V_NAME(function);
    }

    return(V_func(ptr, arglist));
}

/**
 ** pp_mk_arglist() - Append arg to arglist
 **
 **     Either value can be NULL.
 **     arglist == NULL, make new arglist.
 **     arg == NULL, entire arglist is NULL
 **/
Var *
pp_mk_arglist(Var *arglist, Var *arg) 
{
    Var *p;
    if (arglist == NULL) {
        arglist = arg;
        return(arglist);
    }
    if (arg == NULL) {
        return(arglist);
    }

    for (p = arglist ; p != NULL ; p = p->next) {
        if (p->next == NULL) {
            p->next = arg;
            arg->next = NULL;
            break;
        }
    }
    return(arglist);
}

/* convert keyword pair to arg */
Var *
pp_keyword_to_arg(Var *keyword, Var *ex) 
{
    V_TYPE(keyword) = ID_KEYWORD;
    V_KEYVAL(keyword) = ex;
    return(keyword);
}


/**
 ** pp_shellArgs() - return argument specified on the shell command line.
 **/

Var *
pp_shellArgs(Var *v)
{
    int n;
    char name[256];
    Var *s=NULL;
    char *value;

    if (V_TYPE(v) == ID_VAL) {
        n = V_INT(v);
        sprintf(name,"$%d", n);
    } else {
        strcpy(name, V_NAME(v));
    }

    if ((value = get_env_var(name)) == NULL) {
        return(NULL);
    }
	

    /**
    ** if symbol is not in symtab, put it there.
    **/
    if ((s = get_sym(name)) == NULL) {
        s = new(Var);
        V_TYPE(s) = ID_STRING;
        V_STRING(s) = strdup(value);
        V_NAME(s) = strdup(name);
        put_sym(s);
    }
    /**
    ** This cannot return the Var we just put into the symtab, cause 
    ** it will have ->next already set.  Make a new Var containing
    ** just its name.
    **/
    s = new(Var);
    V_NAME(s) = strdup(name);
    V_TYPE(s) = ID_UNK;

    return(s);
}

/**
 ** pp_argv() - Get $arg value from current scope.
 **
 **/

Var *
pp_argv(Var *left, Var *right)
{
    int n;
    char name[256];
    Var *s=NULL, *v;
    char *value;
    Scope *scope;

    scope = scope_tos();

    if (right != NULL) {
        if (strcasecmp(V_NAME(left), "argv")) {
            sprintf(error_buf, "Unable to subscript $%s", V_NAME(left));
            parse_error(NULL);
            return(NULL);
        }
        v = eval(right);
    } else {
        v = left;
    }

    if (V_TYPE(v) == ID_VAL) {
        n = V_INT(v);
        if (n == 0) {
            /**
            ** $0 is a special case.  For the global scope, return
            ** argv[0].  For everyone else, find the function name
            ** and return it.  In both these cases, $0 has already
            ** been stuffed (by name) into the scope->dd
            **/
            return(get_sym("$0"));
        }
        if (n > dd_argc(scope)) {
            sprintf(error_buf, "Argument does not exist: $%d\n", n);
            parse_error(NULL);
            return(NULL);
        } else {
            /**
            ** This returns memory from the dd.  Don't free it.
            **/
            return(dd_get_argv(scope,n));
        }
    } else if (!strcasecmp(V_NAME(v), "argc")) {
        /**
        ** special case, number of dd->args.
        **/
        return(dd_argc_var(scope));
    } else {
        strcpy(name, V_NAME(v));
        if ((value = get_env_var(name)) == NULL) {
            return(NULL);
        }
        /**
        ** if symbol is not in symtab, put it there.
        **/
        if ((s = get_global_sym(name)) == NULL) {
            s = new(Var);
            V_TYPE(s) = ID_STRING;
            V_STRING(s) = strdup(value);
            V_NAME(s) = strdup(name);
            put_global_sym(s);
        }
        return(s);
    }
}

Var *
pp_math_strings(Var *exp1, int op, Var *exp2)
{
    Var *s,*e;

    int i,j,k;
    int format;
    double d1,d2;
    char *ptr;

    if (exp1 == NULL || exp2 == NULL) return(NULL);

    if ((e = eval(exp1)) == NULL) {
        sprintf(error_buf, "Variable not found: %s", V_NAME(exp1));
        parse_error(NULL);
        return (NULL);
    }
    exp1 = e;
    if ((e = eval(exp2)) == NULL) {
        sprintf(error_buf, "Variable not found: %s", V_NAME(exp2));
        parse_error(NULL);
        return (NULL);
    }
    exp2 = e;

    if (V_TYPE(exp1) == ID_STRING && V_TYPE(exp2) == ID_STRING) {
        i = strcmp(V_STRING(exp1), V_STRING(exp2));
        switch (op) {
        case ID_EQ:		k = (i == 0);	break;
        case ID_NE:		k = (i != 0);	break;
        case ID_LT:		k = (i < 0);	break;
        case ID_GT:		k = (i > 0);	break;
        case ID_LE:		k = (i <= 0);	break;
        case ID_GE:		k = (i >= 0);	break;
		case ID_ADD:	return(pp_add_strings(exp1, exp2)); break;
        case ID_OR:
        case ID_AND:
            parse_error("Cannot perform boolean logic on type STRING");
            return(NULL);
        }
    } else if (op != ID_ADD) {
		parse_error("Operation not supported on string and non-string");
		return(NULL);
	} else {
		return(pp_add_strings(exp1, exp2));
	}

    s = new(Var);
    V_TYPE(s) = ID_VAL;
    V_SIZE(s)[0] = V_SIZE(s)[1] = V_SIZE(s)[2] = V_DSIZE(s) = 1;
    V_FORMAT(s) = INT;
    V_ORG(s) = BSQ;
    V_DATA(s) = calloc(1, sizeof(int));
    V_INT(s) = k;

    return(s);
}

Var *
pp_relop(Var *exp1, int op, Var *exp2)
{
    Var *s,*e;

    int i,j,k;
    int format;
    double d1,d2;
    char *ptr;

    if (exp1 == NULL || exp2 == NULL) return(NULL);

    if ((e = eval(exp1)) == NULL) {
        sprintf(error_buf, "Variable not found: %s", V_NAME(exp1));
        parse_error(NULL);
        return (NULL);
    }
    exp1 = e;
    if ((e = eval(exp2)) == NULL) {
        sprintf(error_buf, "Variable not found: %s", V_NAME(exp2));
        parse_error(NULL);
        return (NULL);
    }
    exp2 = e;

    if (V_TYPE(exp1) == ID_STRING && V_TYPE(exp2) == ID_STRING) {
        i = strcmp(V_STRING(exp1), V_STRING(exp2));
        switch (op) {
        case ID_EQ:		k = (i == 0);	break;
        case ID_NE:		k = (i != 0);	break;
        case ID_LT:		k = (i < 0);	break;
        case ID_GT:		k = (i > 0);	break;
        case ID_LE:		k = (i <= 0);	break;
        case ID_GE:		k = (i >= 0);	break;
        case ID_OR:
        case ID_AND:
            parse_error("Cannot perform boolean logic on type STRING");
            return(NULL);
        }
    } else {
        if (V_TYPE(exp1) != ID_VAL || V_TYPE(exp2) != ID_VAL) {
            parse_error("Relational operators require values.");
            return(NULL);
        }
        if (V_DSIZE(exp1) != 1 || V_DSIZE(exp2) != 1) {
            parse_error("Relational operators only work with single values.");
            return(NULL);
        }

        format = max(V_FORMAT(exp1),V_FORMAT(exp2));
        switch(format) {
        case BYTE:
        case SHORT:
        case INT:
            i = extract_int(exp1,0);
            j = extract_int(exp2,0);
            switch (op) {
            case ID_EQ:		k = (i == j);	break;
            case ID_NE:		k = (i != j);	break;
            case ID_LT:		k = (i < j);	break;
            case ID_GT:		k = (i > j);	break;
            case ID_LE:		k = (i <= j);	break;
            case ID_GE:		k = (i >= j);	break;
            case ID_OR:		k = (i || j);	break;
            case ID_AND:	k = (i && j);	break;
            }
            break;
        case FLOAT:
        case DOUBLE:
            d1 = extract_double(exp1,0);
            d2 = extract_double(exp2,0);
            switch (op) {
            case ID_EQ:		k = (d1 == d2);	break;
            case ID_NE:		k = (d1 != d2);	break;
            case ID_LT:		k = (d1 < d2);	break;
            case ID_GT:		k = (d1 > d2);	break;
            case ID_LE:		k = (d1 <= d2);	break;
            case ID_GE:		k = (d1 >= d2);	break;
            case ID_OR:		k = (d1 || d2);	break;
            case ID_AND:	k = (d1 && d2);	break;
            }
            break;
        }
    }

    s = new(Var);
    V_TYPE(s) = ID_VAL;
    V_SIZE(s)[0] = V_SIZE(s)[1] = V_SIZE(s)[2] = V_DSIZE(s) = 1;
    V_FORMAT(s) = INT;
    V_ORG(s) = BSQ;
    V_DATA(s) = calloc(1, sizeof(int));
    V_INT(s) = k;

    return(s);
}

char *
toBytes(int i)
{
    static char buf[256];


    if (i < 10240) {
        sprintf(buf, "%d", i);
    } else if (i < 102400) {
        sprintf(buf, "%.2f KB", (float)i/1024);
    } else if (i < 1024*1024) {
        sprintf(buf, "%d KB", i/1024);
    } else {
        sprintf(buf, "%.2f MB", (float)i/1024/1024);
    }
    return(buf);
}

/**
 ** Pretty-print a number, with commas.
 **/
void
commaize(char *s)
{
    int i,j;
    int len = strlen(s);

    for (i = len-3 ; i > 0 ; i-=3) {
        for (j = strlen(s+i)+1 ; j > 0 ; j--) {
            s[i+j] = s[i+j-1];
        }
        s[i] = ',';
    }
}

Var *
pp_help(Var *s)
{
	char *p;

	if (s == NULL) p = NULL;
	else if (V_NAME(s)) p = V_NAME(s);
	else if (V_STRING(s)) p = V_STRING(s);

	do_help(p);
	return(NULL);
}

Var *
pp_shell(char *cmd)
{
	if (cmd[0] == '!') cmd++;
	system(cmd);
	return(NULL);
}

/**
 ** pp_set_where() - perform where substitution
 **/

Var *
pp_set_where(Var *id, Var *where, Var *exp) 
{
    Var *v, *e;
    int i,j,k, l;
    int size[3], s1,s2,s3,d1,d2,d3, od[3], os[3], d, s;
    Range *r, rout;
	int ival, dsize, format;
	double dval;

    /**
    ** If exp is named, it is a simple variable substitution.
    ** If its not named, we can use its memory directly.
    **/
    if (exp == NULL) return(NULL);

    /**
    ** this does the actual equivalence.
    ** If the rhs is a named value, duplicate it.
    ** otherwise, use the memory directly.
    **/

	if ((v = eval(exp)) == NULL) {
		parse_error("rhs is NULL.\n");
		return(NULL);
	}
	exp = v;


	if ((v = eval(id)) == NULL) {
		parse_error("lhs does not exist\n");
		return(NULL);
	}
	id = v;

	if ((v = eval(where)) == NULL) {
		parse_error("'where' does not exist\n");
		return(NULL);
	}
	where = v;

/*
	if (V_ORG(id) != V_ORG(where) ||
	    V_SIZE(id)[0] != V_SIZE(where)[0] ||
	    V_SIZE(id)[1] != V_SIZE(where)[1] ||
	    V_SIZE(id)[2] != V_SIZE(where)[2]) {

		parse_error("'where' value doesn't match org/shape of lhs\n");
		return(NULL);
	}
	if (V_ORG(id) != V_ORG(exp) ||
	    V_SIZE(id)[0] != V_SIZE(exp)[0] ||
	    V_SIZE(id)[1] != V_SIZE(exp)[1] ||
	    V_SIZE(id)[2] != V_SIZE(exp)[2]) {

		parse_error("rhs doesn't match org/shape of lhs of where\n");
		return(NULL);
	}
*/

	if (V_DSIZE(exp) != 1) {
		for (i = 0 ; i < 3 ; i++) {
			j = V_SIZE(id)[orders[V_ORG(id)][i]];
			k = V_SIZE(where)[orders[V_ORG(where)][i]];
			l = V_SIZE(exp)[orders[V_ORG(exp)][i]];
			if (j == 1) {
				if (k == 1 || l == 1) continue;
				if (k != l) {
					parse_error("Sizes don't match\n");
				}
			}
			if (k == 1) {
				if (j == 1 || l == 1) continue;
				if (j != l) {
					parse_error("Sizes don't match\n");
				}
			}
			if (l == 1) {
				if (j == 1 || k == 1) continue;
				if (j != k) {
					parse_error("Sizes don't match\n");
				}
			}
		}
	}


	if (V_DSIZE(exp) == 1) {
		dsize = V_DSIZE(id);
		format = V_FORMAT(id);
		ival  = extract_int(exp, 0);
		dval  = extract_double(exp, 0);

		for (i = 0 ; i < dsize ; i++) {
			if (extract_int(where, i)) {
				switch (format) {
					case BYTE:		((u_char *)V_DATA(id))[i] = ival; break;
					case SHORT:		((short *)V_DATA(id))[i] = ival; break;
					case INT:		((int *)V_DATA(id))[i] = ival; break;
					case FLOAT:		((float *)V_DATA(id))[i] = dval; break;
					case DOUBLE:	((double *)V_DATA(id))[i] = dval; break;
				}
			}
		}
	} else {
		dsize = V_DSIZE(id);
		format = V_FORMAT(id);

		for (i = 0 ; i < dsize ; i++) {
			j = rpos(i, id, where);
			if (extract_int(where, j)) {
				k = rpos(i, id, exp);
				switch (format) {
					case BYTE:		
						((u_char *)V_DATA(id))[i] = extract_int(exp, k); 
						break;
					case SHORT:		
						((short *)V_DATA(id))[i] = extract_int(exp, k); 
						break;
					case INT:		
						((int *)V_DATA(id))[i] = extract_int(exp, k); 
						break;
					case FLOAT:		
						((float *)V_DATA(id))[i] = extract_double(exp, k); 
						break;
					case DOUBLE:	
						((double *)V_DATA(id))[i] = extract_double(exp, k); 
						break;
				}
			}
		}

	}
    return(id);
}
