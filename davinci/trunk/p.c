#include "parser.h"

/**
 ** This file contains routines to interact with the yacc grammar.
 **
 ** evaluate() - Evaluate a parse tree
 ** p_mkval() - Convert an input string to a value
 ** p_mknod() - Make a node of the specified type
 **/

#define PRINTABLE_STR(s) ((s)? (s): "(null)")

char *unescape(char *), *unquote(char *);

/**
 ** p_mknod() - Make a node of the specified type
 **/
Var *
p_mknod(int type, Var * left, Var * right)
{
    Var *v;
    Node *n;

    /* v = newVar(); */
	/* mem_claim(v); */
    v = calloc(1, sizeof(Var));
    n = V_NODE(v);
    v->type = type;

    n->left = left;
    n->right = right;

    return (v);
}

/**
 ** p_mkval() - Convert an input string to a value
 **/

Var *
p_mkval(int type, char *str)
{
    Var *v;
    /* v = newVar(); */
	/* mem_claim(v); */
    v = calloc(1, sizeof(Var));

    switch (type) {
        case ID_STRING:
            V_STRING(v) = strdup(unescape(unquote(str)));
            V_TYPE(v) = ID_STRING;
            break;
        case ID_IVAL:
            make_sym(v, INT, str);
            V_TYPE(v) = ID_VAL;
            break;
        case ID_RVAL:
            make_sym(v, FLOAT, str);
            V_TYPE(v) = ID_VAL;
            break;
        case ID_ID:
            V_NAME(v) = strdup(str);
            V_TYPE(v) = ID_UNK;
            break;
    }
    return (v);
}


int
is_zero(Var * v)
{
    Var *e;

    if (v == NULL)
        return (1);

    if ((e = eval(v)) != NULL)
        v = e;
    if (V_TYPE(v) == ID_VAL && V_DSIZE(v) == 1) {
        switch (V_FORMAT(v)) {
            case BYTE:
            case SHORT:
            case INT:
                return (extract_int(v, 0) == 0);
            case FLOAT:
            case DOUBLE:
                return (extract_float(v, 0) == 0.0);
        }
    } else {
		int i;
		for (i = 0 ; i < V_DSIZE(v) ; i++) {
			switch (V_FORMAT(v)) {
				case BYTE:
				case SHORT:
				case INT:
					if (extract_int(v, i) == 0) return(1);
				case FLOAT:
				case DOUBLE:
					if (extract_float(v, i) == 0.0) return(1);
			}
		}
		return (0);
	}
}

/**
 ** evaluate() - Evaluate a parse tree
 **/


