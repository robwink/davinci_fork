/******************************** symbol.c *********************************/
#include "parser.h"

/**
 **
 ** Symbol table management routines.
 **
 **		get_sym(name)	- search symbol table for named symbol 
 **		put_sym(name)	- Insert (replace) named symbol in table
 **
 ** The symbol table is linked together using V_NEXT(v).
 **/

/**
 ** get_sym()	 - search symbol table for named symbol
 **/


Var *
HasValue(vfuncptr func, Var *arg)
{
	Scope *scope = scope_tos();
	Symtable *s;
	char *name;
	Var *v = new(Var);
	int i;

	make_sym(v, INT, "0");
	V_TYPE(v) = ID_VAL;

	if (arg == NULL || V_NAME(arg) == NULL) {
		V_INT(v) = 1;
		return(v);
	}

	name = V_NAME(arg);
	/**
	 ** search the dd first.
	 **/
	if (scope->dd && scope->dd->count && scope->dd->name) {
		for (i = 1 ; i < scope->dd->count ; i++) {
			if (scope->dd->name[i] && !strcmp(scope->dd->name[i], name)) {
				if (scope->dd->value[i] != NULL) {
					V_INT(v) = 1;
					return(v);
				} else {
					V_INT(v) = 0;
					return(v);
				}
			}
		}
	}
	for (s = scope->symtab ; s != NULL ; s = s->next) {
		if (V_NAME(s->value) && !strcmp(V_NAME(s->value), name)) {
			return(p_mkval(ID_IVAL, "1"));
		}
	}
	V_INT(v) = 0;
	return(v);
}


Var *search_dd(Scope *scope, char *name)
{
	int i;
	if (scope->dd && scope->dd->count && scope->dd->name) {
		for (i = 1 ; i < scope->dd->count ; i++) {
			if (scope->dd->name[i] && !strcmp(scope->dd->name[i], name)) {
				return(scope->dd->value[i]);
			}
		}
	}
	return(NULL);
}

/**
 ** remove a symbol from the symtab.
 ** this destroys the symtab and hands back the value.
 **/

Var *
rm_symtab(Var *v)
{
	Scope *scope = scope_tos();
	Symtable *s, *t;

	if (scope->symtab == NULL) return(NULL);
	if (scope->symtab->value == v) {
		s = scope->symtab;
		scope->symtab = s->next;
		free(s);
		return(v);
	} else {
		for (s = scope->symtab ; s->next != NULL ; s = s->next) {
			if (s->next->value == v) {
				t = s->next;
				s->next = t->next;
				t->next = NULL;
				free(t);
				return(v);
			}
		}
	}
	return(NULL);
}

Var *
search_symtab(Scope *scope, char *name) 
{
	Symtable *s;
	for (s = scope->symtab ; s != NULL ; s = s->next) {
		if (V_NAME(s->value) && !strcmp(V_NAME(s->value), name))
			return(s->value);
	}
	return(NULL);
}

Var *get_sym(char *name)
{
	Scope *scope = scope_tos();
	Var *v;

	if ((v = search_dd(scope, name)) != NULL) return(v);
	if ((v = search_symtab(scope, name)) != NULL) return(v);
	return(NULL);
}
Var *get_global_sym(char *name)
{
	Scope *scope = global_scope();
	Var *v;

	if ((v = search_dd(scope, name)) != NULL) return(v);
	if ((v = search_symtab(scope, name)) != NULL) return(v);
	return(NULL);
}

/**
 ** delete symbol from symbol table
 **/

void
rm_sym(char *name)
{
	Scope *scope = scope_tos();
	Symtable *s, *last = NULL;

	if (name == NULL) return;

	for (s = scope->symtab ; s != NULL ; s = s->next) {
		if (!strcmp(V_NAME(s->value), name)) {
			if (last != NULL) {
				last->next = s->next;
			} else {
				scope->symtab = s->next;
			}
			free_var(s->value);
			free(s);
			return;
		}
		last = s;
	}
	return;
}

/**
 ** put_sym()	 - store symbol in symbol table
 **/

Var *
sym_put(Scope *scope, Var *s)
{
	Symtable *symtab;
	Var *v, tmp;
	/**
	 ** If symbol already exists, we must re-use its structure.
	 ** and, at the same time, free up its old data values
	 **/
	if ((v = get_sym(V_NAME(s))) != NULL) {

		/* swap around the values */
		tmp = *v;
		*v = *s;
		*s = tmp;

		/* put the names back */
		tmp.name = v->name;
		v->name = s->name;
		s->name = tmp.name;
		free_var(s);
	} else {
		symtab = (Symtable *)calloc(1, sizeof(Symtable));
		symtab->value = s;
		symtab->next = scope->symtab;
		scope->symtab = symtab;
	}
	/**
	 ** Possible this has already been done.  do it again for safety.
	 **/
	mem_claim(s);
	return(s);
}

Var *
put_sym(Var *s)
{
	return(sym_put(scope_tos(), s));
}
Var *
put_global_sym(Var *s)
{
	return(sym_put(global_scope(), s));
}


/**
 ** Force symtab evaluation
 **/

Var *
eval(Var *v)
{
	if (v == NULL) return(NULL);
	if (V_NAME(v)) v = get_sym(V_NAME(v));
	return(v);
}


/**
 ** enumerate the symbol table.
 **/
Var *
ff_list(vfuncptr func, Var *arg)
{
	Scope *scope = scope_tos();
	Symtable *s= scope->symtab;

	Var *v;
	char bytes[256];

	for (s = scope->symtab ; s != NULL ; s = s->next) {
		v = s->value;
		if (V_TYPE(v) == ID_STRING) {
			sprintf(bytes, "%d", strlen(V_STRING(v)));
			commaize(bytes);
			printf("%s:\tstring [%s bytes]\n",
				V_NAME(v), bytes);
		} else if (V_NAME(v) != NULL && V_TYPE(v) == ID_VAL) {
			sprintf(bytes, "%d", NBYTES(V_FORMAT(v))*V_DSIZE(v));
			commaize(bytes);

			printf("%s:\t%dx%dx%d array of %s, %s format [%s bytes]\n",
					V_NAME(v) ? V_NAME(v) : "", 	
					GetSamples(V_SIZE(v),V_ORG(v)),
					GetLines(V_SIZE(v),V_ORG(v)),
					GetBands(V_SIZE(v),V_ORG(v)),
					Format2Str(V_FORMAT(v)),
					Org2Str(V_ORG(v)),
					bytes);
		}
	}
	return(NULL);
}

void
free_var(Var *v)
{
	int type;
	int i;
	if (v == NULL) return;

	type=V_TYPE(v);
	switch (type) {
		case ID_IVAL:
		case ID_RVAL:
		case ID_VAL:
			if (V_DATA(v)) free(V_DATA(v));
			if (V_NAME(v)) free(V_NAME(v));
			break;
		case ID_STRING:
			if (V_STRING(v)) free(V_STRING(v));
			if (V_NAME(v)) free(V_NAME(v));
			break;
		case ID_STRUCT:
			for (i = 0 ; i < V_STRUCT(v).count ; i++) {
				free_var(V_STRUCT(v).data[i]);
				free(V_STRUCT(v).names[i]);
			}
			if (V_NAME(v)) free(V_NAME(v));
			break;

		default:
			if (V_NAME(v)) free(V_NAME(v));
			break;
	}
	free(v);
}
