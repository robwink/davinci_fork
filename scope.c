#include "parser.h"

/*

   Values should only become permanent when going through set.
   The rest of the time they should be temporary, and cleanable
   at the end of a statement evaluation.

   This invokes the idea of a local stack, and a symbol table, per scope.
   Additionally, lower scopes need the ability to reference the top
   level scope.

*/

CVEC_NEW_DEFS2(dict_item, RESIZE)

static cvector_void scope_stack;


void clean_scope(Scope*);

void init_scope_stack()
{
	cvec_void(&scope_stack, 0, 8, sizeof(Scope), NULL, NULL);
}


int scope_stack_count()
{
	return scope_stack.size;
}

Scope* scope_stack_get(int i)
{
	return CVEC_GET_VOID(&scope_stack, Scope, i);
}

Scope* scope_stack_back()
{
	return cvec_back_void(&scope_stack);
}



void scope_push(Scope* s)
{
	cvec_push_void(&scope_stack, s);
}

void scope_pop()
{
	// TODO(rswinkle): use clean_scope as elem_free?
	Scope* s = cvec_back_void(&scope_stack);
	clean_scope(s);

	cvec_pop_void(&scope_stack, NULL);
}

Scope* scope_tos()
{
	return (Scope*)cvec_back_void(&scope_stack);
}

Scope* global_scope()
{
	return (Scope*)&scope_stack.a[0];
}

Scope* parent_scope()
{
	//TODO(rswinkle): why is parent -2 and not -1?
	int count = (scope_stack.size > 2) ? scope_stack.size-2 : 0;
	return CVEC_GET_VOID(&scope_stack, Scope, count);
}

Var* dd_find(Scope* s, char* name)
{
	dict_item* p;

	int i;
	for (i = 1; i < s->dd.size; i++) {
		p = &s->dd.a[i];
		if (p->name && !strcmp(p->name, name)) {
			return p->value;
		}
	}
	return NULL;
}

Var* dd_get_argv(Scope* s, int n)
{
	if (n < s->args.size) {
		return s->args.a[n].value;
	}
	return NULL;
}

Var* dd_get_argc(Scope* s)
{
	return newInt(s->args.size - 1);
}

