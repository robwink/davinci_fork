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
parse_args(int ac, Var **av, Alist *alist)
{
    int i,j,k,count;
    Var *s, *v, *e;
    char *fname = (char *)av[0];
    char *ptr;
    int len;

    for (i = 1 ; i < ac ; i++) {
        v = av[i];
        if (v == NULL) continue;
        if (V_TYPE(v) == ID_KEYWORD) {
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
                return (1);
            }
            if (count > 1) {
                parse_error(NULL);
                return (1);
            }
            /**
            ** Get just the argument part
            **/
            v = V_KEYVAL(v);
        } else {
            for (j = 0 ; alist[j].name != NULL ; j++) {
                if (alist[j].filled == 0) break;
            }
            if (alist[j].name == NULL) {
                parse_error("Too many arguments to function: %s()", fname);
                return (1);
            }
        }

        
        /**
        ** putting av[i] into alist[j]
        **/

        if (alist[j].type == ID_STRING) {
            char **p;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
                return(1);
            }
			v = e;
            if (V_TYPE(v) != ID_STRING) {
                parse_error(
                    "Illegal argument to function %s(), expected STRING", 
                    fname);
                return(1);
            }
            p = (alist[j].value);
            *p = V_STRING(v);
            alist[j].filled = 1;
        } else if (alist[j].type == ID_VAL) {
            Var **vptr;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
                return(1);
            }
            v = e;
            if (V_TYPE(v) != ID_VAL) {
                parse_error("Illegal argument %s(...%s=...), expected VAL", 
                            fname, alist[j].name);
                return(1);
            }
            vptr = (alist[j].value);
            *vptr = v;
            alist[j].filled = 1;
        } else if (alist[j].type == INT) {
            int *iptr;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
                return(1);
            }
            v = e;
            if (V_TYPE(v) != ID_VAL || V_FORMAT(v) > INT) {
                parse_error("Illegal argument %s(...%s=...), expected INT", 
                            fname, alist[j].name);
                return(1);
            }
            iptr = (alist[j].value);
            *iptr = extract_int(v, 0);
            alist[j].filled = 1;
        } else if (alist[j].type == ID_ENUM) {
            char **p, *q = NULL, *ptr;
            char **values = alist[j].limits;

            if (V_TYPE(v) == ID_STRING)
                ptr = V_STRING(v);
            else
                ptr = V_NAME(v);

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
                return(1);
            }

            p = (alist[j].value);
            *p = q;
            alist[j].filled = 1;
        }
    }
    return(0);
}

make_args(int *ac, Var ***av, vfuncptr func, Var *args)
{
    int count = 0, i = 0;
    Var *v, *next;

    for (v = args ; v != NULL ; v=v->next)
        count++;

    *av = calloc(count+2, sizeof(Var *));

    (*av)[i++] = (Var *)func->name;
    for (v = args ; v != NULL ; v = next) {
        (*av)[i++] = v;
        next = v->next;
        v->next = NULL;
    }
    *ac = i;
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
