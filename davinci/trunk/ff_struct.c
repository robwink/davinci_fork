#include "parser.h"
#include "func.h"
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/mman.h>
#endif

/*
** new_struct() - Create an empty structure
*/

Var *
new_struct(int ac)
{
    Var *o = newVar();

    V_TYPE(o) = ID_STRUCT;
    V_STRUCT(o) = Narray_create(max(ac, 1));

    return(o);
}

/*
** Create a structure with the specified names
*/

Var *
make_struct(int ac, Var **av)
{
    Var *o;
    Var *data;
    int i;
    char *zero;

    o = new_struct(ac);

    for (i = 0 ; i < ac ; i++) {
        zero = (char *)calloc(1,1);
        data = newVal(BSQ, 1,1,1, BYTE, zero);
        mem_claim(data);
        add_struct(o, V_NAME(av[i]), data);
    }
    return(o);
}

/*
** ff_struct() - create a structure
*/

Var *
ff_struct(vfuncptr func, Var * arg)
{
	Var *v;
    int ac;
    Var **av;

    make_args(&ac, &av, func, arg);
	v = make_struct(ac-1, av+1);
	free(av);
    return(v);
}

/*
*/
void
add_struct(Var *s, char *name, Var *exp)
{
    int i;
    Var *old = NULL;

    if (Narray_add(V_STRUCT(s), name, exp) == -1) {
        /*
        ** Oops, element already existed
        */
        if ((i = Narray_find(V_STRUCT(s), name, NULL)) != -1) {
            Narray_replace(V_STRUCT(s), i, exp, (void **)&old);
            if (old) {
				/*
				** Gotta claim the memory before we free it
				*/
				mem_claim(old);
				free_var(old);
			}
        }
    }
	/*
	** This appears to be necessary to prevent structure elements from
	** being free'd later.
	*/
	mem_claim(exp);
}


/*
** add_struct()
**
**    (name=), append zero value with 'name' to end
**    (value=), append unnamed value to end
**    (name=,value=), append value as name
*/


Var *
ff_insert_struct(vfuncptr func, Var * arg)
{
    Var *a = NULL, b, *v = NULL, *e;
    char *name = NULL;
	Var *before = NULL, *after = NULL;
	int pos;

    int ac;
    Var **av;
    Alist alist[6];
    alist[0] = make_alist( "object",    ID_STRUCT,    NULL,     &a);
    alist[1] = make_alist( "value",     ID_UNK,     NULL,     &v);
    alist[2] = make_alist( "name",      ID_STRING,     NULL,     &name);
    alist[3] = make_alist( "before",    ID_UNK,     NULL,     &before);
    alist[4] = make_alist( "after",     ID_UNK,     NULL,     &after);
    alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (a == NULL) {
        parse_error("Object is null");
        return(NULL);
    }

	if (before && after) {
		parse_error("Only one of before= or after= can be specified");
		return(NULL);
	}
	
	/*
	** If the user doesn't pass a name, then try to take the name from the
	** incoming variable.
	*/
    if (name == NULL) {
		if (v != NULL) name = V_NAME(v);
    }

    if (v == NULL) {
        v = newVal(BSQ, 1, 1, 1, BYTE, calloc(1,1));
    } else {
        e = eval(v);
        if (e == NULL) {
            parse_error("Unable to find variable: %s", V_NAME(v));
            return(NULL);
        }
        v = e;
    }

    if (V_NAME(v) != NULL || mem_claim(v) == NULL) {
        /**
         ** if we can't claim the memory, we can't use it.
         **/
        v = V_DUP(v);
    }

	if (name != NULL && find_struct(a, name, NULL) != -1) {
		parse_error("Structure already contains element: %s", name);
		return(NULL);
	}

	if (before != NULL) {
		pos = struct_pos(a, before);
		if (pos >= 0) {
			insert_struct(a, pos, name, v);
		} else {
			parse_error("bad position");
			return(NULL);
		}
	} else if (after) {
		pos = struct_pos(a, after);
		if (pos >= 0) {
			insert_struct(a, pos+1, name, v);
		} else {
			parse_error("bad position");
			return(NULL);
		}
	} else {
		add_struct(a, name, v);
	}

    return(v);
}

/*
** Evaluate a structure 'position'.  Returns (int) index into struct
** This can be a named element, or it can be a numeric value.
** Numbers come from the user and are 1 based
*/

