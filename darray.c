
#include "darray.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
** Darray_create(size)
**
** Create a Dynamic Array with an estimate of the number
** of elements needed.
**
** It is ok to pass 0 here, some default value will be used.
*/

Darray* Darray_create(int size)
{
	Darray* d;

	if (size <= 0) size = 16; /* a reasonable default size */

	d        = (Darray*)calloc(1, sizeof(Darray));
	d->size  = size;
	d->count = 0;
	d->data  = (void**)calloc(size, sizeof(void*));
	return (d);
}

/*
** Insert an element in a Darray
**
** Returns the element's index on success,
** -1 on error
*/
int Darray_insert(Darray* d, void* New, int pos)
{
	int i;
	if (d == NULL) return (-1);

	if (d->count >= d->size) {
		d->size *= 2;
		d->data = (void**)realloc(d->data, d->size * sizeof(void*));
	}
	if (pos == -1) pos = d->count;

	for (i = d->count; i > pos; i--) {
		d->data[i] = d->data[i - 1];
	}
	d->data[pos] = New;
	d->count++;

	return (pos);
}

/*
** Add an element to a Darray
**
** Returns the element's index on success,
** -1 on error
*/
int Darray_add(Darray* d, void* New)
{
	return (Darray_insert(d, New, -1));
}

/*
** Remove an element from Darray
*/
void* Darray_remove(Darray* d, int i)
{
	void* el = NULL;

	if (d == NULL || i >= d->count || i < 0) return NULL;

	el = d->data[i];
	memmove(&d->data[i], &d->data[i + 1], sizeof(char*) * (d->count - i - 1));

	d->count--;

	return el;
}

/*
** Get an element from a Darray.
**
** Returns 1 if the element was found,
**         0 if the element was not found,
**        -1 on error
*/
int Darray_get(const Darray* d, const int i, void** ret)
{
	if (d == NULL) return (-1);

	*ret = NULL;
	if (i < d->count) {
		*ret = d->data[i];
		return (1);
	}
	return (0);
}

/*
** Replace an element, and get previous value
**
** Returns 1 if the element was found,
**         0 if the element was not found,
**        -1 on error
*/
int Darray_replace(Darray* d, int i, void* in, void** out)
{
	if (d == NULL) return (-1);

	if (out != NULL) *out = NULL;
	if (i < d->count) {
		if (out != NULL) *out = d->data[i];
		d->data[i]            = in;
		return (1);
	}
	return (0);
}

/*
** Get the number of elements in the array
*/
int Darray_count(const Darray* d)
{
	if (d) return (d->count);
	return (-1);
}

void Darray_release(Darray* d, Darray_FuncPtr fptr)
{
	int i;

	if (fptr) {
		for (i = 0; i < d->count; i++) {
			if (d->data[i]) fptr(d->data[i]);
		}
	}
	d->count = 0;
}

void Darray_free(Darray* d, Darray_FuncPtr fptr)
{
	Darray_release(d, fptr);
	free(d->data);
	free(d);
}

/*
****************************************************************************
** Associative Array
****************************************************************************
*/

typedef struct _anode {
	int index;
	const char* key;
	void* value;
	avl_node_t node;
} Nnode;

int avl_compare(const avl_node_t *lhs, const avl_node_t *rhs, const void *aux)
{
	const Nnode* a = avl_ref(lhs, Nnode, node);
	const Nnode* b = avl_ref(rhs, Nnode, node);
	return strcmp(a->key, b->key);
}

Nnode* Nnode_create(const char* key, void* value)
{
	Nnode* a        = (Nnode*)calloc(1, sizeof(Nnode));
	if (key) a->key = strdup(key);
	a->value        = value;
	a->index        = -1; /* unassigned */
	return (a);
}

void Nnode_free(Nnode* a, Narray_FuncPtr fptr)
{
	if (a->key) free((void*)a->key);
	if (fptr && a->value) fptr(a->value);
	free(a);
}

