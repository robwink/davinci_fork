#include "parser.h"

/*
char *write_options[] = {
    "vicar", "grd", "pgm", "ppm", "ascii", "ers", "imath", "isis", "specpr"
};


Var *object;
char *filename;
char *type;
char *title;
int force;

Alist alist[] = {
    { "object",    ID_VAL,    NULL,             &object },
    { "filename",  ID_STRING, NULL,             &filename },
    { "type",      ID_ENUM,   write_options,    &type },
    { "title",     ID_STRING, NULL,             &title },
    { "force",     ID_VAL,    NULL,             &force },
    NULL
};
*/

/**
 ** verify that the arguments are of the expected type
 **/

 /**
    How do we get av[0] installed?
  **/

int 
parse_args(vfuncptr name, Var *args, Alist *alist)
{
    int i,j,k,count;
    Var *v, *e;
    char *fname;
    char *ptr;
    int len;

    int ac;
    Var **av;

    make_args(&ac, &av, name, args);

    fname = (char *)av[0];

    for (i = 1 ; i < ac ; i++) {
        v = av[i];
        if (v == NULL) continue;
        if (V_TYPE(v) == ID_KEYWORD && V_NAME(v) != NULL) {
			ptr = V_NAME(v);
			len = strlen(ptr);
			/**
			 ** set up error_buf, in case more than one keyword matches
			 **/
			sprintf(error_buf,
					"Non-unique keyword match: %s(...%s=...)\n"
					"Possible matches:\n",
					fname, ptr);

			for (count = k = 0 ; alist[k].name != NULL ; k++) {
				if (!strncasecmp(alist[k].name, ptr, len)) {
					/**
					 ** Append to error buf.  Again, just in case.
					 **/
					sprintf(error_buf + strlen(error_buf),
							"\t%s\n", alist[k].name);
					count++;
					j = k;
				}
			}
			if (count == 0) {
				parse_error("Unknown keyword to function: %s(... %s= ...)",
							fname, ptr);
				free(av);
				return(0);
			}
			if (count > 1) {
				parse_error(NULL);
				free(av);
				return(0);
			}
			/**
			** Get just the argument part
			**/
			v = V_KEYVAL(v);
        } else {
			/* special case created by create_args  */
			if (V_TYPE(v) == ID_KEYWORD) {
				v = V_KEYVAL(v);
			}
            for (j = 0 ; alist[j].name != NULL ; j++) {
                if (alist[j].filled == 0) break;
            }
            if (alist[j].name == NULL) {
                parse_error("Too many arguments to function: %s()", fname);
				free(av);
                return(0);
            }
        }

        
        /**
        ** putting av[i] into alist[j]
        **/

        if (alist[j].type == ID_STRING) {
            char **p;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
                return(0);
            }
            v = e;
            if (V_TYPE(v) != ID_STRING) {
                parse_error(
                    "Illegal argument to function %s(), expected STRING", 
                    fname);
				free(av);
                return(0);
            }
            p = (char **)(alist[j].value);
            *p = V_STRING(v);
            alist[j].filled = 1;
        } else if (alist[j].type == ID_TEXT) {
			/*
			** This works just like string, but should also allow the
			** user to pass just a string, which can get promoted.
			*/
            Var **vptr;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
                return(0);
            }
            v = e;
            if (V_TYPE(v) != ID_TEXT && V_TYPE(v) != ID_STRING) {
                parse_error(
                    "Illegal argument to function %s(), expected STRING",
                    fname);
				free(av);
                return(0);
            }

			if (V_TYPE(v) == ID_STRING) {
				char **s = (char **)calloc(1, sizeof(char *));
				s[0] = strdup(V_STRING(v));
				v = newText(1, s);
			}
			vptr = (Var **)(alist[j].value);
			*vptr = v;
            alist[j].filled = 1;
        } else if (alist[j].type == ID_VAL) {
            Var **vptr;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
                return(0);
            }
            v = e;
            if (V_TYPE(v) != ID_VAL) {
                parse_error("Illegal argument %s(...%s=...), expected VAL", 
                            fname, alist[j].name);
				free(av);
                return(0);
            }
            vptr = (Var **)(alist[j].value);
            *vptr = v;
            alist[j].filled = 1;
        } else if (alist[j].type == ID_STRUCT) {
            Var **vptr;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
                return(0);
            }
            v = e;
            if (V_TYPE(v) != ID_STRUCT) {
                parse_error("Illegal argument %s(...%s=...), expected STRUCT", 
                            fname, alist[j].name);
				free(av);
                return(0);
            }
            vptr = (Var **)(alist[j].value);
            *vptr = v;
            alist[j].filled = 1;
        } else if (alist[j].type == INT) {
            int *iptr;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
                return(0);
            }
            v = e;
            if (V_TYPE(v) != ID_VAL || V_FORMAT(v) > INT) {
                parse_error("Illegal argument %s(...%s=...), expected INT", 
                            fname, alist[j].name);
				free(av);
                return(0);
            }
            iptr = (int *)(alist[j].value);
            *iptr = extract_int(v, 0);
            alist[j].filled = 1;
        } else if (alist[j].type == FLOAT) {
            float *fptr;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
                return(0);
            }
            v = e;
            if (V_TYPE(v) != ID_VAL) {
                parse_error("Illegal argument %s(...%s=...), expected FLOAT", 
                            fname, alist[j].name);
				free(av);
                return(0);
            }
            fptr = (float *)(alist[j].value);
            *fptr = extract_float(v, 0);
            alist[j].filled = 1;
        } else if (alist[j].type == DOUBLE) {
            double *fptr;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
                return(0);
            }
            v = e;
            if (V_TYPE(v) != ID_VAL) {
                parse_error("Illegal argument %s(...%s=...), expected DOUBLE", 
                            fname, alist[j].name);
				free(av);
                return(0);
            }
            fptr = (double *)(alist[j].value);
            *fptr = extract_double(v, 0);
            alist[j].filled = 1;
        } else if (alist[j].type == ID_ENUM) {
            char **p, *q = NULL, *ptr;
            char **values = (char **)alist[j].limits;

            if (V_TYPE(v) == ID_STRING)
                ptr = V_STRING(v);
            else
                ptr = V_NAME(v);

			/*
			** This is in case we don't know what we're looking for
			*/

			if (values == NULL) {
				if (ptr != NULL) {
					alist[j].filled = 1;
					p = (char **)(alist[j].value);
					*p = ptr;
				}
			} else {
				/**
				** try once with, hopefully, a passed string
				**/
				for (p = values; p && *p; p++) {
					if (ptr && !strcasecmp(ptr, *p)) {
						q = *p;
					}
				}
				if (q == NULL) {
					if ((e = eval(v)) != NULL) {
						if (V_TYPE(e) == ID_STRING) {
							ptr = V_STRING(e);
							for (p = values; p && *p; p++) {
								if (ptr && !strcasecmp(ptr, *p)) {
									q = *p;
								}
							}
						}
					}
				}
				if (q == NULL) {
					parse_error("Illegal argument to function %s(...%s...)", 
								fname, alist[j].name);
					free(av);
					return(0);
				}

				p = (char **)(alist[j].value);
				*p = q;
				alist[j].filled = 1;
			}
        } else if (alist[j].type == ID_UNK) {
            Var **vptr;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
                return(0);
            }
            v = e;
            vptr = (Var **)(alist[j].value);
            *vptr = v;
            alist[j].filled = 1;
        } else if (alist[j].type == ONE_AXIS) {
            int _t;
            /*
            ** Shorthand for enumerated axis.  One of "x", "y" or "z"
            */

            char **p, *q = NULL, *ptr;
            char *values[] = { "x", "y", "z" };

            if (V_TYPE(v) == ID_STRING) 
                ptr = V_STRING(v);
            else 
                ptr = V_NAME(v);

            for (_t = 0 ; _t < 2 ; _t++) {
                if (ptr) {
                    for (p = values; p && *p; p++) {
                        if (ptr && !strcasecmp(ptr, *p)) {
                            q = *p;
                            break;
                        }
                    }
                }
                if ((e = eval(v)) == NULL || (V_TYPE(e) != ID_STRING)) {
                    break;
                }
                ptr = V_STRING(e);
            }

            if (q == NULL) {
                parse_error("Illegal argument to function %s(...%s...)", 
                            fname, alist[j].name);
				free(av);
                return(0);
            }

            p = (char **)(alist[j].value);
            *p = q;
            alist[j].filled = 1;
        } else if (alist[j].type == ANY_AXIS) {

        } else {
            fprintf(stderr, "parse_args: Bad programmer, no biscuit.\n");
        }
    }
    
    free(av);
    return(ac);
}

