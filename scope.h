#ifndef SCOPE_H
#define SCOPE_H

#include "ufunc.h"

// Symbol table.  This actually hold the memory for values created
// in this scope.  Child scopes will point to these vars
typedef struct Symtable Symtable;
struct Symtable {
	Var* value; // variable holder
	Symtable* next;
};

// A data dictionary.  value[0] holds the var representation of argc
typedef struct Dictionary {
	int count;
	int size;
	char** name; // variable name
	Var** value;
} Dictionary;

typedef struct Stack {
	int size;
	int top;
	Var** value;
} Stack;

typedef struct Scope {
	Dictionary* dd;   // named variable data dictionary
	Dictionary* args; // number arguments data dictionary
	Symtable* symtab; // local symbol table.
	Darray* tmp;      // tmp memory list
	Stack* stack;     // local stack
	UFUNC* ufunc;     // function pointer

	Var* rval;    // value returned
	int broken;   // loop counter flag
	int loop;     // loop counter flag
	int returned; // loop counter flag
} Scope;

// When the scope is destroyed, everything in symtab (and of course stack)
// also gets destroyed.  dd and args don't, so don't put any memory in there.
// (besides duplicated names).

void scope_push(Scope*);
Scope* scope_pop();
Scope* scope_tos();
Scope* global_scope();
Scope* parent_scope();

Var* dd_find(Scope*, char*);
Var* dd_get_argv(Scope* s, int n);
Var* dd_get_argc(Scope* s);
void dd_put(Scope* s, char* name, Var* v);
int dd_put_argv(Scope* s, Var* v);
void dd_unput_argv(Scope* s);
int dd_argc(Scope* s);
Var* dd_argc_var(Scope* s);
Var* dd_make_arglist(Scope* s);

Scope* new_scope();
void free_scope(Scope*);
void push(Scope*, Var*);
Var* pop(Scope*);
void clean_scope(Scope*);
void cleanup(Scope*);

Var* mem_claim(Var*);
Var* mem_malloc();

#endif
