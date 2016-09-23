#include "darray.h"
#include "parser.h"

/*

   Values should only become permanent when going through set.
   The rest of the time they should be temporary, and cleanable
   at the end of a statement evaluation.

   This invokes the idea of a local stack, and a symbol table, per scope.
   Additionally, lower scopes need the ability to reference the top
   level scope.

*/

//static int cvector_void scope_stack = { 0 };

// TODO(rswinkle) replace with vec_void or vector_Scope
static int scope_count     = 0;
static int scope_size      = 0;
static Scope** scope_stack = NULL;

int scope_stack_count()
{
	return scope_count;
}

void scope_push(Scope* s)
{
//	if (scope_stack.capacity == 0) {
//		cvec_void(&scope_stack, 0, 2, sizeof(Scope), free_scope, new_scope);
//	}
//
	if (scope_count == scope_size) {
		scope_size  = max(scope_size * 2, 2);
		scope_stack = (Scope**)my_realloc(scope_stack, scope_size * sizeof(Scope*));
	}
	scope_stack[scope_count++] = s;
}

Scope* scope_pop()
{
	return (scope_stack[--scope_count]);
}

Scope* scope_tos()
{
	return (scope_stack[scope_count - 1]);
}

Scope* global_scope()
{
	return (scope_stack[0]);
}

Scope* parent_scope()
{
	int count = max(scope_count - 2, 0);
	return (scope_stack[count]);
}

Var* dd_find(Scope* s, char* name)
{
	cvector_void* dd = &s->dd;
	dict_item* p;

	int i;
	for (i = 1; i < dd->size; i++) {
		p = CVEC_GET_VOID(dd, dict_item, i);
		if (p->name && !strcmp(p->name, name)) {
			return p->value;
		}
	}
	return NULL;
}

Var* dd_get_argv(Scope* s, int n)
{
	if (n < s->args.size) {
		return CVEC_GET_VOID(&s->args, dict_item, n)->value;
	}
	return NULL;
}

Var* dd_get_argc(Scope* s)
{
	return newInt(s->args.size - 1);
}

void dd_put(Scope* s, char* name, Var* v)
{
	cvector_void* dd = &s->dd;
	int i;
	dict_item* p;

	for (i = 1; i < dd->size; i++) {
		p = CVEC_GET_VOID(dd, dict_item, i);
		if (!strcmp(p->name, name)) {
			p->value = v;
			return;
		}
	}

	// C99 spec 6.7.8.21
	// If there are fewer initializers in a brace-enclosed list than there are
	// elements or members of an aggregate, or fewer characters in a string
	// literal used to initialize an array of known size than there are elements
	// in the array, the remainder of the aggregate shall be initialized implicitly
	// the same as objects that have static storage duration.
	//
	// Actually GNU99 allows empty braces too
	dict_item item = { 0 };

	if (name && strlen(name))
		item.name = strdup(name); // strdup here?

	item.value = v;
	cvec_push_void(dd, &item);
}

// stick an arg on the end of the arg list.
// Update argc.
int dd_put_argv(Scope* s, Var* v)
{
	cvector_void* dd = &s->args;

	/*
	** WARNING: This looks like it will break if you try to global a
	**          value that was passed.
	*/
	dict_item item = { 0 };

	item.value = v;
	item.name = V_NAME(v);
	V_NAME(v) = NULL;

	cvec_push_void(dd, &item);

	dict_item* p = CVEC_GET_VOID(dd, dict_item, 0);
	// subtract 1 for $0
	V_INT(p->value) = dd->size - 1;

	return dd->size - 1;
}

void dd_unput_argv(Scope* s)
{
	cvector_void* dd = &s->args;
	Var* v;
	int i;
	dict_item* p;

	for (i = 1; i < dd->size; i++) {
		p = CVEC_GET_VOID(dd, dict_item, i);
		v         = p->value;
		V_NAME(v) = p->name;
	}
}

int dd_argc(Scope* s)
{
	// subtract 1 for $0
	return s->args.size - 1;
}

// return argc as a Var
Var* dd_argc_var(Scope* s)
{
	return CVEC_GET_VOID(&s->args, dict_item, 0);
}

Var* dd_make_arglist(Scope* s)
{
	cvector_void* dd = &s->args;
	Var* v         = new_struct(dd->size);
	Var* p;
	int i;
	void* zero;
	dict_item* item;

	for (i = 1; i < dd->size; i++) {
		item = CVEC_GET_VOID(dd, dict_item, i);
		if (V_TYPE(item->value) == ID_UNK) {
			zero = calloc(1, 1);
			p    = newVal(BSQ, 1, 1, 1, DV_UINT8, zero);
			mem_claim(p);
		} else {
			p = V_DUP(item->value);
		}
		add_struct(v, item->name, p);
	}
	return v;
}

void init_dd(cvector_void* d)
{
	cvec_void(d, 1, 1, sizeof(dict_item), NULL, NULL);

	dict_item* p = CVEC_GET_VOID(d, dict_item, 0);

	// Make a var for $argc
	p->value = (Var*)calloc(1, sizeof(Var));

	// that cast isn't even necessary right?
	make_sym(p->value, DV_INT32, (char*)"0");
	V_TYPE(p->value) = ID_VAL;
}

/**
 ** Allocate an init space for a scope
 **/
Scope* new_scope()
{
	Scope* s = (Scope*)calloc(1, sizeof(Scope));

	init_dd(&s->dd);
	init_dd(&s->args);
	return s;
}

