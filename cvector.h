#ifndef CVECTOR_H
#define CVECTOR_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif


/** Data structure for int vector. */
typedef struct cvector_i
{
	int* a;            /**< Array. */
	size_t size;       /**< Current size (amount you use when manipulating array directly). */
	size_t capacity;   /**< Allocated size of array; always >= size. */
} cvector_i;


extern size_t CVEC_I_START_SZ;


int cvec_i(cvector_i* vec, size_t size, size_t capacity);
int cvec_init_i(cvector_i* vec, int* vals, size_t num);

cvector_i* cvec_i_heap(size_t size, size_t capacity);
cvector_i* cvec_init_i_heap(int* vals, size_t num);
void cvec_i_copy(void* dest, void* src);

int cvec_push_i(cvector_i* vec, int a);
int cvec_pop_i(cvector_i* vec);

int cvec_extend_i(cvector_i* vec, size_t num);
int cvec_insert_i(cvector_i* vec, size_t i, int a);
int cvec_insert_array_i(cvector_i* vec, size_t i, int* a, size_t num);
int cvec_replace_i(cvector_i* vec, size_t i, int a);
void cvec_erase_i(cvector_i* vec, size_t start, size_t end);
int cvec_reserve_i(cvector_i* vec, size_t size);
int cvec_set_cap_i(cvector_i* vec, size_t size);
void cvec_set_val_sz_i(cvector_i* vec, int val);
void cvec_set_val_cap_i(cvector_i* vec, int val);

int* cvec_back_i(cvector_i* vec);

void cvec_clear_i(cvector_i* vec);
void cvec_free_i_heap(void* vec);
void cvec_free_i(void* vec);




/** Data structure for double vector. */
typedef struct cvector_d
{
	double* a;         /**< Array. */
	size_t size;       /**< Current size (amount you use when manipulating array directly). */
	size_t capacity;   /**< Allocated size of array; always >= size. */
} cvector_d;

extern size_t CVEC_D_START_SZ;


int cvec_d(cvector_d* vec, size_t size, size_t capacity);
int cvec_init_d(cvector_d* vec, double* vals, size_t num);

cvector_d* cvec_d_heap(size_t size, size_t capacity);
cvector_d* cvec_init_d_heap(double* vals, size_t num);

void cvec_d_copy(void* dest, void* src);


int cvec_push_d(cvector_d* vec, double a);
double cvec_pop_d(cvector_d* vec);

int cvec_extend_d(cvector_d* vec, size_t num);
int cvec_insert_d(cvector_d* vec, size_t i, double a);
int cvec_insert_array_d(cvector_d* vec, size_t i, double* a, size_t num);
double cvec_replace_d(cvector_d* vec, size_t i, double a);
void cvec_erase_d(cvector_d* vec, size_t start, size_t end);
int cvec_reserve_d(cvector_d* vec, size_t size);
int cvec_set_cap_d(cvector_d* vec, size_t size);
void cvec_set_val_sz_d(cvector_d* vec, double val);
void cvec_set_val_cap_d(cvector_d* vec, double val);

double* cvec_back_d(cvector_d* vec);

void cvec_clear_d(cvector_d* vec);
void cvec_free_d_heap(void* vec);
void cvec_free_d(void* vec);



/** Data structure for string vector. */
typedef struct cvector_str
{
	char** a;          /**< Array. */
	size_t size;       /**< Current size (amount you use when manipulating array directly). */
	size_t capacity;   /**< Allocated size of array; always >= size. */
} cvector_str;


extern size_t CVEC_STR_START_SZ;

char* mystrdup(const char* str);

int cvec_str(cvector_str* vec, size_t size, size_t capacity);
int cvec_init_str(cvector_str* vec, char** vals, size_t num);

cvector_str* cvec_str_heap(size_t size, size_t capacity);
cvector_str* cvec_init_str_heap(char** vals, size_t num);

void cvec_str_copy(void* dest, void* src);

int cvec_push_str(cvector_str* vec, char* a);
void cvec_pop_str(cvector_str* vec, char* ret);

int cvec_extend_str(cvector_str* vec, size_t num);
int cvec_insert_str(cvector_str* vec, size_t i, char* a);
int cvec_insert_array_str(cvector_str* vec, size_t i, char** a, size_t num);
void cvec_replace_str(cvector_str* vec, size_t i, char* a, char* ret);
void cvec_erase_str(cvector_str* vec, size_t start, size_t end);
int cvec_reserve_str(cvector_str* vec, size_t size);
int cvec_set_cap_str(cvector_str* vec, size_t size);
void cvec_set_val_sz_str(cvector_str* vec, char* val);
void cvec_set_val_cap_str(cvector_str* vec, char* val);

char** cvec_back_str(cvector_str* vec);

void cvec_clear_str(cvector_str* vec);
void cvec_free_str_heap(void* vec);
void cvec_free_str(void* vec);



typedef unsigned char byte;

/** Data structure for generic type (cast to void) vectors */
typedef struct cvector_void
{
	byte* a;                 /**< Array. */
	size_t size;             /**< Current size (amount you should use when manipulating array directly). */
	size_t capacity;         /**< Allocated size of array; always >= size. */
	size_t elem_size;        /**< Size in bytes of type stored (sizeof(T) where T is type). */
	void (*elem_init)(void*, void*);
	void (*elem_free)(void*);
} cvector_void;

extern size_t CVEC_VOID_START_SZ;

#define CVEC_GET_VOID(VEC, TYPE, I) ((TYPE*)&(VEC)->a[(I)*(VEC)->elem_size])

int cvec_void(cvector_void* vec, size_t size, size_t capacity, size_t elem_sz, void(*elem_free)(void*), void(*elem_init)(void*, void*));
int cvec_init_void(cvector_void* vec, void* vals, size_t num, size_t elem_sz, void(*elem_free)(void*), void(*elem_init)(void*, void*));

cvector_void* cvec_void_heap(size_t size, size_t capacity, size_t elem_sz, void (*elem_free)(void*), void(*elem_init)(void*, void*));
cvector_void* cvec_init_void_heap(void* vals, size_t num, size_t elem_sz, void (*elem_free)(void*), void(*elem_init)(void*, void*));

void cvec_void_copy(void* dest, void* src);

int cvec_push_void(cvector_void* vec, void* val);
void cvec_pop_void(cvector_void* vec, void* ret);
void* cvec_get_void(cvector_void* vec, size_t i);

int cvec_extend_void(cvector_void* vec, size_t num);
int cvec_insert_void(cvector_void* vec, size_t i, void* a);
int cvec_insert_array_void(cvector_void* vec, size_t i, void* a, size_t num);
void cvec_replace_void(cvector_void* vec, size_t i, void* a, void* ret);
void cvec_erase_void(cvector_void* vec, size_t start, size_t end);
int cvec_reserve_void(cvector_void* vec, size_t size);
int cvec_set_cap_void(cvector_void* vec, size_t size);
void cvec_set_val_sz_void(cvector_void* vec, void* val);
void cvec_set_val_cap_void(cvector_void* vec, void* val);

void* cvec_back_void(cvector_void* vec);

void cvec_clear_void(cvector_void* vec);
void cvec_free_void_heap(void* vec);
void cvec_free_void(void* vec);


#ifdef __cplusplus
}
#endif

/* header ends */
#endif

