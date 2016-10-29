
#include "narray.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
****************************************************************************
** Associative Array
****************************************************************************
*/

typedef struct _anode {
	int index;
	//const char* key;
	char* key;
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

Narray* Narray_create(int capacity)
{
	Narray* a;
	a       = (Narray*)calloc(1, sizeof(Narray));
	cvec_voidptr(&a->data, 0, capacity, NULL, NULL);
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
	return (Narray_insert(a, key, data, a->data.size));
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

	//this is stupid
	//if (pos == -1) pos = a->data.size;
	
	//if pos < 0 || > size bad things happen

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
	n->index = pos;
	cvec_insert_voidptr(&a->data, pos, (void**)&n);
	for (i = pos+1; i<a->data.size; ++i) {
		n = (Nnode*)a->data.a[i];
		n->index++;
	}

	return pos;
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
		cvec_erase_voidptr(&a->data, tmp->index, tmp->index);

		// Re-index the nodes which have indices higher than
		// found->index.
		//
		// Don't know of a better way as yet!
		// TODO
		for (i = 0; i < a->data.size; ++i) {
			node = (Nnode*)a->data.a[i];
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
** Deletes the index from the Narray and returns the data
** associated with it.
*/
void* Narray_remove(Narray* a, int index)
{
	int i;
	Nnode n;
	Nnode* node;
	void* data;

	avl_node_t* found;

	if (a == NULL) return NULL;
	if (index > a->data.size) return NULL;

	data = a->data.a[index];
	cvec_erase_voidptr(&a->data, index, index);

	if (node->key != NULL) {
		memset(&n, 0, sizeof(n));
		n.key = node->key;
		found = avl_search(&a->tree, &n.node, avl_compare);

		if (found) {
			avl_remove(&a->tree, found);
			Nnode_free(avl_ref(found, Nnode, node), NULL);
		}
	}

	for (i = 0; i < a->data.size; i++) {
		node = (Nnode*)a->data.a[i];
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
	if (i < a->data.size) {
		n = (Nnode*)a->data.a[i];
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
	if (i < a->data.size) {
		n = (Nnode*)a->data.a[i];
		if (key) *key   = n->key;
		if (data) *data = n->value;
		return 1;
	}
	return -1;
}

int Narray_count(const Narray* a)
{
	if (a) return a->data.size;
	return -1;
}

void Narray_free(Narray* a, Narray_FuncPtr fptr)
{
	int i;
	int count = a->data.size;

	for (i = 0; i < count; i++) {
		Nnode_free((Nnode*)a->data.a[i], fptr);
	}
	cvec_free_voidptr(&a->data);
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
