/**
 ** Dynamic Array - self-resizing array
 **
 ** Darray * Darray_create( int size )
 **
 ** 	Create an array with an initial estimated size
 **
 ** int Darray_add( array, int item )
 **
 ** 	Append item to array.  Returns item's index or -1 on error
 **
 ** int Darray_get( array, int index, void **item )
 **
 ** 	Get item at index from array.  Returns 1 on success.
 **
 ** int Darray_replace( array, int index, void *new, void **old)
 **
 ** 	Replace item at index with new value.  Retrieves old value.
 ** 	Returns 1 on success.
 **/

/**
 ** Associative Array
 **
 ** An array whose elements can be indexed by integer or by an associated
 ** key (string).
 **
 **
 ** Aarray * Aarray_create(int size)
 **
 ** 	Create an associative array with an estimated size.
 **     It is ok to pass 0 for size.
 **
 ** int Aarray_add(Aarray *a, char *key, void *data)
 **
 ** 	Append data to the end of the array and add key to the lookup table.
 ** 	Returns the element's index in the array on success or -1 on error.
 **     Will fail if key already exists.
 **
 ** int Aarray_find(Aarray *a, char *key, void **data)
 **
 ** 	Find the element with associated key.  Get its data and
 ** 	return its index on success.  Ok to pass a NULL for data.
 **     Fails if key doesn't exist.  Returns -1 on error.
 **
 ** int Aarray_get(Aarray *a, int index, char **key, void **data)
 **
 ** 	Get the key and data of the element at index.
 **     Ok to pass either key or data as NULL.
 ** 	Returns 1 on success, -1 on error.
 **
 ** int Aarray_count(Aarray *a)
 **
 ** 	Returns number of elements in array, or -1 on error.
 **/

#include <stdio.h>
#include <stdarg.h>

/*
** Darray_create(size)
**
** Create a Dynamic Array with an estimate of the number 
** of elements needed.
**
** It is ok to pass 0 here, some default value will be used.
*/

Darray *
Darray_create(int size)
{
    Darray *d;

    if (size <= 0) size = 16;		/* a reasonable default size */

    d = (Darray *)calloc(1, sizeof(Darray));
    d->size = size;
    d->count = 0;
    d->data = (void **)calloc(size, sizeof(void *));
    return(d);
}

/*
** Add an element to a Darray
**
** Returns the element's index on success,
** -1 on error
*/
int 
Darray_add(Darray *d, void *new)
{
    if (d == NULL) return(-1);
    
    if (d->count >= d->size) {
        d->size *= 2;
        d->data = (void **)realloc(d->data, d->size * sizeof(void *));
    }
    d->data[d->count++] = new;

    return(d->count-1);
}

/*
** Get an element from a Darray.
**
** Returns 1 if the element was found,
**         0 if the element was not found,
**        -1 on error
*/
int
Darray_get(Darray *d, int i, void **ret)
{
    if (d == NULL) return(-1);
        
    *ret = NULL;
    if (i < d->count) {
        *ret = d->data[i];
        return(1);
    }
    return(0);
}

/*
** Replace an element, and get previous value
**
** Returns 1 if the element was found,
**         0 if the element was not found,
**        -1 on error
*/
int
Darray_replace(Darray *d, int i, void *in, void **out)
{
    if (d == NULL) return(-1);
        
    *out = NULL;
    if (i < d->count) {
        *out = d->data[i];
        d->data[i] = in;
        return(1);
    }
    return(0);
}

/*
** Get the number of elements in the array
*/
int Darray_count(Darray *d)
{
    if (d) return(d->count);
    return(-1);
}

/*
****************************************************************************
** Associative Array
****************************************************************************
*/

typedef struct _anode {
    int index;
    char *key;
    void *value;
} Anode;


int Acmp(const void *a, const void *b, void *param)
{
    return(strcmp(((Anode *)a)->key, ((Anode *)b)->key));
}

Anode *
Anode_create(char *key, void *value)
{
    Anode *a = (Anode *)calloc(1, sizeof(Anode));
    a->key = key;
    a->value = value;
    a->index = -1;      /* unassigned */
    return(a);
}


Aarray *
Aarray_create(int size)
{
    Aarray *a;
    a = (Aarray *)calloc(1, sizeof(Aarray));
    a->data = Darray_create(size);
    a->tree = avl_create(Acmp, NULL);
    return(a);
}

/*
** Add the named element to the array.
**
** If the key already exists, abort and return an error
**
** If name is null, element is still added, but can only
** be accessed via index.
**
** Returns index (>= 0) on success
**        -1 on failure
*/
int
Aarray_add(Aarray *a, char *key, void *data)
{
    char *r;
    Anode *n;

    if (a == NULL) return(-1);
    /*
    ** See if this key already exists
    */
    n = Anode_create(key, data);
    
    if (key) {
        r = avl_insert(a->tree, n);
        if (r != NULL) {
            /*
            ** Key already exists.  Abort.
            */
            free(n);
            return(-1);
        }
    }
    /*
    ** Add the node to the array, and update the index.
    */
    n->index = Darray_add(a->data, n);
    return(n->index);
}

/*
** Return the array index of an element if it exists.
** otherwise, returns -1.
*/
int
Aarray_find(Aarray *a, char *key, char **data)
{
    Anode n;
    Anode *found;

    n.key = key;
    
    if (a == NULL) return(-1);
    
    found = avl_find(a->tree, &n);
    if (found) {
        if (data) *data = found->value;
        return(found->index);
    }
    return(-1);
}

/*
** Aarray_get() - Get values of element at index.
*/
int
Aarray_get(Aarray *a, int i, char **key, void **data)
{
    Anode *n;
    if (Darray_get(a->data, i, (void **)&n) == 1) {
        if (key) *key = n->key;
        if (data) *data = n->value;
        return(1);
    }
    return(-1);
}

int 
Aarray_count(Aarray *a)
{
    if (a) return(Darray_count(a->data));
    return(-1);
}

#ifdef TEST
int Fail(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (void) vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(-1);
}

test_Aarray()
{
    int i;
    Aarray *a;
    int r_i;
    void *r_v;
    int d[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    char *k[] = { "one", "two", "three", "four",
                  "five", "six", "seven", "eight",
                  "nine", "ten" };
    char *key;
    int *data;

    a = Aarray_create(0);

    r_i = Aarray_find(a, "one", NULL);
    if (r_i != -1) Fail("find on nonexistant.\n");

    for (i = 0 ; i < 10 ; i++) {
        r_i = Aarray_add(a, k[i], &d[i]);
        if (r_i < 0) Fail("addition of element: %d\n", i);
    }

    for (i = 0 ; i < 10 ; i++) {
        r_i = Aarray_add(a, k[0], &d[0]);
        if (r_i > 0) Fail("addition of existing element: %d\n", i);
    }

    for (i = 0 ; i < 10 ; i++) {
        r_i = Aarray_add(a, NULL, &d[i]);
    }    

    for (i = 9 ; i >= 0 ; i--) {
        r_i = Aarray_find(a, k[i], (void *)&data);
        printf("Aarray[%d] (%s) = %d\n", r_i, k[i], *data);
    }

    printf("\n");

    for (i = 0 ; i < Aarray_count(a) ; i++) {
        Aarray_get(a, i, &key, (void *)&data);
        printf("Aarray[%d] (%s) = %d\n",
               i,
               (key == NULL ?  "(null)" : key),
               *data);
    }

    
}

main() { test_Aarray(); }
#endif