Var *
evaluate(Var * n)
{
    Var *left, *right, *range, *where;
    Var *p1 = NULL, *p2 = NULL, *p3 = NULL;
    Scope *scope = scope_tos();
    int type;

    if (n == NULL) return (NULL);
    type = V_TYPE(n);

    /**
     ** These are not nodes, but merely vals.  push 'em. and return; 
     **/
    switch (type) {
        case ID_IVAL:
        case ID_RVAL:
        case ID_ID:
        case ID_UNK:
        case ID_STRING:
        case ID_VAL:
        case ID_TEXT:					/*Added: Thu Mar  2 16:00:18 MST 2000*/
            push(scope, V_DUP(n));
            return (NULL);
    }

    left = V_NODE(n)->left;
    right = V_NODE(n)->right;

    if (type == ID_ARGV) {
		if (left) {
			evaluate(left);
			p1 = pop(scope);
		}

		if (right) {
			evaluate(right);
			p2 = pop(scope);
		}
        push(scope, pp_argv(p1, p2));
        return (NULL);
    }


    switch (type) {
		case ID_LINE:
		{
			if (debug && right) printf("--> %s", V_STRING(right));
			return(evaluate(left));
			break;
		}
        case ID_OR: /* binary operators */
        case ID_AND:
        case ID_EQ:
        case ID_NE:
        case ID_LT:
        case ID_GT:
        case ID_LE:
        case ID_GE:
        case ID_ADD:
        case ID_SUB:
        case ID_MULT:
        case ID_DIV:
        case ID_MOD:
        case ID_POW:
            if (left) evaluate(left);
            if (right) evaluate(right);
            p2 = pop(scope);
            p1 = pop(scope);
            push(scope, pp_math(p1, type, p2));
            break;

		case ID_INC:
		case ID_DEC:
            if (left) evaluate(left);
            if (right) evaluate(right);
			p2 = pop(scope);
			p1 = pop(scope);

			switch (type) {
				case ID_INC: p3 = pp_math(p1, ID_ADD, p2); break;
				case ID_DEC: p3 = pp_math(p1, ID_SUB, p2); break;
			}

			if (V_TYPE(left) == ID_DEREF) {
				evaluate(V_NODE(left)->left);
				evaluate(V_NODE(left)->right);

                p2 = pop(scope);
                p1 = pop(scope);
                push(scope, pp_set_struct(p1, p2, p3));
			} else {
				push(scope, pp_set_var(p1, NULL, p3));
			}
			break;

        case ID_UMINUS:
            if (left) evaluate(left);
            if (right) evaluate(right);
            p1 = pop(scope);
            push(scope, pp_math(NULL, ID_SUB, p1));
            break;

        case ID_RETURN:
			if (left != NULL) {
				cleanup(scope);
				evaluate(left);
				p1 = pop(scope);
				if ((p2 = eval(p1)) != NULL) p1 = p2;
				scope->rval = p1;
			}
			scope->returned = 1;
            break;

        case ID_BREAK:
            scope->broken = scope->loop;
            break;

        case ID_CONT:
            scope->broken = scope->loop + 1;
            break;

        case ID_FOR:    /* just a special holder for while loops */
            if (left) evaluate(left);
            if (!scope->returned) cleanup(scope);
            break;

        case ID_WHILE:
            scope->loop++;
            while (1) {
                if (left != NULL) {
                    evaluate(left);
                    if (is_zero(pop(scope)))
						break;
                }
                scope->broken = 0;
                evaluate(right);
                if (scope->returned != 0 || 
						(scope->broken != 0 && scope->broken <= scope->loop)) {
                    break;
                }
            /**
             ** special case for FOR loops.
             **/
                if (V_TYPE(right) == ID_FOR) {
                    evaluate(V_NODE(right)->right);
                }
				if (!scope->returned) {
					cleanup(scope);
					if (scope->broken > scope->loop) {
						continue;
					}
				}
            }
            scope->loop--;
            scope->broken = 0;
            if (!scope->returned) cleanup(scope);
            break;


        case ID_LIST:
            while (!scope->returned && !scope->broken && V_TYPE(n) == ID_LIST) {
                evaluate(left);
                if (scope->broken == 0 && scope->returned == 0) {
                    n = right;
                    left = V_NODE(n)->left;
                    right = V_NODE(n)->right;
                }
                if (!scope->returned) cleanup(scope);
            }
            if (scope->broken == 0 && scope->returned == 0)
                evaluate(n);
            break;

        case ID_ELSE:
			/**
			 ** A true and a false option.  Run the true one.
			 **/
            evaluate(left);
            break;

        case ID_IF:
            evaluate(left);
            if (!is_zero(pop(scope))) {
                evaluate(right);
            } else if (right && V_TYPE(right) == ID_ELSE) {
				/**
				 ** In the event of a test that fails, and our child node
				 ** is an else, run the false node.
				 **/
                evaluate(V_NODE(right)->right);
            }
            break;

        case ID_FUNCT:
            p1 = NULL;
            if (right != NULL) {
                evaluate(right);
                p1 = pop(scope);
            }

			if (V_TYPE(left) == ID_DEREF) {
				/* module dereference */
				evaluate(left);

				p2 = pop(scope);
				if (p2 && V_TYPE(p2) == ID_FUNCTION){
					push(scope, pp_call_dv_module_func(V_FUNC(p2), p1));
				}
				else {
					parse_error("Dereference is not a function.");
				}
			}
			else {
				push(scope, pp_func(left, p1));
			}

			scope->returned = 0;

            break;

        case ID_ARGS:
            if (left) evaluate(left);
            if (right) evaluate(right);
            if (right) p2 = pop(scope);
            if (left) p1 = pop(scope);
            push(scope, pp_mk_arglist(p1, p2));
            break;

        case ID_ARG:
            if (left != NULL) {
                evaluate(left);
                evaluate(right);
                p2 = pop(scope);
                p1 = pop(scope);
                push(scope, pp_keyword_to_arg(p1, p2));
            } else {
                evaluate(right);
            }
            break;

        case ID_ARRAY:
			evaluate(left);
			evaluate(right);
			p2 = pop(scope);
			p1 = pop(scope);
			push(scope, pp_range(p1, p2));
			break;

        case ID_RANGE:  /* pair of range values */
            if (left) {
				evaluate(left);
				p1 = pop(scope);
			}
			if (right) {
				evaluate(right);
				p2 = pop(scope);
			}
            push(scope, pp_mk_range(p1, p2));
            break;

        case ID_RSTEP:  /* ranges, with a step value */
            if (left) {
				evaluate(left);
				p1 = pop(scope);
			}
			if (right) {
				evaluate(right);
				p2 = pop(scope);
			}
            push(scope, pp_mk_rstep(p1, p2));
            break;

        case ID_RANGES: /* list of range values */
            if (left == NULL) {
				/**
				 ** A single range value.  mk_range will deal with it.
				 **/
                evaluate(right);
			} else {
				evaluate(left);
				evaluate(right);
                p2 = pop(scope);
                p1 = pop(scope);
				push(scope, pp_add_range(p1,p2));
			}
            break;

        case ID_SET:    /* equivalence */
            if (V_TYPE(left) == ID_ARRAY) {
                evaluate(V_NODE(left)->left);
                evaluate(V_NODE(left)->right);
                evaluate(right);

                p2 = pop(scope);
                range = pop(scope);
                p1 = pop(scope);

                push(scope, pp_set_var(p1, range, p2));
			} else if (V_TYPE(left) == ID_WHERE) {

/*
                                    ID_SET
                                  /        \
                           ID_WHERE          EXPR
                          /        \
                         ID       VAL
*/
				evaluate(V_NODE(left)->left);
				evaluate(V_NODE(left)->right);
				evaluate(right);

                p2 = pop(scope);
                where = pop(scope);
                p1 = pop(scope);
				push(scope, pp_set_where(p1, where, p2));

			} else if (V_TYPE(left) == ID_DEREF) {

/*
                                    ID_SET
                                  /        \
                           ID_DEREF          EXPR
                          /        \
                         ID         ID 
*/
				evaluate(V_NODE(left)->left);
				evaluate(V_NODE(left)->right);
				evaluate(right);

                p3 = pop(scope);
                p2 = pop(scope);
                p1 = pop(scope);
                push(scope, pp_set_struct(p1, p2, p3));
            } else {
                evaluate(left);
                evaluate(right);
                p2 = pop(scope);
                p1 = pop(scope);
                push(scope, pp_set_var(p1, NULL, p2));
            }
            break;
	    case ID_CAT:
            if (left) evaluate(left);
            if (right) evaluate(right);
            p2 = pop(scope);
            p1 = pop(scope);
			push(scope, do_cat(p1, p2, 0));
		    break;

		case ID_DEREF:
			/*
			** Derefernce of a structure/module element.
			** Push the assoicated Var
			*/
			evaluate(left);
			evaluate(right);

            p2 = pop(scope);
            p1 = pop(scope);

			/* not a structure -- try modules */
			if (p1 == NULL || p2 == NULL) {
				parse_error("dereference with null value");
				push(scope, NULL);
			} else if (p3 = search_in_list_of_loaded_modules(V_NAME(p1))) {
				vfuncptr t;

				if(t = find_module_func(&V_MODULE(p3), V_NAME(p2))){
					p1 = newVar();
					p1->value.function = t;
					p1->type = ID_FUNCTION;

					/* push the function for the caller to grab */
					push(scope, p1);
				}
				else {
					parse_error("Module %s does not contain member: %s",
						V_NAME(p3), PRINTABLE_STR(V_NAME(p2)));

					push(scope, NULL);
				}
			} else {
				if (find_struct(p1, p2, &p3) != -1) {
					push(scope, p3);
				}
				else {
					parse_error("structure does not contain member: %s",
								V_NAME(p2));
					push(scope, NULL);
				}
			}
			
			break;

		case ID_CONSTRUCT:
			/*
			** Construct a structure from the passed arguments
			*/
			if (left != NULL) {
				evaluate(left);
				p1 = pop(scope);
			} else {
				p1 = NULL;
			}
			push(scope, create_struct(p1));

			break;

		default:
			fprintf(stderr, "Unknown type: %d\n", type);
    }
    return (NULL);
}