int
make_args(int *ac, Var ***av, vfuncptr func, Var *args)
{
    int count = 0, i = 0;
    Var *v, *next;

    for (v = args ; v != NULL ; v=v->next)
        count++;

    *av = (Var **)calloc(count+2, sizeof(Var *));

    if (func) (*av)[i++] = (Var *)func->name;
    for (v = args ; v != NULL ; v = next) {
        (*av)[i++] = v;
        next = v->next;
        v->next = NULL;
    }
    *ac = i;
    return 0;
}

Alist
make_alist(char *name, int type, void *limits, void *value)
{
    Alist a;
    a.name = name;
    a.type = type;
    a.limits = limits;
    a.value = value;
    a.filled = 0;
    return(a);
}

#include <stdarg.h>

Var *
create_args(int ac, ...)
{
	Var *args, *tail = NULL;
	va_list ap;

	char *key;
	Var *val, *v;

	va_start(ap, ac);
	while(1) {
		key = va_arg(ap, char *);
		val = va_arg(ap, Var *);
		if (val == NULL) {
			va_end(ap);
			break;
		}
		v = newVar();
		V_NAME(v) = (key ? strdup(key) : NULL);

		if (tail == NULL) {
			args = tail = pp_keyword_to_arg(v, val);
		} else {
			tail->next = pp_keyword_to_arg(v, val);
			tail = tail->next;
		}
	}
	return(args);
}
