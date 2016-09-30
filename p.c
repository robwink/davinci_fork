#include "func.h"
#include "help.h"
#include "parser.h"
/**
 ** This file contains routines to interact with the yacc grammar.
 **
 ** evaluate() - Evaluate a parse tree
 ** p_mkval() - Convert an input string to a value
 ** p_mknod() - Make a node of the specified type
 **/

#define PRINTABLE_STR(s) ((s) ? (s) : "(null)")
#define MODULE_HELP (const char*)"help"

char* cleanup_input(char* s);
Var* pp_new_parallel(Var* axis, Var* arg);

/**
 ** p_mknod() - Make a node of the specified type
 **/
Var* p_mknod(int type, Var* left, Var* right)
{
	Var* v;
	Node* n;

	/*
	 * 96, 32, 72 on 64-bit
	printf("sizeof(Var) = %zu\n", sizeof(Var));
	printf("sizeof(Node) = %zu\n", sizeof(Node));
	printf("sizeof(Sym) = %zu\n", sizeof(Sym));
	*/

	v       = calloc(1, sizeof(Var));
	n       = V_NODE(v);
	v->type = type;

	n->left  = left;
	n->right = right;

	return (v);
}

/**
 ** p_mkval() - Convert an input string to a value
 **/

Var* p_mkval(int type, char* str)
{
	Var* v;
	v = calloc(1, sizeof(Var));

	switch (type) {
	case ID_STRING:
		V_STRING(v) = strdup(unescape(unquote(str)));
		V_TYPE(v)   = ID_STRING;
		break;
	case ID_IVAL:
		// NOTE(rswinkle) default to i64
		make_sym(v, DV_INT64, str);
		V_TYPE(v) = ID_VAL;
		break;
	case ID_RVAL:
		// NOTE(rswinkle) fix intermediate float bug
		make_sym(v, DV_DOUBLE, str);
		V_TYPE(v) = ID_VAL;
		break;
	case ID_ID:
		V_NAME(v) = strdup(str);
		V_TYPE(v) = ID_UNK;
		break;
	}
	return (v);
}

int is_zero(Var* v)
{
	Var* e;

	if (v == NULL) return (1);

	if ((e = eval(v)) != NULL) v = e;

	if (V_TYPE(v) == ID_VAL && V_DSIZE(v) == 1) {
		switch (V_FORMAT(v)) {
		case DV_UINT8:
		case DV_UINT16:
		case DV_UINT32:
		case DV_UINT64:

		case DV_INT8:
		case DV_INT16:
		case DV_INT32:
		case DV_INT64: return (extract_i64(v, 0) == 0);

		case DV_FLOAT:
		case DV_DOUBLE: return (extract_float(v, 0) == 0.0);
		}
	} else {
		size_t i;
		// NOTE(rswinkle) This seems like yet another bad/wrong design decision.  In C, 0 is
		// the only false value, any set bit is true.  Likewise in any scripting language that I know
		// testing for "false"/0 on a sequence is true only if all elements are false 0 whereas any non-0
		// element makes the entire thing "true".  Maybe this gives some advantage for working on images but
		// it's unintuitive.
		for (i = 0; i < V_DSIZE(v); i++) {
			switch (V_FORMAT(v)) {
			case DV_UINT8:
			case DV_UINT16:
			case DV_UINT32:
			case DV_UINT64:

			case DV_INT8:
			case DV_INT16:
			case DV_INT32:
			case DV_INT64: if (extract_i64(v, i) == 0) return 1;

			case DV_FLOAT:
			case DV_DOUBLE: if (extract_float(v, i) == 0.0) return 1;
			}
		}
	}
	return (0);
}


