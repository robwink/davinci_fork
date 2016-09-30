#ifndef SCOPE_H
#define SCOPE_H

#include "cvector.h"


typedef struct varptr {
	Var* p;
} varptr;

typedef struct dict_item {
	char* name;
	Var* value;
} dict_item;


typedef struct Scope {

	// a[0].value holds the var representation of argc
	cvector_void dd;   // named variable data dictionary
	cvector_void args; // number arguments data dictionary



	// symbol table.  This actually holds the memory for values created
	// in this scope. Child scopes will point to these vars
	cvector_void symtab; // local symbol table

	Darray* tmp;         // tmp memory list

	cvector_void stack;  // local stack

	UFUNC* ufunc;        // function pointer

	Var* rval;           // value returned
	int broken;          // loop counter flag
	int loop;            // loop counter flag
	int returned;        // loop counter flag
} Scope;

/**
 ** When the scope is destroyed, everything in symtab (and of course stack)
 ** also gets destroyed.  dd and args don't, so don't put any memory in there.
 ** (besides duplicated names).
 **/

Var* dd_get_argv(Scope* s, int n);
void dd_put(Scope* s, char* name, Var* v);
void dd_unput_argv(Scope* s);
Var* dd_find(Scope*, char*);


void init_scope(Scope* s);
void init_scope_stack();

void scope_push(Scope*);
void scope_pop(void);
int scope_stack_count();
Scope* scope_stack_get(int i);
Scope* scope_stack_back();
Scope* scope_tos(void);
void free_scope(Scope*);

void cleanup(Scope*);

void push(Scope*, Var*);
Var* pop(Scope*);
Var* dd_argc_var(Scope*);
Scope* global_scope(void);
Scope* parent_scope(void);
int dd_argc(Scope*);

#endif /* SCOPE_H */