int
struct_pos(Var *a, Var *v)
{
	int pos = -1;
	Var *e;
	char *name;

	if (V_TYPE(v) == ID_STRING) {
		name = V_STRING(v);
		pos = find_struct(a, V_STRING(v), NULL);
		if (pos == -1) {
			parse_error("Structure does not contain element: %s", name);
		}
	} else {
		/* The user passed a number, it is one based */
		e = eval(v);
		if (e == NULL) {
            parse_error("Unable to find variable: %s", V_NAME(v));
            return(NULL);
		}
		v = e;
		if (V_TYPE(v) == ID_VAL) {
			pos = extract_int(v, 0);
			if (pos > 0 && pos <= get_struct_count(a)) {
				return(pos-1);
			} else {
				return(-1);
			}
		} else {
			parse_error("Not a value");
		}
	}
	return(pos);
}

insert_struct(Var *a, int pos, char *name, Var *data)
{
	Narray_insert(V_STRUCT(a), name, data, pos);
}

Var *
ff_get_struct(vfuncptr func, Var * arg)
{
    Var *a = NULL, b, *v;
    char *name = NULL;

    Alist alist[3];
    alist[0] = make_alist( "object",    ID_STRUCT,    NULL,     &a);
    alist[1] = make_alist( "name",      ID_STRING,     NULL,     &name);
    alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

    if (a == NULL) {
        parse_error("Object is null");
        return(NULL);
    }
	
    if (name == NULL) {
        parse_error("name is null");
        return(NULL);
    }

    find_struct(a, name, &v);
    return(v);
}

Var *
varray_subset(Var *v, Range *r)
{
    Var *s;
    int size;
    int i, j;
    char *name;
    Var **data, *old;

    size = 1 + (r->hi[0] - r->lo[0]) / r->step[0];

    if (size == 1) {
        get_struct_element(v, r->lo[0], NULL, &s);
    } else {
		/* this code is broken */
		/*
        s = new_struct(size);
        for (i = 0 ; i < size ; i++) {
            j = r->lo[0] + i *r->step[0];
            get_struct_element(v, j, NULL, &data);
            Narray_replace(V_STRUCT(s), i, data, (void **)&old);
            if (old) free_var(old);
        }
		*/
		s = new_struct(size);
        for (i = 0 ; i < size ; i++) {
            j = r->lo[0] + i *r->step[0];
            get_struct_element(v, j, &name, &data);
			data = V_DUP(data);
            Narray_add(V_STRUCT(s), name, data);
			mem_claim(data);
		}
    }
    return(s);
}

/*
** Set 1 to 1
** Set 1 to struct
** Set many to 1		( replication )
** Set many to many		( same size )
*/

Var *
set_varray(Var *v, Range *r, Var *e)
{
    int i;
    int count = 0;

    int size = 1 + (r->hi[0] - r->lo[0]) / r->step[0];

    Var *data;
    Var *old;

    /*
    ** The case of 1 <- N, just duplicate N and stuff it in here
    */
    if (size == 1) {
        i = r->lo[0];
        data = V_DUP(e);
        mem_claim(data);
        Narray_replace(V_STRUCT(v), i, data, (void **)&old);
        if (old) free_var(old);
    } else {
        if (V_TYPE(e) == ID_STRUCT) {
            if (size != get_struct_count(e)) {
                parse_error("Structure sizes don't match.");
                return(NULL);
            }
        }

        for (i = r->lo[0] ; i <= r->hi[0] ; i += r->step[0]) {
            if (V_TYPE(e) == ID_STRUCT) {
                get_struct_element(e, count++, NULL, &data);
                data = V_DUP(data);
            } else {
                data = V_DUP(e);
            }
            mem_claim(data);
            Narray_replace(V_STRUCT(v), i, data, (void **)&old);
            if (old) free_var(old);
        }
    }

    return(varray_subset(v, r));
}

Var *
create_struct(Var *v)
{
    Var *p, *q, *r, *s, *tmp;
    char *name;
    p = v;

    s = new_struct(0);

    while(p != NULL) {
        q = p->next;
        name = NULL;
        if (V_TYPE(p) == ID_KEYWORD) {
            name = V_NAME(p);
            p = V_KEYVAL(p);
        }
        r = eval(p);
        if (r == NULL) {
            parse_error("Unable to find variable: %s", V_NAME(p));
            free_var(s);
            return(NULL);
        }
		tmp = V_DUP(r);
		mem_claim(tmp);
        add_struct(s, name, tmp);
        p = q;
    }
    return(s);
}

