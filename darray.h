#ifndef _DARRAY_H_
#define _DARRAY_H_

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

Darray *	Darray_create(int size);
int		Darray_add(Darray *d, void *New);
int		Darray_get(const Darray *d, const int i, void **ret);
int		Darray_replace(Darray *d, int i, void *in, void **out);
int		Darray_count(const Darray *d);
void		Darray_release(Darray *d, void (*fptr)(void *));
void		Darray_free(Darray *d, void (*fptr)(void *));

Narray *	Narray_create(int size);
int		Narray_add(Narray *a, char *key, void *data);
void *		Narray_delete(Narray *a, char *key);
int		Narray_find(Narray *a, char *key, void **data);
int		Narray_replace(Narray *a, int i, void *New, void **old);
int		Narray_get(const Narray *a, const int i,
			   char **key, void **data);
int		Narray_count(const Narray *a);
void		Narray_free(Narray *a, void (*fptr)(void *));

#endif /* _DARRAY_H_ */