static debug_print_var_type(int type)
{
static char* enum_strings[] = {
	"ID_NONE  = 0, /* a non value */",
	"ID_ERROR = 99,",
	"ID_BASE  = 100, /* in case of conflicts */",
	"ID_UNK,         /* Unknown type - also used as a generic type */",
	"ID_STRING,      /* NULL terminated character string */",
	"ID_KEYWORD,     /* keyword argument */",
	"ID_VAL,         /* everything with dim != 0 */",
	"ID_STRUCT,      /* Structure */",
	"ID_TEXT,        /*1-D Array of Strings*/",

	"ID_IVAL, /* Integer value */",
	"ID_RVAL, /* real value */",
	"ID_ID,   /* Identifier */",

	"ID_LIST,   ",
	"ID_IF"
	"ID_ELSE,   ",
	"ID_WHILE,  ",
	"ID_CONT,   ",
	"ID_BREAK,  ",
	"ID_RETURN, ",

	"ID_RANGES, /* list of ranges */",
	"ID_RSTEP,  /* list of ranges */",
	"ID_RANGE,  /* single range value */",
	"ID_SET,    /* assignment expression */",
	"ID_OR",
	"ID_AND",
	"ID_EQ",
	"ID_NE",
	"ID_LT",
	"ID_GT",
	"ID_LE",
	"ID_GE",
	"ID_ADD",
	"ID_SUB",
	"ID_MULT",
	"ID_DIV",
	"ID_MOD",
	"ID_UMINUS",
	"ID_LSHIFT",
	"ID_RSHIFT",
	"ID_FUNCT",
	"ID_ARRAY,  /* application of ranges to array */",

	// I think these descriptions are swapped based on evaluate()
	"ID_ARG,    /* list of arguments */",
	"ID_ARGS,   /* single argument */",

	"ID_FOR",
	"ID_FOREACH",
	"ID_EACH",

	"ID_ARGV,    /* $VALUE argument. Evalue at run */",

	"ID_INC,    /* increment value */",
	"ID_DEC,    /* decrement value */",
	"ID_INCSET, /* increment value */",
	"ID_DECSET, /* decrement value */",
	"ID_MULSET, /* *= value */",
	"ID_DIVSET, /* /= value */",

	"ID_POW,   /* exponent */",
	"ID_CAT,   /* concatenate */",
	"ID_ENUM,  /* enumerated argument, not parsed */",
	"ID_DECL,  /* Declaration */",
	"ID_WHERE, /* Where */",
	"ID_DEREF, /* Structure dereference */",

	"ID_CONSTRUCT,   /* Structure constructor */",
	"ID_DECONSTRUCT, /* Structure deconstructor */",

	"ONE_AXIS, /* argument options */",
	"ANY_AXIS, /* argument options */",

	"ID_LINE",

	"ID_MODULE,   /* davinci module variable ID */",
	"ID_FUNCTION, /* davinci module function variable ID */",
	"ID_PARALLEL, /* parallelization */",
	"ID_VARARGS,  /* varargs arguments */",

	"ID_FPTR /* a function pointer */",
	};

	if (type > 0)
		type -= ID_ERROR-1;

	puts(enum_strings[type]);
}

/*
define myfunc() {
 printf("argc = %d\n", $0)
 printf("argc = %d\n", $argc)

 l = $argv
 printf("l = %s %s\n", l)


 c = 5
 d = 6
 ls()

}

*/

/**
 ** evaluate() - Evaluate a parse tree
 **/