/*
** Find the position (and data) of a named structure element.
** Returns: >= 0 if found,
**            -1 if not found
*/
int
find_struct(Var *a, char *b, Var **data)
{
    Var *s;
    int i;
    if (a == NULL || b == NULL) return(-1);

    if ((s = eval(a)) != NULL) {
        a = s;
    }

    if (V_TYPE(a) != ID_STRUCT) {
        if (V_NAME(a)) {
            parse_error("%s: Not a struct", V_NAME(a));
        } else {
            parse_error("element is not a struct");
        }
        return(-1);
    }

    return(Narray_find(V_STRUCT(a), b, (void **)data));
}

free_struct(Var *v)
{
    Narray_free(V_STRUCT(v), free_var);
}


compare_struct(Var *a, Var *b)
{
    int i;
    int count = get_struct_count(a);
    char *name_a, *name_b;
    Var *data_a, *data_b;

    if (get_struct_count(b) != count) return(0);
    
    for (i = 0 ; i < count ; i++) {
        get_struct_element(a, i, &name_a, &data_a);
        get_struct_element(b, i, &name_b, &data_b);
        
        if ((name_a && !name_b) || (name_b && !name_a)) return(0);
        if (name_a && name_b && strcmp(name_a, name_b)) return(0);

        if (compare_vars(data_a, data_b) == 0) return(0);
    }
    return(1);
}

void
get_struct_element(Var *v, int i, char **name, Var **data)
{
    Narray_get(V_STRUCT(v), i, name, (void **)data);
}
int
get_struct_count(Var *v) 
{
    return(Narray_count(V_STRUCT(v)));
}

int
get_struct_names(Var *v, char ***names, char *prefix) 
{
	Var *data;
	char *name;
	int len = (prefix != NULL ? strlen(prefix) : 0);
	int n = get_struct_count(v);
	char **p = calloc(n+1, sizeof(char **));
	int count = 0;
	int i;

	for (i = 0 ; i < n ; i++) {
		name = NULL;
		get_struct_element(v, i, &name, &data);
		if (name != NULL && (len == 0 || !strncmp(prefix, name, len))) {
			p[count++] = strdup(name);
		}
	}
	p[count++] = NULL;
	*names = p;
	return(count);
}

Var *
duplicate_struct(Var *v)
{
    int i;
    int count = get_struct_count(v);
    Var *r = new_struct(count);
    char *name;
    Var *data;

    for (i = 0 ; i < count ; i++) {
        get_struct_element(v, i, &name, &data);
		data = V_DUP(data);
		mem_claim(data);
        add_struct(r, name, data);
    }
    return(r);
}


/*
** This function makes totally new data, not just append.
**
** This looks like it will fail with unnamed elements
*/

Var *
concatenate_struct(Var *a, Var *b)
{
	int i;
	char *name;
	Var *c, *data;
	Var *aa, *bb;

	/* verify that both args are usable */
	if (V_TYPE(a) == NULL || V_TYPE(b) == NULL) return(NULL);

	/* verify that there's no name conflicts */
	for (i = 0 ; i < get_struct_count(b) ; i++) {
		get_struct_element(b, i, &name, NULL);
		if (find_struct(a, name, NULL) >= 0) {
			parse_error("Structures both contain element named: %s", name);
			return(NULL);
		}
	}
	/* ok to proceed */

	aa = duplicate_struct(a);

	for (i = 0 ; i < get_struct_count(b) ; i++) {
		get_struct_element(b, i, &name, &data);
		data = V_DUP(data);
		mem_claim(data);
		add_struct(aa, name, data);
	}
	return(aa);
}

/**************************************************************************
* ff_remove_struct() - Removes an element from a structure and returns it.
***************************************************************************/
Var *
ff_remove_struct(vfuncptr func, Var * arg)
{
	Var *v = NULL, *a= NULL, *s;
	char *name = NULL;

    Alist alist[3];
    alist[0] = make_alist( "structure",    ID_STRUCT,    NULL,     &a);
    alist[1] = make_alist( "name",         ID_STRING,     NULL,     &name);
    alist[2].name = NULL;
	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (a == NULL ) {
		parse_error("No structure specified.");
		return(NULL);
	}
	if (name == NULL) {
		parse_error("No name specified to remove from structure.");
		return(NULL);
	}

	if (find_struct(a, name, NULL) < 0) {
		parse_error("Structure does not contain element: %s", name);
		return(NULL);
	} else {
		s = Narray_delete(V_STRUCT(a), name);
		v = V_DUP(s);
		free_var(s);
	}
	return(v);
}
