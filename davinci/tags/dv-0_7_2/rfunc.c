#include <stddef.h>
#include "parser.h"
#include "rfunc.h"

typedef struct _Arg_simple_axis {	/* simple axis argument structure */
    Var *obj;
    int axis;
} Arg_simple_axis;

char *axis_list[] = {		/* options list for axis */
    "x", "y", "z", NULL
};

Args Args_cat[] = {
    { "obj",  ID_VAL,    NULL,    offsetof(Arg_simple_axis, obj),  0 },
    { "axis", ID_ENUM, axis_list, offsetof(Arg_simple_axis, axis), 0 },
    NULL
};


Var *rf_cat(ArgsRegister *, Var *);

ArgsRegister regs[] = {
    { "zcat", rf_cat, Args_cat, sizeof(Arg_simple_axis), NULL }
};

/**
 ** Locate and dispatch must be separated, so you can tell if
 ** the function returned NULL, or if the function wasn't found.
 **/

ArgsRegister *
locate_rfunc(char *name) 
{
    ArgsRegister *r;

    for (r = regs ; r != NULL ; r++) {
        if (!strcasecmp(r->name, name)) {
            return(r);
        }
    }
    return(NULL);
}

Var *
dispatch_rfunc(ArgsRegister *r, Var *arg) 
{
	return ((*(r->func))(r, arg));
}

/**
 ** Parse things up.
 **/

void *
R_MakeArgs(ArgsRegister *r, Var *arg, u_char *out)
{
    int i,j,k,count;
    Var *s, *v, *e, *next;
    char *ptr;
    int len;
    Args *alist = r->args;
    char *fname = r->name;

    while (alist->name != NULL) {
        alist->filled = 0;
        alist++;
    }
    alist = r->args;

    while(arg != NULL) {
        v = arg;
        arg = v->next;
        v->next = NULL;
		
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
                return (NULL);
            }
            if (count > 1) {
                parse_error(NULL);
                return (NULL);
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
                return (NULL);
            }
        }

        
        /**
        ** putting av[i] into alist[j]
        **/

        if (alist[j].type == ID_STRING) {
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
                return(NULL);
            }
            v = e;
            if (V_TYPE(v) != ID_STRING) {
                parse_error(
                    "Illegal argument to function %s(), expected STRING", 
                    fname);
                return(NULL);
            }
            memcpy(out+alist[j].offset, &(V_STRING(v)), sizeof(char *));
            alist[j].filled = 1;
        } else if (alist[j].type == ID_VAL) {
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
                return(NULL);
            }
            v = e;
            if (V_TYPE(v) != ID_VAL) {
                parse_error("Illegal argument %s(...%s=...), expected VAL", 
                            fname, alist[j].name);
                return(NULL);
            }
            memcpy(out+alist[j].offset, &v, sizeof(char *));
            alist[j].filled = 1;
        } else if (alist[j].type == INT) {
            int ival;
            if ((e = eval(v)) == NULL) {
                parse_error("%s: Variable not found: %s", fname, V_NAME(v));
                return(NULL);
            }
            v = e;
            if (V_TYPE(v) != ID_VAL || V_FORMAT(v) > INT) {
                parse_error("Illegal argument %s(...%s=...), expected INT", 
                            fname, alist[j].name);
                return(NULL);
            }
            ival = extract_int(v, 0);
            memcpy(out+alist[j].offset, &ival, sizeof(char *));
            alist[j].filled = 1;
        } else if (alist[j].type == ID_ENUM) {
            char **p, *q = NULL, *ptr;
            char **values = alist[j].options;

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
                return(NULL);
            }

            memcpy(out+alist[j].offset, &q, sizeof(char *));
            alist[j].filled = 1;
        }
    }
    return(out);
}



/**
 ** The problem here is that the user has no way of
 **
 ** 1) providing default values
 ** 2) telling what args were passed
 **
 ** We can allow the user to pass a structure to be overwritten,
 ** thus allowing default values, but I don't know what to do about
 ** finding filled values
 **/
Var *rf_cat(ArgsRegister *r, Var *vargs)
{
    Arg_simple_axis args = { 0 };
    R_MakeArgs(r, vargs, (u_char *)&args); 

    printf("%d %s\n", V_INT(args.obj), args.axis ? args.axis : "(null)");
    return(NULL);
}