Var* evaluate(Var* n)
{
	Var *left, *right, *range, *where;
	Var *p1 = NULL, *p2 = NULL, *p3 = NULL, *p4 = NULL, *p5 = NULL;
	Scope* scope = scope_tos();
	int type;

	if (n == NULL) return (NULL);
	type = V_TYPE(n);

	// DEBUG(rswinkle)
#if 0
	//printf("evaluate scope_count = %d\n", scope_stack_count());
	debug_print_var_type(type);
#endif
	
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
	case ID_TEXT: /*Added: Thu Mar  2 16:00:18 MST 2000 */ push(scope, V_DUP(n)); return (NULL);
	}

	left  = V_NODE(n)->left;
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
	case ID_LINE: {
		if (debug && right) {
			printf("--> %s", V_STRING(right));
			fflush(stdout);
		}
		return (evaluate(left));
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
	case ID_LSHIFT:
	case ID_RSHIFT:
	case ID_MOD:
	case ID_POW:
		if (left) evaluate(left);
		if (right) evaluate(right);
		p2 = pop(scope);
		p1 = pop(scope);
		push(scope, pp_math(p1, type, p2));
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
			if ((p2 = eval(p1)) != NULL) {
				p1 = p2;
			}
			scope->rval                     = p1;
		}
		scope->returned = 1;
		break;

	case ID_BREAK: scope->broken = scope->loop; break;

	case ID_CONT: scope->broken = scope->loop + 1; break;

	case ID_FOR: /* just a special holder for while loops */
		if (left) evaluate(left);
		if (!scope->returned) cleanup(scope);
		break;

	case ID_WHILE:
		scope->loop++;
		while (1) {
			if (left != NULL) {
				evaluate(left);
				if (is_zero(pop(scope))) break;
			}
			scope->broken = 0;
			evaluate(right);
			if (scope->returned != 0 || (scope->broken != 0 && scope->broken <= scope->loop)) {
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
				n     = right;
				left  = V_NODE(n)->left;
				right = V_NODE(n)->right;
			}
			if (!scope->returned) cleanup(scope);
		}
		if (scope->broken == 0 && scope->returned == 0) evaluate(n);
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

		/*
		** If a variable is a module variable, we can have dereferences to
		** functions within it. An ordinary variable, however, cannot have
		** such a function dereference.
		*/
		if (V_TYPE(left) == ID_DEREF) {
#ifdef BUILD_MODULE_SUPPORT
			/* module dereference */
			evaluate(left);

			p2 = pop(scope);
			if (p2 && V_TYPE(p2) == ID_FUNCTION) {
				push(scope, pp_call_dv_module_func(V_FUNC(p2), p1));
			} else {
				parse_error("Function dereference allowed for module variables only.");
				push(scope, NULL);
			}
#else  /* no module support */
			parse_error("Function dereference allowed for module variables only.");
#endif /* BUILD_MODULE_SUPPORT */
		} else {
			push(scope, pp_func(left, p1));
		}

		scope->returned = 0;

		break;

	case ID_ARGS:
		if (left) evaluate(left);
		if (right) evaluate(right);
		if (right) p2 = pop(scope);
		if (left) p1  = pop(scope);
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

	case ID_RANGE: /* pair of range values */
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

	case ID_RSTEP: /* ranges, with a step value */
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
			push(scope, pp_add_range(p1, p2));
		}
		break;

	case ID_SET: /* equivalence */

		if (V_TYPE(left) == ID_ARRAY) {
			evaluate(V_NODE(left)->left);
			evaluate(V_NODE(left)->right);
			evaluate(right);

			p2    = pop(scope);
			range = pop(scope);
			p1    = pop(scope);

			push(scope, pp_set_var(p1, range, p2));
		} else if (V_TYPE(left) == ID_WHERE) {
			/*
			 *              ID_SET
			 *            /        \
			 *    ID_WHERE          EXPR
			 *   /        \
			 *  ID       VAL
			*/
			evaluate(V_NODE(left)->left);
			evaluate(V_NODE(left)->right);
			evaluate(right);

			p2    = pop(scope);
			where = pop(scope);
			p1    = pop(scope);
			push(scope, pp_set_where(p1, where, p2));

		} else if (V_TYPE(left) == ID_DEREF) {

			/*
			 *               ID_SET
			 *             /        \
			 *        ID_DEREF          EXPR
			 *      /        \
			 *     ID         ID
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
	/*
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
	*/
	case ID_INC: /* increment */
	case ID_DEC: /* decrement */
		         /* These are supposed to work just like equivalence,
		            but in this case, we want to extract the array,
		            do some math with it, and put it back
		         */
		if (V_TYPE(left) == ID_ARRAY) {
			evaluate(V_NODE(left)->left);
			evaluate(V_NODE(left)->right);
			evaluate(right);
			evaluate(left);

			p3    = pop(scope); /* get the actual subsetted data (left) */
			p2    = pop(scope); /* expression */
			range = pop(scope); /* range of destination */
			p1    = pop(scope); /* destination */

			if (type == ID_INC) p5 = pp_math(p3, ID_ADD, p2);
			if (type == ID_DEC) p5 = pp_math(p3, ID_SUB, p2);
			push(scope, pp_set_var(p1, range, p5));
		} else if (V_TYPE(left) == ID_WHERE) {
			/*
			 *              ID_SET
			 *            /        \
			 *    ID_WHERE          EXPR
			 *   /        \
			 *  ID       VAL
			*/
			evaluate(V_NODE(left)->left);
			evaluate(V_NODE(left)->right);
			evaluate(right);

			evaluate(V_NODE(left)->left);
			p3 = pop(scope);

			p2    = pop(scope);
			where = pop(scope);
			p1    = pop(scope);

			if (type == ID_INC) p5 = pp_math(p3, ID_ADD, p2);
			if (type == ID_DEC) p5 = pp_math(p3, ID_SUB, p2);
			push(scope, pp_set_where(p1, where, p5));
		} else if (V_TYPE(left) == ID_DEREF) {
			/*
			 *               ID_SET
			 *             /        \
			 *        ID_DEREF     EXPR
			 *      /        \
			 *     ID         ID
			*/
			evaluate(V_NODE(left)->left);
			evaluate(V_NODE(left)->right);
			evaluate(right);
			evaluate(left);

			p4 = pop(scope); /* structure.member */
			p3 = pop(scope); /* expr */
			p2 = pop(scope); /* member */
			p1 = pop(scope); /* structure */

			if (type == ID_INC) p5 = pp_math(p4, ID_ADD, p3);
			if (type == ID_DEC) p5 = pp_math(p4, ID_SUB, p3);
			push(scope, pp_set_struct(p1, p2, p5));
		} else {
			evaluate(left);
			evaluate(right);
			p2 = pop(scope);
			p1 = pop(scope);

			if (type == ID_INC) p5 = pp_math(p1, ID_ADD, p2);
			if (type == ID_DEC) p5 = pp_math(p1, ID_SUB, p2);
			push(scope, pp_set_var(p1, NULL, p5));
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
		}
#ifdef BUILD_MODULE_SUPPORT
		else if ((p3 = search_in_list_of_loaded_modules(V_NAME(p1))) != NULL) {
			vfuncptr t;
			// Variables added for module help
			int subtopics         = 0;
			int retval            = 0;
			char* input_line      = NULL;
			char module_help[256] = {0};
			char keyword[256]     = {0};
			char* tmp             = NULL;

			// Support for help
			if (strcmp((V_NAME(p2)), MODULE_HELP) == 0) {
				if ((tmp = getenv("DV_MOD_PATH"))) {
					strncpy(module_help, tmp, 256);
				} else { // use default
					sprintf(module_help, "/usr/lib/davinci/modules");
				}
				sprintf(module_help, "%s/%s.gih", module_help, V_NAME(p3));
				retval = help(V_NAME(p3), module_help, &subtopics);

				if (retval == H_FOUND) {
					input_line = cleanup_input(readline("Subtopic: "));

					if (strlen(input_line)) {
						subtopics = 0;
						sprintf(keyword, "%s %s", keyword, input_line);
						retval = help(keyword, module_help, &subtopics);
						// if (retval == H_FOUND) return(evaluate(n));
					}
				}
			} else if ((t = find_module_func(&V_MODULE(p3), V_NAME(p2))) != NULL) {
				p1                 = newVar();
				p1->value.function = t;
				p1->type           = ID_FUNCTION;

				/* push the function for the caller to grab */
				push(scope, p1);
			} else {
				parse_error("Module %s does not contain member: %s", V_NAME(p3),
				            PRINTABLE_STR(V_NAME(p2)));

				push(scope, NULL);
			}
		}
#endif /* BUILD_MODULE_SUPPORT */
		else {
			char* name = p2 ? V_NAME(p2) : NULL;
			if (find_struct(p1, name, &p3) != -1) {
				push(scope, p3);
			} else {
				parse_error("structure does not contain member: %s", V_NAME(p2));
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

	/*
	 * The PARALLEL construct @(), is a pseudo-function used by the
	 * function dispatcher to identify a section of code that can
	 * be performed in parallel.
	 *
	 *           PARALLEL
	 *           /     \
	 *         id      arg
	 *
	 * Use this node in the arg list
	 */

	case ID_PARALLEL:
		if (right) {
			Var* axis = NULL;
			evaluate(right);
			p1 = pop(scope);
			if (left) {
				evaluate(left);
				axis = pop(scope);
			}
			if (p1 == NULL) {
				return NULL;
			}
			p2 = pp_new_parallel(axis, p1);
			push(scope, p2);
		}
		break;

	default: fprintf(stderr, "Unknown type: %d\n", type);
	}
	return (NULL);
}

Var* p_lnode(Var* parent, Var* child)
{
	Var* p;

	for (p = parent; V_NODE(p)->left != NULL; p = V_NODE(p)->left)
		;

	V_NODE(p)->left = child;
	return (parent);
}

Var* p_rnode(Var* parent, Var* child)
{
	Var* p;

	for (p = parent; V_NODE(p)->right != NULL; p = V_NODE(p)->right)
		;

	V_NODE(p)->right = child;
	return (parent);
}

Var* p_rlist(int type, Var* list, Var* stmt)
{
	Var* p;
	if (stmt == NULL) return (list);
	if (list == NULL) return (stmt);
	if (V_TYPE(list) != type) return (p_mknod(type, list, stmt));
	for (p = list; V_TYPE(V_NODE(p)->right) == type; p = V_NODE(p)->right)
		;
	V_NODE(p)->right = p_mknod(type, V_NODE(p)->right, stmt);
	return (list);
}

Var* p_llist(int type, Var* list, Var* stmt)
{
	Var* p;
	if (stmt == NULL) return (list);
	if (list == NULL) return (stmt);
	if (V_TYPE(list) != type) return (p_mknod(type, stmt, list));
	for (p = list; V_TYPE(V_NODE(p)->left) == type; p = V_NODE(p)->left)
		;
	V_NODE(p)->left = p_mknod(type, stmt, V_NODE(p)->left);
	return (list);
}

/**
 ** Free all the memory allocated in a tree.  Do this like evaluate does.
 **/

void free_tree(Var* n)
{
	Var *left, *right;
	int type;

	while (1) {
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
		case ID_TEXT: /*Added: Thu Mar  2 16:01:19 MST 2000 */ free_var(n); return;
		}

		left  = V_NODE(n)->left;
		right = V_NODE(n)->right;

		if (left != NULL && left != right) free_tree(left);
		free(n);
		n = right;
	}
}