// NOTE(rswinkle): Only used 3 times, all in ufunc.c::dispatch_ufunc()
void free_scope(Scope* s)
{
	/* this looks wrong
	   for (i = 0 ; i < s->dd->count ; i++) {
	   free(s->dd->name);
	   }
	*/

	cvec_free_void(&s->dd);
	cvec_free_void(&s->args);

	free(s);
}

void push(Scope* scope, Var* v)
{
	varptr t = { v };

	if (!scope->stack.a)
		cvec_void(&scope->stack, 0, 2, sizeof(varptr), NULL, NULL);

	cvec_push_void(&scope->stack, &t);
}

Var* pop(Scope* scope)
{
	cvector_void* s = &scope->stack;

	if (!s->size) return NULL;

	varptr ret;
	cvec_pop_void(s, &ret);

	return ret.p;
}

void clean_table(Symtable* s)
{
	Symtable* t;

	while (s != NULL) {
		t = s->next;
		free_var(s->value);
		free(s);
		s = t;
	}
}

void clean_stack(Scope* scope)
{
	cvector_void* s = &scope->stack;
	//Var* v;
	// could use v if we wanted
	// cvec_pop_void(s, &v) would work fine
	// it's a void* parameter and the space is the
	// same.  Really you could pass the address of anything
	// as long as it had >= elem_size space.
	//
	// I only made varptr for if/when I move to cvec_varptr

	varptr r;
	while (s->size) {
		cvec_pop_void(s, &r);
		if (!r.p) continue;

		if (mem_claim(r.p)) free_var(r.p);
	}
}

/**
 ** Clean the stack and tmptab of the current scope
 **/

void cleanup(Scope* scope)
{
	clean_stack(scope);
	if (scope->tmp) Darray_free(scope->tmp, (Darray_FuncPtr)free_var);
	scope->tmp = NULL;
}

/**
 ** allocate memory in the scope tmp list
 **/
Var* mem_malloc()
{
	Scope* scope = scope_tos();
	Var* v       = (Var*)calloc(1, sizeof(Var));
	Var* top     = NULL;
	Var* junk    = NULL;
	int count    = 0;

	if (scope->tmp == NULL) scope->tmp = Darray_create(0);

	/*
	 * If the top of the tmp scope is null, insert there, instead
	 * of adding a new one.  This will prevent the temp list from
	 * gowing indefinitely.
	 */
	if ((count = Darray_count(scope->tmp)) > 0) {
		if (Darray_get(scope->tmp, count - 1, (void**)&top) == 1 && top == NULL) {
			Darray_replace(scope->tmp, count - 1, v, (void**)&junk);
			return (v);
		}
	}

	Darray_add(scope->tmp, v);
	return (v);
}

Var* mem_claim_struct(Var* v)
{
	int i;
	int count;
	Var* data;

	if (V_TYPE(v) == ID_STRUCT) {
		count = get_struct_count(v);
		for (i = 0; i < count; i++) {
			get_struct_element(v, i, NULL, &data);
			mem_claim(data);
		}
	}
	return (v);
}

// claim memory in the scope tmp list, so it doesn't get free'd
// return NULL if it isn't here.
Var* mem_claim(Var* ptr)
{
	Scope* scope = scope_tos();
	Var* v;
	int count;
	int i;

	if (scope->tmp == NULL) return (NULL);
	if ((count = Darray_count(scope->tmp)) == 0) return (NULL);

	for (i = count - 1; i >= 0; i--) {
		if (Darray_get(scope->tmp, i, (void**)&v) == 1 && v == ptr) {
			/*
			** This is it.
			*/
			Darray_replace(scope->tmp, i, NULL, (void**)&v);
			return (mem_claim_struct(v));
		}
	}
	return (NULL);
}

// free the scope tmp list
void mem_free(Scope* scope)
{
	if (scope->tmp) Darray_free(scope->tmp, (Darray_FuncPtr)free_var);
}

void unload_symtab_modules(Scope* scope)
{
#ifdef BUILD_MODULE_SUPPORT
	Symtable* s = scope->symtab;
	while (s) {
		Var* v = s->value;
		s      = s->next;
		if (V_TYPE(v) == ID_MODULE) {
			unload_dv_module(V_NAME(v));
		}
	}
#endif /* BUILD_MODULE_SUPPORT */
}

/**
 ** destroy the scopes dd and args
 **
 ** All the vars in dd belong to parent.  Dont free those.
 ** All the vars in args belong to the parent.  Dont free those.
 ** argv[0]
 **/
void clean_scope(Scope* scope)
{

	// NOTE(rswirkle): Need to think about whether these checks are necessary
	// but for now just replicating old logic
	if (scope->dd.a) {
		free_var(CVEC_GET_VOID(&scope->dd, dict_item, 0)->value);
		cvec_free_void(&scope->dd);
	}

	if (scope->args.a) {
		free_var(CVEC_GET_VOID(&scope->args, dict_item, 0)->value);
		cvec_free_void(&scope->args);
	}

	clean_stack(scope);
	if (scope->tmp) Darray_free(scope->tmp, (Darray_FuncPtr)free_var);
	scope->tmp = NULL;

	/* Clean modules since before cleaning symbol table
	   since they have a builtin mechansim of removing
	   themselves from the symbol table. Not doing so
	   causes a double free on the Symtable corresponding
	   to the module variable in the global symbol table. */
	unload_symtab_modules(scope);

	clean_table(scope->symtab);
	scope->symtab = NULL;

	cvec_free_void(&scope->stack);

	free(scope);
}
