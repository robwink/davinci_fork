#include "parser.h"
#include "func.h"
#include <sys/stat.h>
#ifndef __MSDOS__
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
    Var *obj = NULL, *compress = NULL, *normalize = NULL;
    int type = 0;
    int x,y,z, n, i, j, dsize, low, high, v;
    int *data;
    char *ptr = NULL;

    int ac;
    Var **av;

    make_args(&ac, &av, func, arg);
    return(make_struct(ac-1, av+1));
}

/*
*/
void
add_struct(Var *s, char *name, Var *exp)
{
    int i;
    Var *old;

    if (Narray_add(V_STRUCT(s), name, exp) == -1) {
        /*
        ** Oops, element already existed
        */
        if ((i = Narray_find(V_STRUCT(s), name, NULL)) != -1) {
            Narray_replace(V_STRUCT(s), i, exp, (void **)&old);
            if (old) free_var(old);
        }
    }
}


Var *
LoadVanilla(char *filename)
{
#ifdef __MSDOS__
    extern unsigned char *mmap(void *, size_t , int, int , int , size_t );
    extern void munmap(unsigned char *, int);
    typedef	void*	caddr_t;
#endif
 
    int fd;
    struct stat sbuf;
    int rows;
    int cols;
    int len;
    int i, j, k;
    int state;
    char *buf, *p, **data, c;
    int *type;
    char **names;
    Var *o, *v;

    rows = cols = 0;
    
    if (stat(filename, &sbuf) != 0) {
        parse_error("Unable to open file: %s\n", filename);
        return(NULL);
    }
    if ((len = sbuf.st_size) == 0) return(NULL);

    if ((fd= open(filename, O_RDONLY)) < 0) {
        parse_error("Unable to open file: %s", filename);
        return(NULL);
    }
    buf =  mmap(NULL, len+1, (PROT_READ | PROT_WRITE), MAP_PRIVATE, fd, 0);
    close(fd);

    /*
    ** Remove trailing spaces and newlines
    */
    for (i = len - 1; i > 0 ; i--) {
        if (isspace(buf[i])) {
            buf[i] = ' ';
            continue;
        }
        break;
    }
    if (i == 0) return(NULL);			/* empty file? */

    /*
    ** How many rows?
    */
    rows = 1;
    for (i = 0 ; i < len ; i++) {
        if (buf[i] == '\n') rows++;
    }

    /*
    ** If we wanted to be nice to the user, we could allow them
    ** to create an empty structure with one line of header, no data.
    */
    if (rows == 1) {
        parse_error("No data in file: %s\n", filename);
        munmap(buf, len);
        return(NULL);
    }

    /*
    ** How many columns?
    */
    p = buf;
    while (*p != '\n') {
        while (*p == ' ' || *p == '\t') p++;
        while (!isspace(*p)) p++;
        cols++;
        while (*p == ' ' || *p == '\t') p++;
    }


    type = (int *)calloc(cols, sizeof(int));
    data = (char **)calloc(rows*cols, sizeof(char *));
    names = (char **)calloc(cols, sizeof(char *));
    names[0] = strndup(buf, p-buf+1);

    i = 0;
    for (p = strtok(names[0], " \t\n") ; p && *p ; p = strtok(NULL, " \t\n")) {
        names[i++] = p;
    }

    /*
    ** Find each column and verify the format of each column
    */

    i = j = 0;
    state = 0;
    while (i < len) {
        c = buf[i];
        if (state == 0) {		/* reading spaces */
            if (!isspace(c)) {
                if (j == rows*cols) {
                    parse_error("Too many values in file");
                    break;
                }
                data[j++] = buf+i;
                state = 1;
            }
        } else {                        /* reading !spaces */
            if (isspace(c)) {
                buf[i] = '\0';
                state = 0;
            }
        }

        /*
        ** Update column type
        */
        if (j-1 > cols) {
            k = (j-1) % cols;
            if (isdigit(c) || c == '+' || c == '-')  type[k] |= 1;
            else if (strchr("Ee.", c))               type[k] |= 2;
            else if (!isspace(c))                    type[k] |= 4;
        }

        /*
        ** verify we got enough columns per row
        */
        if (c == '\n' && (j % cols) != 0) {
            parse_error("Ragged row in file: %s row %d\n", filename, j / cols);
            break;
        }
        i++;
    }

    /* error condition */
    if (i != len || j != rows*cols) {
        fprintf(stderr, "Error condition\n");
        munmap(buf, len);
        free(data);
        return(NULL);
    }


    o = new_struct(cols);

    /*
    ** Ok, we have each column in text.  Create the Var and convert the data
    */
    for (i = 0 ; i < cols ; i++) {
        if (type[i] & 4) {			/* string */
            char **out = (char **)calloc(rows, sizeof(char *));
            char *zero;
            for (j = 0 ; j < rows ; j++)  {
                out[i] = strdup(data[(j*cols)+i]);
            }
            /* v = newString(1, cols, 1, out); */
            zero = (char *)calloc(1,1);
            v = newVal(BSQ, 1,1,1, BYTE, zero);
        } else if (type[i] & 2) {	/* float */
            float *out = (float *)calloc(rows, sizeof(float));
            for (j = 0 ; j < rows ; j++)  {
                out[i] = atof(data[(j*cols)+i]);
            }
            v = newVal(BSQ, 1, rows, 1, FLOAT, out);
        } else if (type[i] & 1) {	/* int */
            int *out = (int *)calloc(rows, sizeof(float));
            for (j = 0 ; j < rows ; j++) {
                out[i] = atoi(data[(j*cols)+i]);
            }
            v = newVal(BSQ, 1, rows, 1, INT, out);
        }
        add_struct(o, names[i], v);
    }

    return(o);
}