void dd_put(Scope* s, char* name, Var* v)
{
	int i;
	dict_item* p;

	for (i = 1; i < s->dd.size; i++) {
		p = &s->dd.a[i];
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
	cvec_push_dict_item(&s->dd, &item);
}

// stick an arg on the end of the arg list.
// Update argc.
int dd_put_argv(Scope* s, Var* v)
{
	cvector_dict_item* dd = &s->args;

	/*
	** WARNING: This looks like it will break if you try to global a
	**          value that was passed.
	*/
	dict_item item = { 0 };

	item.value = v;
	item.name = V_NAME(v);
	V_NAME(v) = NULL;

	cvec_push_dict_item(dd, &item);

	// subtract 1 for $0
	V_INT(dd->a[0].value) = dd->size - 1;

	return dd->size - 1;
}

// NOTE(rswinkle): This literally does nothing!
void dd_unput_argv(Scope* s)
{
	Var* v;
	int i;
	dict_item* p;
	cvector_dict_item* dd = &s->args;

	for (i = 1; i < dd->size; i++) {
		p = &dd->a[i];
		v         = p->value;
		V_NAME(v) = p->name;
	}
}

int dd_argc(Scope* s)
{
	// subtract 1 for $0
	return s->args.size - 1;
}

//NOTE(rswinkle): not used anywhere!
// return argc as a Var
/*
Var* dd_argc_var(Scope* s)
{
	return s->args.a[0].value;
}
*/

Var* dd_make_arglist(Scope* s)
{
	Var* p;
	int i;
	void* zero;
	dict_item* item;

	cvector_dict_item* dd = &s->args;
	Var* v         = new_struct(dd->size);

	for (i = 1; i < dd->size; i++) {
		item = &dd->a[i];
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

void init_dd(cvector_dict_item* d)
{
	cvec_dict_item(d, 1, 1, NULL, NULL);

	dict_item* p = &d->a[0];

	// Make a var for $argc
	p->value = calloc(1, sizeof(Var));

	// that cast isn't even necessary right?
	// why don't we use newInt?  Why don't we want to mem_malloc it?
	// (which is called in newInt -> newVal -> newVar)
	make_sym(p->value, DV_INT32, (char*)"0");
	V_TYPE(p->value) = ID_VAL;
}

void free_var2(void* v)
{
	free_var(*(Var**)v);
}

void init_scope(Scope* s)
{
	//not sure if this is necessary
	memset(s, 0, sizeof(Scope));

	// TODO(rswinkle): add elem_free later
	//
	// These actually aren't even necessary since cvec allocator macro handles
	// 0 capacity correctly, but when/if we use elem_free it makes more sense
	cvec_varptr(&s->symtab, 0, 8, NULL, NULL);
	cvec_varptr(&s->stack, 0, 2, NULL, NULL);

	cvec_varptr(&s->tmp, 0, 16, NULL, NULL);

	init_dd(&s->dd);
	init_dd(&s->args);
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
}

void push(Scope* scope, Var* v)
{
	cvec_push_varptr(&scope->stack, &v);
}

Var* pop(Scope* scope)
{
	cvector_varptr* s = &scope->stack;

	if (!s->size) return NULL;

	Var* ret;
	cvec_pop_varptr(s, &ret);

	return ret;
}

void clean_table(cvector_varptr* vec)
{
	for (int i=0; i<vec->size; ++i) {
		free(vec->a[i]);
	}
	cvec_free_varptr(vec);
}

void clean_stack(Scope* scope)
{
	cvector_varptr* s = &scope->stack;

	Var* v;
	while (s->size) {
		cvec_pop_varptr(s, &v);
		if (!v) continue;

		if (mem_claim(v)) free_var(v);
	}
}

// only used 6 times, 1 in main.c, 5 in p.c
// Clean the stack and tmptab of the current scope
void cleanup(Scope* scope)
{
	clean_stack(scope);
	
	if (scope->tmp.capacity) {
		if (!scope->tmp.a)
			printf("%p %p %zu %zu\n", &scope->tmp, scope->tmp.a, scope->tmp.size, scope->tmp.capacity);

		cvec_free_varptr(&scope->tmp);

		// NOTE(rswinkle): This is necessary or else *bad things* happen
		// because whoever wrote davinci didn't actually think about memory
		// management so things are "cleaned up" in multiple places to be safe
		//scope->tmp.a = NULL;
	}
}

Var* mem_malloc()
{
	size_t count    = 0;

	Scope* scope = scope_tos();
	cvector_varptr* vec = &scope->tmp;

	Var* v       = calloc(1, sizeof(Var));

	// If the top of the tmp scope is null, insert there, instead
	// of adding a new one.  This will prevent the temp list from
	// growing indefinitely.
	//
	// TODO(rswinkle): I don't really grok the above statement.  Why
	// would there ever be a NULL at the end?
	
	if ((count = vec->size) > 0) {
		if (!vec->a[count-1]) {
			vec->a[count-1] = v;
			return v;
		}
	}
	cvec_push_varptr(vec, &v);
	return v;
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

	cvector_varptr* vec = &scope->tmp;

	if ((count = vec->size) == 0) return NULL;

	// TODO(rswinkle): why loop backward?
	for (i = count - 1; i >= 0; --i) {
		if (vec->a[i] == ptr) {
			v = vec->a[i];
			vec->a[i] = NULL;
			return mem_claim_struct(v);
		}
	}

	return NULL;
}


void unload_symtab_modules(Scope* scope)
{
#ifdef BUILD_MODULE_SUPPORT
	cvector_varptr* vec = &scope->symtab;
	Var* v;
	for (int i=0; i<vec->size; ++i) {
		v = vec->a[i];
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
		free_var(scope->dd.a[0].value);
		cvec_free_dict_item(&scope->dd);
	}

	if (scope->args.a) {
		free_var(scope->args.a[0].value);
		cvec_free_dict_item(&scope->args);
	}

	clean_stack(scope);
	if (scope->tmp.capacity) {
		if (!scope->tmp.a)
			printf("%p %p %zu %zu\n", &scope->tmp, scope->tmp.a, scope->tmp.size, scope->tmp.capacity);
		cvec_free_varptr(&scope->tmp);
		// NOTE(rswinkle): again this is necessary or bad things happen ...
		//scope->tmp.a = NULL;
	}

	/* Clean modules since before cleaning symbol table
	   since they have a builtin mechansim of removing
	   themselves from the symbol table. Not doing so
	   causes a double free on the Symtable corresponding
	   to the module variable in the global symbol table. */
	unload_symtab_modules(scope);

	// replace with cvec_free with elem_free
	clean_table(&scope->symtab);

	cvec_free_varptr(&scope->stack);
}
