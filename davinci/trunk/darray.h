#include "avl.h"

typedef struct _Darray {
    int size;
    int count;
    void **data;
} Darray;

typedef struct _Narray {
    Darray *data;
    avl_tree *tree;
} Narray;

Darray * Darray_create(int size);
int Darray_add(Darray *d, void *new);
int Darray_get(Darray *d, int i, void **ret);
int Darray_replace(Darray *d, int i, void *in, void **out);
int Darray_count(Darray *d);
void Darray_free(Darray *d, void (*fptr)()) ;
Narray * Narray_create(int size);
int Narray_add(Narray *a, char *key, void *data);
int Narray_find(Narray *a, char *key, void **data);
int Narray_replace(Narray *a, int i, void *new, void **old);
int Narray_get(Narray *a, int i, char **key, void **data);
int Narray_count(Narray *a);
void Narray_free(Narray *a, void (*fptr)());