Narray* Narray_create(int size)
{
	Narray* a;
	a       = (Narray*)calloc(1, sizeof(Narray));
	a->data = Darray_create(size);
	avl_init(&a->tree, NULL);
	return a;
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
int Narray_add(Narray* a, const char* key, void* data)
{
	char* r;
	Nnode* n;

	return (Narray_insert(a, key, data, -1));
}

/*
** Insert an element in the array.
**
** If the key already exists, abort and return an error
**
** If name is null, element is still added, but can only
** be accessed via index.
**
** Returns index (>= 0) on success
**        -1 on failure
*/
int Narray_insert(Narray* a, const char* key, void* data, int pos)
{
	char* r;
	Nnode* n;
	int i;
	avl_node_t* node;

	if (a == NULL) return -1;

	// See if this key already exists
	n = Nnode_create(key, data);

	if (key) {
		node = avl_insert(&a->tree, &n->node, avl_compare);
		if (node) {
			// Key already exists.  Abort.
			Nnode_free(n, NULL);
			return -1;
		}
	}

	// Add the node to the array, and update the indexes.
	n->index = Darray_insert(a->data, n, pos);
	pos      = n->index;

	for (i = pos + 1; i < a->data->count; i++) {
		n = (Nnode*)a->data->data[i];
		n->index++;
	}

	return (n->index);
}

/*
** Deletes the key from the Narray and returns the data
** associated with it.
*/
void* Narray_delete(Narray* a, const char* key)
{
	int i;
	Nnode n;
	Nnode* node;
	void* data;

	if (a == NULL) return NULL;

	memset(&n, 0, sizeof(n));
	n.key = key;

	avl_node_t* found = NULL;

	found = avl_search(&a->tree, &n.node, avl_compare);

	if (found) {
		Nnode* tmp = avl_ref(found, Nnode, node);
		// remove element from the linearly ordered array
		Darray_remove(a->data, tmp->index);

		// Re-index the nodes which have indices higher than
		// found->index.
		//
		// Don't know of a better way as yet!
		for (i = 0; i < a->data->count; i++) {
			node = (Nnode*)a->data->data[i];
			if (node->index > tmp->index) {
				node->index--;
			}
		}

		avl_remove(&a->tree, found);
		data = tmp->value;
		Nnode_free(tmp, NULL);
		return data;
	}

	return NULL;
}

/*
** Deletes the key from the Narray and returns the data
** associated with it, and renumbers.
*/
void* Narray_remove(Narray* a, int index)
{
	int i;
	Nnode n;
	Nnode* node;
	void* data;

	avl_node_t* found;

	if (a == NULL) return NULL;
	if (index > a->data->count) return NULL;

	node = Darray_remove(a->data, index);
	data = node->value;

	if (node->key != NULL) {
		memset(&n, 0, sizeof(n));
		n.key = node->key;
		found = avl_search(&a->tree, &n.node, avl_compare);

		if (found) {
			avl_remove(&a->tree, found);
			Nnode_free(avl_ref(found, Nnode, node), NULL);
		}
	}

	for (i = 0; i < a->data->count; i++) {
		node = (Nnode*)a->data->data[i];
		if (node->index > index) {
			node->index--;
		}
	}

	return data;
}
/*
** Return the array index of an element if it exists.
** otherwise, returns -1.
*/
int Narray_find(Narray* a, const char* key, void** data)
{
	Nnode n;

	avl_node_t* found;
	n.key = key;

	if (a == NULL) return -1;

	found = avl_search(&a->tree, &n.node, avl_compare);
	if (found) {
		Nnode* tmp = avl_ref(found, Nnode, node);
		if (data) *data = tmp->value;
		return tmp->index;
	}
	return -1;
}

/*
**  Narray_replace_data
**
**  Replace just the data at index i, returning the old data
*/
int Narray_replace(Narray* a, int i, void* New, void** old)
{
	Nnode* n;
	if (Darray_get(a->data, i, (void**)&n) == 1) {
		*old     = n->value;
		n->value = New;
		return 1;
	}
	return -1;
}

/*
** Narray_get() - Get values of element at index.
*/
int Narray_get(const Narray* a, const int i, char** key, void** data)
{
	Nnode* n;
	if (Darray_get(a->data, i, (void**)&n) == 1) {
		if (key) *key   = (char*)n->key;
		if (data) *data = n->value;
		return (1);
	}
	return (-1);
}

int Narray_count(const Narray* a)
{
	if (a) return (Darray_count(a->data));
	return (-1);
}

void Narray_free(Narray* a, Narray_FuncPtr fptr)
{
	int i;
	int count = Darray_count(a->data);
	Nnode* n;

	for (i = 0; i < count; i++) {
		Darray_get(a->data, i, (void**)&n);
		Nnode_free(n, fptr);
	}
	Darray_free(a->data, NULL);
	free(a);
}


#ifdef TEST
int Fail(char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(-1);
}

void test_Narray()
{
	int i;
	Narray* a;
	int r_i;
	void* r_v;
	int d[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	char* k[] = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"};
	char* key;
	int* data;

	a = Narray_create(0);

	r_i = Narray_find(a, "one", NULL);
	if (r_i != -1) Fail("find on nonexistant.\n");

	for (i = 0; i < 10; i++) {
		r_i = Narray_add(a, k[i], &d[i]);
		if (r_i < 0) Fail("addition of element: %d\n", i);
	}

	for (i = 0; i < 10; i++) {
		r_i = Narray_add(a, k[0], &d[0]);
		if (r_i > 0) Fail("addition of existing element: %d\n", i);
	}

	for (i = 0; i < 10; i++) {
		r_i = Narray_add(a, NULL, &d[i]);
	}

	for (i = 9; i >= 0; i--) {
		r_i = Narray_find(a, k[i], (void*)&data);
		printf("Narray[%d] (%s) = %d\n", r_i, k[i], *data);
	}

	printf("\n");

	for (i = 0; i < Narray_count(a); i++) {
		Narray_get(a, i, &key, (void*)&data);
		printf("Narray[%d] (%s) = %d\n", i, (key == NULL ? "(null)" : key), *data);
	}

	Narray_free(a, NULL);
}

int main()
{
	test_Narray();
	return 0;
}
#endif
