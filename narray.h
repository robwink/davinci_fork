#ifndef _DARRAY_H_
#define _DARRAY_H_

#include "avl.h"
#include "cvector.h"


typedef struct _Narray {
	cvector_voidptr data;
	avl_tree_t tree;
} Narray;

typedef void (*Narray_FuncPtr)(void*);


Narray* Narray_create(int size);
int Narray_add(Narray* a, const char* key, void* data);
void* Narray_delete(Narray* a, const char* key);
void* Narray_remove(Narray* a, int index);
int Narray_find(Narray* a, const char* key, void** data);
int Narray_replace(Narray* a, int i, void* New, void** old);
int Narray_insert(Narray* a, const char* key, void* data, size_t pos);
int Narray_get(const Narray* a, const int i, char** key, void** data);
int Narray_count(const Narray* a);
void Narray_free(Narray* a, Narray_FuncPtr fptr);

#endif /* _DARRAY_H_ */