Var *
p_lnode(Var * parent, Var * child)
{
    Var *p;

    for (p = parent; V_NODE(p)->left != NULL; p = V_NODE(p)->left);

    V_NODE(p)->left = child;
    return (parent);
}

Var *
p_rnode(Var * parent, Var * child)
{
    Var *p;

    for (p = parent; V_NODE(p)->right != NULL; p = V_NODE(p)->right);

    V_NODE(p)->right = child;
    return (parent);
}

Var *
p_rlist(int type, Var * list, Var * stmt)
{
    Var *p;
    if (stmt == NULL)
        return (list);
    if (list == NULL)
        return (stmt);
    if (V_TYPE(list) != type)
        return (p_mknod(type, list, stmt));
    for (p = list; V_TYPE(V_NODE(p)->right) == type; p = V_NODE(p)->right);
    V_NODE(p)->right = p_mknod(type, V_NODE(p)->right, stmt);
    return (list);

}

Var *
p_llist(int type, Var * list, Var * stmt)
{
    Var *p;
    if (stmt == NULL)
        return (list);
    if (list == NULL)
        return (stmt);
    if (V_TYPE(list) != type)
        return (p_mknod(type, stmt, list));
    for (p = list; V_TYPE(V_NODE(p)->left) == type; p = V_NODE(p)->left);
    V_NODE(p)->left = p_mknod(type, stmt, V_NODE(p)->left);
    return (list);

}


/**
 **
 **/
int
check_ufunc(Var *v)
{
    Scope *scope = scope_tos();

    if (v == NULL) return 0;

    if (scope != global_scope()) {
        if (scope->ufunc && scope->ufunc->tree == NULL)  {
            scope->ufunc->tree = v;
            clean_scope(scope_pop());
            return(0);
        }
    }
    return(1);
}

/**
 ** Free all the memory allocated in a tree.  Do this like evaluate does.
 **/

void
free_tree(Var *n)
{
    Var *left, *right;
    int type;

    while(1) {
        if (n == NULL) return;
        type = V_TYPE(n);

        /**
         ** These are not nodes, but merely vals.  free 'em. and return; 
         **/
        switch (type) {
			case ID_IVAL:
			case ID_RVAL:
			case ID_VAL:
			case ID_STRING:
			case ID_ID:
			case ID_UNK:
			case ID_KEYWORD:
			case ID_TEXT:			/*Added: Thu Mar  2 16:01:19 MST 2000*/
			   free_var(n);
			   return;
        }
	
        left = V_NODE(n)->left;
        right = V_NODE(n)->right;

		if (left != NULL && left != right) free_tree(left);
		free(n);
		n = right;
    }
}

