#ifndef SCOPE_H
#define SCOPE_H

/**
 ** Symbol table.  This actually hold the memory for values created
 ** in this scope.  Child scopes will point to these vars
 **/

typedef struct tag_Symtable Symtable;
struct tag_Symtable {
	Var *value;			/* variable holder */
	Symtable *next;		/* dont use var->next.  Bad mojo */
};

/**
 ** A data dictionary.  value[0] holds the var representation of argc
 **/
typedef struct {
	int count;
	int size;
	char **name;		/* variable name */
	Var **value;
} Dictionary;


typedef struct tag_Stack Stack;
struct tag_Stack {
	int size;
	int top;
	Var **value;
};


typedef struct tag_Scope Scope;

struct tag_Scope {
	Dictionary *dd;		/* named variable data dictionary */
	Dictionary *args;	/* number arguments data dictionary */
	Symtable *symtab;		/* local symbol table. */
	Symtable *tmp;		/* tmp memory list */
	Stack *stack;		/* local stack */
	UFUNC *ufunc;		/* function pointer */

	Var *rval;			/* value returned */
	int broken;			/* loop counter flag */
	int loop;			/* loop counter flag */
	int returned;		/* loop counter flag */
};

/**
 ** When the scope is destroyed, everything in symtab (and of course stack)
 ** also gets destroyed.  dd and args don't, so don't put any memory in there.
 ** (besides duplicated names).
 **/

Var * dd_get_argv(Scope *s, int n);
void dd_put(Scope *s, char *name, Var *v);
void dd_unput_argv(Scope *s);
Var *dd_find(Scope *, char *);

Scope *new_scope();
void scope_push(Scope *);
Scope * scope_pop();
Scope * scope_tos();
void free_scope(Scope *);
void push(Scope *, Var *);
Var *pop(Scope *);
Var *dd_argc_var(Scope *);
Scope *global_scope();
Scope *parent_scope();
void clean_scope(Scope *);
void cleanup(Scope *);
int dd_argc(Scope *);

#endif /* SCOPE_H */
