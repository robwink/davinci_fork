/******************************** symbol.c *********************************/
#include "parser.h"

/**
 **
 ** Symbol table management routines.
 **
 **    get_sym(name)   - search symbol table for named symbol
 **    put_sym(name)   - Insert (replace) named symbol in table
 **
 **/


Var* ff_hasvalue(vfuncptr func, Var* arg)
{
	int ac;
	Var **av, *v;

	make_args(&ac, &av, func, arg);
	int ret = 0;
	if (ac == 2 && (v = eval(av[1])) != NULL) {
		ret = 1;
	}
	free(av);
	return newInt(ret);
}

// remove a symbol from the symtab.
// this destroys the symtab and hands back the value.
Var* rm_symtab(Var* v)
{
	Scope* scope = scope_tos();
	
	cvector_varptr* vec = &scope->symtab;
	Var* ptr;

	for (int i=0; i<vec->size; ++i) {
		ptr = vec->a[i];
		if (ptr == v) {
			cvec_erase_varptr(vec, i, i);
			return v;
		}
	}

	return NULL;
}

Var* ff_delete(vfuncptr func, Var* arg)
{
	Var* obj;
	Alist alist[2];
	alist[0]      = make_alist("obj", ID_UNK, NULL, &obj);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);
	if (obj == NULL) return (NULL);

	free_var(obj);

	return (NULL);
}

Var* search_symtab(Scope* scope, char* name)
{
	cvector_varptr* vec = &scope->symtab;

	Var* v;

	for (int i=0; i<vec->size; ++i) {
		v = vec->a[i];
		if (V_NAME(v) && !strcmp(V_NAME(v), name)) {
			return v;
		}
	}

	return NULL;
}

Var* get_sym(char* name)
{
	Scope* scope = scope_tos();
	Var* v;

	if ((v = dd_find(scope, name)) != NULL) return (v);
	if ((v = search_symtab(scope, name)) != NULL) return (v);
	return (NULL);
}

Var* get_global_sym(char* name)
{
	Scope* scope = global_scope();
	Var* v;

	if ((v = dd_find(scope, name)) != NULL) return (v);
	if ((v = search_symtab(scope, name)) != NULL) return (v);
	return (NULL);
}

// delete symbol from symbol table
void rm_sym(char* name)
{
	Scope* scope = scope_tos();

	cvector_varptr* vec = &scope->symtab;
	Var* v;

	for (int i=0; i<vec->size; ++i) {
		v = vec->a[i];
		if (!strcmp(V_NAME(v), name)) {
			free_var(v);
			cvec_erase_varptr(vec, i, i);
			return;
		}
	}

	return;
}

// put_sym()    - store symbol in symbol table
Var* sym_put(Scope* scope, Var* s)
{
	Var *v, tmp;
	/**
	 ** If symbol already exists, we must re-use its structure.
	 ** and, at the same time, free up its old data values
	 **/
	if ((v = get_sym(V_NAME(s))) != NULL) {

		mem_claim(s);
		// swap around the values
		tmp = *v;
		*v  = *s;
		*s  = tmp;

		// put the names back
		//
		// TODO(rswinkle) I really don't get this but
		// somehow commenting these out breaks the test
		// basic/ufunc/call-by-ref.dvtest
		tmp.name = v->name;
		v->name  = s->name;
		s->name  = tmp.name;

		free_var(s);
		s = v;
	} else {
		cvec_push_varptr(&scope->symtab, &s);
		mem_claim(s);
	}
	
	return s;
}

Var* put_sym(Var* s)
{
	return (sym_put(scope_tos(), s));
}

Var* put_global_sym(Var* s)
{
	return (sym_put(global_scope(), s));
}

// Force symtab evaluation
Var* eval(Var* v)
{
	if (v == NULL) return NULL;
	switch (V_TYPE(v)) {
	case ID_STRING:
	case ID_VAL:
	case ID_STRUCT:
	case ID_TEXT: break;
	default:
		if (V_NAME(v)) v = get_sym(V_NAME(v));
		break;
	}
	return (v);
}



/**
 ** enumerate the symbol table.
 **/

//ufunc.c/h
extern avl_tree_t ufuncs_avl;
extern int nufunc;


//ff.h/c
extern struct _vfuncptr vfunclist[];
extern int num_internal_funcs;


Var* ff_list(vfuncptr func, Var* arg)
{
	Scope* scope = scope_tos();

	cvector_varptr* vec = &scope->symtab;

	Var* v;
	int i;
	int list_ufuncs = 0, list_sfuncs = 0;
	Alist alist[3];
	alist[0]      = make_alist("ufunc", DV_INT32, NULL, &list_ufuncs);
	alist[1]      = make_alist("sfunc", DV_INT32, NULL, &list_sfuncs);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (list_ufuncs == 0 && list_sfuncs == 0) {
		for (i=0; i<vec->size; ++i) {
			v = vec->a[i];
			if (V_NAME(v)) pp_print_var(v, V_NAME(v), 0, 0, NULL);
		}
			
	} else {
		int nfuncs = 0;
		int count  = 0;
		char** names;

		if (list_ufuncs) {
			nfuncs += nufunc;
		}
		if (list_sfuncs) {
			nfuncs += num_internal_funcs;
		}

		names = calloc(nfuncs, sizeof(char*));
		if (list_ufuncs) {
			i=0;
			avl_node_t* cur = avl_head(&ufuncs_avl);
			while (cur) {
				UFUNC* u = avl_ref(cur, UFUNC, node);
				names[i++] = strdup(u->name);
				cur = avl_next(cur);
			}
			count += nufunc;
		}
		if (list_sfuncs) {
			for (i = 0; i < num_internal_funcs; i++) {
				names[i + count] = strdup(vfunclist[i].name);
			}
		}
		return (newText(nfuncs, names));
	}

	return (NULL);
}

void free_var(Var* v)
{
	int type;
	int i;
	if (v == NULL) return;

	type = V_TYPE(v);
	switch (type) {
	case ID_IVAL:
	case ID_RVAL:
	case ID_VAL:
		if (V_DATA(v)) free(V_DATA(v));
		break;
	case ID_STRING:
		if (V_STRING(v)) free(V_STRING(v));
		break;
	case ID_STRUCT: free_struct(v); break;
	case ID_TEXT: /*Added: Thu Mar  2 16:03:49 MST 2000*/
		for (i = 0; i < V_TEXT(v).Row; i++) {
			free(V_TEXT(v).text[i]);
		}
		free(V_TEXT(v).text);
		break;
	case ID_ARGS:
		if (V_ARGS(v)) {
			Narray_free(V_ARGS(v), NULL);
		}
		break;

#ifdef BUILD_MODULE_SUPPORT
	case ID_MODULE: del_module(v); break;
#endif /* BUILD_MODULE_SUPPORT */

	default: break;
	}
	if (V_NAME(v)) free(V_NAME(v));
	free(v);
}
