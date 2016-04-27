/*
https://github.com/rswinkle/CVector
http://www.robertwinkler.com/projects/cvector/index.html

The MIT License (MIT)

Copyright (c) 2011-2016 Robert Winkler

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/

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
typedef struct vector_i
{
	int* a;            /**< Array. */
	size_t size;       /**< Current size (amount you use when manipulating array directly). */
	size_t capacity;   /**< Allocated size of array; always >= size. */
} vector_i;

extern size_t VEC_I_START_SZ;

int vec_i(vector_i* vec, size_t size, size_t capacity);
int init_vec_i(vector_i* vec, int* vals, size_t num);

vector_i* vec_i_heap(size_t size, size_t capacity);
vector_i* init_vec_i_heap(int* vals, size_t num);
void vec_i_copy(void* dest, void* src);

int push_i(vector_i* vec, int a);
int pop_i(vector_i* vec);

int extend_i(vector_i* vec, size_t num);
int insert_i(vector_i* vec, size_t i, int a);
int insert_array_i(vector_i* vec, size_t i, int* a, size_t num);
void erase_i(vector_i* vec, size_t start, size_t end);
int reserve_i(vector_i* vec, size_t size);
int set_capacity_i(vector_i* vec, size_t size);
void set_val_sz_i(vector_i* vec, int val);
void set_val_cap_i(vector_i* vec, int val);

int* back_i(vector_i* vec);

void clear_i(vector_i* vec);
void free_vec_i_heap(void* vec);
void free_vec_i(void* vec);

/** Data structure for double vector. */
typedef struct vector_d
{
	double* a;         /**< Array. */
	size_t size;       /**< Current size (amount you use when manipulating array directly). */
	size_t capacity;   /**< Allocated size of array; always >= size. */
} vector_d;

extern size_t VEC_D_START_SZ;

int vec_d(vector_d* vec, size_t size, size_t capacity);
int init_vec_d(vector_d* vec, double* vals, size_t num);

vector_d* vec_d_heap(size_t size, size_t capacity);
vector_d* init_vec_d_heap(double* vals, size_t num);

void vec_d_copy(void* dest, void* src);

int push_d(vector_d* vec, double a);
double pop_d(vector_d* vec);

int extend_d(vector_d* vec, size_t num);
int insert_d(vector_d* vec, size_t i, double a);
int insert_array_d(vector_d* vec, size_t i, double* a, size_t num);
void erase_d(vector_d* vec, size_t start, size_t end);
int reserve_d(vector_d* vec, size_t size);
int set_capacity_d(vector_d* vec, size_t size);
void set_val_sz_d(vector_d* vec, double val);
void set_val_cap_d(vector_d* vec, double val);

double* back_d(vector_d* vec);

void clear_d(vector_d* vec);
void free_vec_d_heap(void* vec);
void free_vec_d(void* vec);

/** Data structure for string vector. */
typedef struct vector_str
{
	char** a;          /**< Array. */
	size_t size;       /**< Current size (amount you use when manipulating array directly). */
	size_t capacity;   /**< Allocated size of array; always >= size. */
} vector_str;

extern size_t VEC_STR_START_SZ;

char* mystrdup(const char* str);

int vec_str(vector_str* vec, size_t size, size_t capacity);
int init_vec_str(vector_str* vec, char** vals, size_t num);

vector_str* vec_str_heap(size_t size, size_t capacity);
vector_str* init_vec_str_heap(char** vals, size_t num);

void vec_str_copy(void* dest, void* src);

int push_str(vector_str* vec, char* a);
void pop_str(vector_str* vec, char* ret);

int extend_str(vector_str* vec, size_t num);
int insert_str(vector_str* vec, size_t i, char* a);
int insert_array_str(vector_str* vec, size_t i, char** , size_t num);
void erase_str(vector_str* vec, size_t start, size_t end);
int reserve_str(vector_str* vec, size_t size);
int set_capacity_str(vector_str* vec, size_t size);
void set_val_sz_str(vector_str* vec, char* val);
void set_val_cap_str(vector_str* vec, char* val);

char** back_str(vector_str* vec);

void clear_str(vector_str* vec);
void free_vec_str_heap(void* vec);
void free_vec_str(void* vec);

typedef unsigned char byte;

/** Data structure for generic type (cast to void) vectors */
typedef struct vector_void
{
	byte* a;                 /**< Array. */
	size_t size;             /**< Current size (amount you should use when manipulating array directly). */
	size_t capacity;         /**< Allocated size of array; always >= size. */
	size_t elem_size;        /**< Size in bytes of type stored (sizeof(T) where T is type). */
	void (*elem_init)(void*, void*);
	void (*elem_free)(void*);
} vector_void;

extern size_t VEC_VOID_START_SZ;

#define GET_VOID(VEC, TYPE, I) ((TYPE*)&(VEC)->a[(I)*(VEC)->elem_size])

int vec_void(vector_void* vec, size_t size, size_t capacity, size_t elem_sz, void(*elem_free)(void*), void(*elem_init)(void*, void*));
int init_vec_void(vector_void* vec, void* vals, size_t num, size_t elem_sz, void(*elem_free)(void*), void(*elem_init)(void*, void*));

vector_void* vec_void_heap(size_t size, size_t capacity, size_t elem_sz, void (*elem_free)(void*), void(*elem_init)(void*, void*));
vector_void* init_vec_void_heap(void* vals, size_t num, size_t elem_sz, void (*elem_free)(void*), void(*elem_init)(void*, void*));

void vec_void_copy(void* dest, void* src);

int push_void(vector_void* vec, void* val);
void pop_void(vector_void* vec, void* ret);
void* vec_void_get(vector_void* vec, size_t i);

int extend_void(vector_void* vec, size_t num);
int insert_void(vector_void* vec, size_t i, void* a);
int insert_array_void(vector_void* vec, size_t i, void* a, size_t num);
void erase_void(vector_void* vec, size_t start, size_t end);
int reserve_void(vector_void* vec, size_t size);
int set_capacity_void(vector_void* vec, size_t size);
void set_val_sz_void(vector_void* vec, void* val);
void set_val_cap_void(vector_void* vec, void* val);

void* back_void(vector_void* vec);

void clear_void(vector_void* vec);
void free_vec_void_heap(void* vec);
void free_vec_void(void* vec);

#ifdef __cplusplus
}
#endif

/* header ends */
#endif