Var *
ff_add_struct(vfuncptr func, Var * arg)
{
    Var *a = NULL, b, *v = NULL, *e;
    char *name = NULL;

    int ac;
    Var **av;
    Alist alist[4];
    alist[0] = make_alist( "object",    ID_STRUCT,    NULL,     &a);
    alist[1] = make_alist( "name",      ID_STRING,     NULL,     &name);
    alist[2] = make_alist( "value",     ID_UNK,     NULL,     &v);
    alist[3].name = NULL;

    make_args(&ac, &av, func, arg);
    if (parse_args(ac, av, alist)) return(NULL);

    if (a == NULL) {
        parse_error("Object is null");
        return(NULL);
    }
	
    if (name == NULL && (v == NULL || (v != NULL && V_NAME(v) == NULL))) {
        parse_error("name is null");
        return(NULL);
    }

    V_TYPE(&b) = ID_UNK;
    if (name != NULL) {
        V_NAME(&b) = name;
    }  else if (v != NULL && V_NAME(v) != NULL) {
        V_NAME(&b) = (V_NAME(v) ? strdup(V_NAME(v)) : 0);
    }

    if (v == NULL) {
        v = newVal(BSQ, 1, 1, 1, BYTE, calloc(1,1));
    } else {
        e = eval(v);
        if (e == NULL) {
            parse_error("Unable to find variable: %s\n", V_NAME(v));
            return(NULL);
        }
        v = e;
    }

    return(pp_set_struct(a, &b, v));
}

Var *
ff_get_struct(vfuncptr func, Var * arg)
{
    Var *a = NULL, b, *v;
    char *name = NULL;

    int ac;
    Var **av;
    Alist alist[3];
    alist[0] = make_alist( "object",    ID_STRUCT,    NULL,     &a);
    alist[1] = make_alist( "name",      ID_STRING,     NULL,     &name);
    alist[2].name = NULL;

    make_args(&ac, &av, func, arg);
    if (parse_args(ac, av, alist)) return(NULL);

    if (a == NULL) {
        parse_error("Object is null");
        return(NULL);
    }
	
    if (name == NULL) {
        parse_error("name is null");
        return(NULL);
    }

    V_TYPE(&b) = ID_UNK;
    V_NAME(&b) = name;

    find_struct(a, &b, &v);
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
        /*
        ** single occurance, just return the Var
        */
        // s = V_DUP(V_STRUCT(v).data[r->lo[0]]);
        get_struct_element(v, r->lo[0], NULL, &s);
    } else {
        s = new_struct(size);
        for (i = 0 ; i < size ; i++) {
            j = r->lo[0] + i *r->step[0];
            get_struct_element(v, i, NULL, &data);
            Narray_replace(V_STRUCT(s), j, data, (void **)&old);
			if (old) free_var(old);
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
    Var *p, *q, *r, *s;
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
            parse_error("Unable to find variable: %s\n", V_NAME(p));
            free_var(s);
            return(NULL);
        }
        add_struct(s, name, V_DUP(r));
        p = q;
    }
    return(s);
}

int
find_struct(Var *a, Var *b, Var **data)
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

    if ((i = Narray_find(V_STRUCT(a), V_NAME(b), (void **)data)) != -1) {
        return(i);
    }
    return(-1);
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
        add_struct(r, name, data);
    }
    return(r);
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

get_struct_element(Var *v, int i, char **name, Var **data)
{
    Narray_get(V_STRUCT(v), i, name, (void **)data);
}
int
get_struct_count(Var *v) 
{
    return(Narray_count(V_STRUCT(v)));
}
