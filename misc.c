#include "parser.h"

// for when/if I pull globals.h out of parser.h
// #include "globals.h"



// NOTE(rswinkle) Organization (and design) is davinci's biggest problem.  This file
// is now the collection for definitions that I haven't decided where they belong.
//
// The string functions could go in ff_text.c or maybe string.c; probably string.c should
// be merged into ff_text.c or vice versa



//TODO(rswinkle) use cvector_str for argv
//NOTE only used once in ufunc.c ... move there?
// Split a buffer into individual words, at any whitespace or separator
void split_string(char* buf, int* argc, char*** argv, char* s)
{
	char sep[256] = " \t\n\r";
	int size      = 16;
	int count     = 0;
	char* ptr     = buf;
	char* p;

	if (s) strcat(sep, s);

	*argv = (char**)calloc(size, sizeof(char*));
	while ((p = strtok(ptr, sep)) != NULL) {
		(*argv)[count++] = p;
		ptr              = NULL;
		if (count == size) {
			size *= 2;
			*argv = (char**)my_realloc(*argv, size * sizeof(char*));
		}
	}
	*argc = count;
}

char* uppercase(char* s)
{
	char* p;
	for (p = s; p && *p; p++) {
		if (islower(*p)) *p = *p - 'a' + 'A';
	}
	return (s);
}

char* lowercase(char* s)
{
	char* p;
	for (p = s; p && *p; p++) {
		if (isupper(*p)) *p = *p - 'A' + 'a';
	}
	return (s);
}

char* ltrim(char* s, const char* trim_chars)
{
	int st = 0;
	while (s[st] && strchr(trim_chars, s[st])) st++;

	if (st > 0) memmove(s, &s[st], strlen(&s[st]) + 1);

	return s;
}

char* rtrim(char* s, const char* trim_chars)
{
	int len = strlen(s);

	while (len > 0 && strchr(trim_chars, s[len - 1])) s[--len] = '\0';

	return s;
}

// TODO(rswinkle) this leaks almost everywhere it's used because people overwrite the original
// value. Can we not modify in place?
// my version is in dvio_hdf.c:make_valid_identifier()
char* fix_name(const char* input_name)
{
	const char invalid_pfx[] = "__invalid";
	static int invalid_id    = 0;
	char* name;
	int len;
	int i;
	int val;
	const char* trim_chars = "\"\t ";

	name = (char*)calloc(1, strlen(input_name) + 2);
	strcpy(name, input_name);

	ltrim(name, trim_chars);
	rtrim(name, trim_chars);

	len = strlen(name);
	if (len < 1) {
		free(name);
		name = (char*)calloc(strlen(invalid_pfx) + 12, sizeof(char));
		sprintf(name, "%s_%d", invalid_pfx, ++invalid_id);
		return name;
	}

	for (i = 0; i < len; i++) {
		name[i] = isalnum(name[i]) ? tolower(name[i]) : '_';
	}

	if (isdigit(name[0])) {
		for (i = len; i >= 0; i--) {
			name[i + 1] = name[i];
		}
		name[0] = '_';
	}

	return (name);
}


// moved from util.c (seriously a file with a single 12 line function?)
/*
** Retruns VALUE from a string of the form:
**
**       KEYWORD=VALUE
**
** where:
**   s1 is of the form "KEYWORD="
**   s2 is the string
*/

char* get_value(char* s1, char* s2)
{
	char* p;
	int len;

	len = strlen(s2);
	for (p = s1; p && *p; p++) {
		if (!strncasecmp(p, s2, len)) {
			return (p + len);
		}
	}
	return (NULL);
}


// Moved from globals.c
void make_sym(Var* v, int format, char* str)
{
	double d;

	V_FORMAT(v)  = format;
	V_DSIZE(v)   = 1;
	V_SIZE(v)[0] = V_SIZE(v)[1] = V_SIZE(v)[2] = 1;
	V_ORG(v)                                   = BSQ;
	V_DATA(v)                                  = calloc(1, NBYTES(format));

	// TODO(rswinkle) I think we should just default to i64 and double
	// TODO FIX, though it's very unlikely I think this could write out of bounds
	// if you allocated enough space for a smaller format, and then write as if its
	// a larger format ...
	switch (format) {
	case DV_UINT8:
	case DV_UINT16:
	case DV_UINT32:
	case DV_UINT64: *((u64*)(V_DATA(v))) = strtoull(str, NULL, 10); break;

	case DV_INT8:
	case DV_INT16:
	case DV_INT32: *((i32*)(V_DATA(v))) = strtol(str, NULL, 10); break;
	case DV_INT64: *((i64*)(V_DATA(v))) = strtoll(str, NULL, 10); break;

	case DV_FLOAT:
		d = strtod(str, NULL);
		if (((d > FLT_MAX) || (d < FLT_MIN)) && (d != 0)) {
			free(V_DATA(v));
			V_DATA(v)               = calloc(1, NBYTES(DV_DOUBLE));
			V_FORMAT(v)             = DV_DOUBLE;
			*((double*)(V_DATA(v))) = d;
		} else {
			*((float*)(V_DATA(v))) = d;
		}
		break;
	case DV_DOUBLE:
		d = strtod(str, NULL);
		*((double*)(V_DATA(v))) = d;
		break;
	}
}

void quit(int return_code)
{
	char* path = getenv("TMPDIR");

	if (interactive) {
		printf("\n");
#if defined(USE_X11_EVENTS) && defined(HAVE_LIBREADLINE)
		/* JAS FIX */
		rl_callback_handler_remove();
#endif
	}

	// Could free memory here (like scope_stack etc.) but
	// what's the point?
	// It's only a memory leak if forget to free something
	// while running or worse lose access to the pointer entirely.

	// clean up temporary directory
	rmrf(path);
	exit(return_code);
}

/*
** This is similar to parse_buffer, but it doesn't print the stack
** or clean up the scope
*/
Var* eval_buffer(char* buf)
{
	int i, j;
	extern char* yytext;
	Var* v = NULL;
	void* parent_buffer;
	void* buffer;
	Var* node;

	extern void* get_current_buffer();
	extern void* yy_scan_string();
	extern void yy_delete_buffer(void*);
	extern void yy_switch_to_buffer(void*);

	parent_buffer = (void*)get_current_buffer();
	buffer        = (void*)yy_scan_string(buf);

	while ((i = yylex()) != 0) {
		/*
		** if this is a function definition, do no further parsing yet.
		*/
		j = yyparse(i, (Var*)yytext);
		if (j == -1) quit(0);

		if (j == 1 && curnode != NULL) {
			node = curnode;
			evaluate(node);
			/* // v = pop(scope_tos()); */
			free_tree(node);
		}
	}

	yy_delete_buffer((struct yy_buffer_state*)buffer);
	if (parent_buffer) yy_switch_to_buffer(parent_buffer);
	return (v);
}

void yyerror(char* s)
{
	extern int pp_count;
	extern int pp_line;

	printf("***%*s^ ", pp_count, " ");
	printf("%s, line %d\n", s, pp_line);
}

int yywrap()
{
	return (1);
}

char* unquote(char* name)
{
	char* p = name;

	if (name == NULL) return (NULL);
	if (*name == 0) return (name);
	if (*name == '"') {
		p++;
		name++;
		while (*name) {
			if (*name == '"' && *(name - 1) != '\\') break;
			name++;
		}
		*name = '\0';
	} else if (*name == '\'') {
		p++;
		name++;
		while (*name) {
			if (*name == '\'' && *(name - 1) != '\\') break;
			name++;
		}
		*name = '\0';
	}
	return (p);
}

char* unescape(char* str)
{
	char* p = str;
	char* q = str;

	if (str && *str) {
		while (*p) {
			if (*p == '\\') {
				if (*(p + 1) == 't')
					*q = '\t';
				else if (*(p + 1) == 'n')
					*q = '\n';
				else
					*q = *(p + 1);
				p++;
			} else {
				*q = *p;
			}
			p++;
			q++;
		}
		*q = *p;
	}
	return (str);
}


// Moved from newfunc.c


// verify that the arguments are of the expected type
// How do we get av[0] installed?
int parse_args(vfuncptr name, Var* args, Alist* alist)
{
	int i, j, k, count;
	Var *v, *e;
	char* fname;
	char* ptr;
	int len;

	int ac;
	Var** av;

	make_args(&ac, &av, name, args);
	fname = (char*)av[0];

	for (i = 1; i < ac; i++) {
		v = av[i];
		if (v == NULL) continue;
		if (V_TYPE(v) == ID_KEYWORD && V_NAME(v) != NULL) {
			ptr = V_NAME(v);
			len = strlen(ptr);
			// set up error_buf, in case more than one keyword matches
			sprintf(error_buf, "Non-unique keyword match: %s(...%s=...)\nPossible matches:\n", fname, ptr);

			for (count = k = 0; alist[k].name != NULL; k++) {
				if (!strncasecmp(alist[k].name, ptr, len)) {
					// Append to error buf.  Again, just in case.
					sprintf(error_buf + strlen(error_buf), "\t%s\n", alist[k].name);
					count++;
					j = k;
				}
			}
			if (count == 0) {
				parse_error("Unknown keyword to function: %s(... %s= ...)", fname, ptr);
				free(av);
				return 0;
			}
			if (count > 1) {
				parse_error(NULL);
				free(av);
				return 0;
			}
			// Get just the argument part
			v = V_KEYVAL(v);
		} else {
			/*
			 * Unnamed arguments.
			 *
			 * special case created by create_args
			 * where we have a keyword, with no name.
			 * For the life of me, I don't recall how this came to be.
			 */
			if (V_TYPE(v) == ID_KEYWORD) {
				v = V_KEYVAL(v);
			}
			for (j = 0; alist[j].name != NULL; j++) {
				if (alist[j].filled == 0) break;
			}
			if (alist[j].name == NULL) {
				parse_error("Too many arguments to function: %s()", fname);
				free(av);
				return 0;
			}
		}

		// putting av[i] into alist[j]
		if (v == NULL) {
			parse_error("Illegal argument to function %s()\n", fname);
			return 0;
		}

		if (alist[j].type == ID_STRING) {
			char** p;
			if ((e = eval(v)) == NULL) {
				parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
				return 0;
			}
			v = e;
			if (V_TYPE(v) != ID_STRING) {
				parse_error("Illegal argument to function %s(), expected STRING", fname);
				free(av);
				return 0;
			}
			p               = (char**)(alist[j].value);
			*p              = V_STRING(v);
			alist[j].filled = 1;
		} else if (alist[j].type == ID_TEXT) {
			// This works just like string, but should also allow the
			// user to pass just a string, which can get promoted.
			Var** vptr;
			if ((e = eval(v)) == NULL) {
				parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
				return 0;
			}
			v = e;
			if (V_TYPE(v) != ID_TEXT && V_TYPE(v) != ID_STRING) {
				parse_error("Illegal argument to function %s(), expected STRING", fname);
				free(av);
				return 0;
			}

			if (V_TYPE(v) == ID_STRING) {
				char** s = (char**)calloc(1, sizeof(char*));
				s[0]     = strdup(V_STRING(v));
				v        = newText(1, s);
			}
			vptr            = (Var**)(alist[j].value);
			*vptr           = v;
			alist[j].filled = 1;
		} else if (alist[j].type == ID_VAL) {
			Var** vptr;
			if ((e = eval(v)) == NULL) {
				parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
				return 0;
			}
			v = e;
			if (V_TYPE(v) != ID_VAL) {
				parse_error("Illegal argument %s(...%s=...), expected VAL", fname, alist[j].name);
				free(av);
				return 0;
			}
			vptr            = (Var**)(alist[j].value);
			*vptr           = v;
			alist[j].filled = 1;
		} else if (alist[j].type == ID_STRUCT) {
			Var** vptr;
			if ((e = eval(v)) == NULL) {
				parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
				return 0;
			}
			v = e;
			if (V_TYPE(v) != ID_STRUCT) {
				parse_error("Illegal argument %s(...%s=...), expected STRUCT", fname, alist[j].name);
				free(av);
				return 0;
			}
			vptr            = (Var**)(alist[j].value);
			*vptr           = v;
			alist[j].filled = 1;


		// NOTE(rswinkle) this works because the integer types are grouped together
		// *and* the davinci type enum doesn't overlap,
		//
		// ID_NONE  = 0, /* a non value */
		// ID_ERROR = 99,
		// ID_BASE  = 100, /* in case of conflicts */
		// ID_UNK,         /* Unknown type - also used as a generic type */
		// ...
		// etc.
		// though of course we could just shift the starting point to of the numeric
		// enums if we needed to
		} else if (alist[j].type >= DV_UINT8 && alist[j].type <= DV_INT64) {

			// NOTE(rswinkle) previously it only did i32 and all non-real/OBJ args were i32
			// (including those specified as BOOL/BOOLEAN in docs) because it was the largest
			// int type available it could handle anything
			//
			// I am going to handle everything even though it's overkill because I might as well.  Really
			// You could probably just add i64 (and maybe u32 and u64)
			// I'm not going to change every davinci function to use DV_INT64's instead though someone might
			// want to eventually.
			u8* u8ptr = alist[j].value;
			u16* u16ptr = alist[j].value;
			u32* u32ptr = alist[j].value;
			u64* u64ptr = alist[j].value;

			i8* i8ptr = alist[j].value;
			i16* i16ptr = alist[j].value;
			i32* i32ptr = alist[j].value;
			i64* i64ptr = alist[j].value;

			if ((e = eval(v)) == NULL) {
				parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
				return 0;
			}
			v = e;
			if (V_TYPE(v) != ID_VAL || V_FORMAT(v) > DV_INT64) {
				parse_error("Illegal argument %s(...%s=...), expected integer type", fname, alist[j].name);
				free(av);
				return 0;
			}

			// TODO(rswinkle) use extract_int and arch based macros? see parser.h
			switch (alist[j].type) {
			case DV_UINT8: *u8ptr = extract_u64(v, 0); break;
			case DV_UINT16: *u16ptr = extract_u64(v, 0); break;
			case DV_UINT32: *u32ptr = extract_u64(v, 0); break;
			case DV_UINT64: *u64ptr = extract_u64(v, 0); break;

			case DV_INT8: *i8ptr = extract_i64(v, 0); break;
			case DV_INT16: *i16ptr = extract_i64(v, 0); break;
			case DV_INT32: *i32ptr = extract_i64(v, 0); break;
			case DV_INT64: *i64ptr = extract_i64(v, 0); break;
			}
			
			alist[j].filled = 1;
		} else if (alist[j].type == DV_FLOAT) {
			float* fptr;
			if ((e = eval(v)) == NULL) {
				parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
				return 0;
			}
			v = e;
			if (V_TYPE(v) != ID_VAL) {
				parse_error("Illegal argument %s(...%s=...), expected DV_FLOAT", fname, alist[j].name);
				free(av);
				return 0;
			}
			fptr            = (float*)(alist[j].value);
			*fptr           = extract_float(v, 0);
			alist[j].filled = 1;
		} else if (alist[j].type == DV_DOUBLE) {
			double* fptr;
			if ((e = eval(v)) == NULL) {
				parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
				return 0;
			}
			v = e;
			if (V_TYPE(v) != ID_VAL) {
				parse_error("Illegal argument %s(...%s=...), expected DV_DOUBLE", fname, alist[j].name);
				free(av);
				return 0;
			}
			fptr            = (double*)(alist[j].value);
			*fptr           = extract_double(v, 0);
			alist[j].filled = 1;
		} else if (alist[j].type == ID_ENUM) {
			char **p, *q = NULL, *ptr;
			char** values = (char**)alist[j].limits;

			if (V_TYPE(v) == ID_STRING)
				ptr = V_STRING(v);
			else
				ptr = V_NAME(v);

			// This is in case we don't know what we're looking for
			if (values == NULL) {
				if (ptr != NULL) {
					alist[j].filled = 1;
					p               = (char**)(alist[j].value);
					*p              = ptr;
				}
			} else {
				// try once with, hopefully, a passed string
				for (p = values; p && *p; p++) {
					if (ptr && !strcasecmp(ptr, *p)) {
						q = *p;
						break;
					}
				}
				if (q == NULL) {
					if ((e = eval(v)) != NULL) {
						if (V_TYPE(e) == ID_STRING) {
							ptr = V_STRING(e);
							for (p = values; p && *p; p++) {
								if (ptr && !strcasecmp(ptr, *p)) {
									q = *p;
									break;
								}
							}
						}
					}
				}
				if (q == NULL) {
					parse_error("Illegal argument to function %s(...%s...)", fname, alist[j].name);
					free(av);
					return 0;
				}

				p               = (char**)(alist[j].value);
				*p              = q;
				alist[j].filled = 1;
			}
		} else if (alist[j].type == ID_UNK) {
			Var** vptr;
			if ((e = eval(v)) == NULL) {
				parse_error("%s: Variable not found: %s", fname, V_NAME(v));
				free(av);
				return 0;
			}
			v               = e;
			vptr            = (Var**)(alist[j].value);
			*vptr           = v;
			alist[j].filled = 1;
		} else if (alist[j].type == ONE_AXIS) {
			int _t;
			// Shorthand for enumerated axis.  One of "x", "y" or "z"
			char **p, *q = NULL, *ptr;
			const char* values[] = {"x", "y", "z"};

			if (V_TYPE(v) == ID_STRING)
				ptr = V_STRING(v);
			else
				ptr = V_NAME(v);

			for (_t = 0; _t < 2; _t++) {
				if (ptr) {
					for (p = (char**)values; p && *p; p++) {
						if (ptr && !strcasecmp(ptr, *p)) {
							q = *p;
							break;
						}
					}
				}
				if ((e = eval(v)) == NULL || (V_TYPE(e) != ID_STRING)) {
					break;
				}
				ptr = V_STRING(e);
			}

			if (q == NULL) {
				parse_error("Illegal argument to function %s(...%s...)", fname, alist[j].name);
				free(av);
				return 0;
			}

			p               = (char**)(alist[j].value);
			*p              = q;
			alist[j].filled = 1;
		} else if (alist[j].type == ANY_AXIS) {
			; // nothing here on purpose
		} else {
			fprintf(stderr, "parse_args: Bad programmer, no biscuit.\n");
		}
	}

	free(av);
	return ac;
}

int make_args(int* ac, Var*** av, vfuncptr func, Var* args)
{
	int count = 0, i = 0;
	Var* p;

	if (args != NULL) {
		count = Narray_count(V_ARGS(args));
	}
	*av = (Var**)calloc(count + 2, sizeof(Var*));

	if (func) (*av)[0] = (Var*)func->name;
	for (i = 0; i < count; i++) {
		Narray_get(V_ARGS(args), i, NULL, (void**)&p);
		if (V_TYPE(p) == ID_KEYWORD && V_NAME(p) == NULL) {
			p = V_KEYVAL(p);
		}
		(*av)[i + 1] = p;
	}
	*ac = count + 1;
	return 0;
}

Alist make_alist(const char* name, int type, void* limits, void* value)
{
	Alist a;
	a.name   = name;
	a.type   = type;
	a.limits = limits;
	a.value  = value;
	a.filled = 0;
	return a;
}

Var* append_arg(Var* args, char* key, Var* val)
{
	Var* v;

	v         = newVar();
	V_NAME(v) = (key ? strdup(key) : NULL);
	args      = pp_mk_arglist(args, pp_keyword_to_arg(v, val));

	return args;
}

#include <stdarg.h>

Var* create_args(int ac, ...)
{
	Var* args = NULL;
	va_list ap;

	char* key;
	Var* val;

	va_start(ap, ac);
	while (1) {
		key = va_arg(ap, char*);
		val = va_arg(ap, Var*);
		if (val == NULL) {
			va_end(ap);
			break;
		}
		args = append_arg(args, key, val);
	}
	va_end(ap);
	return args;
}

int dv_format_size(int type)
{
	switch (type) {
	case DV_UINT8:
	case DV_INT8:
		return 1;

	case DV_UINT16:
	case DV_INT16:
		return 2;

	case DV_UINT32:
	case DV_INT32:

	// should be IEEE-754 so 4 bytes
	case DV_FLOAT:
		return 4;

	case DV_UINT64:
	case DV_INT64:
		return 8;

	case DV_DOUBLE:
		return sizeof(double);
	default:
	    // unsupported type
		fprintf(stderr, "unknown format: %d\n", type);
		return 0;
	}
}


const char* dv_format_to_str(int type)
{
	switch (type) {
	case DV_UINT8:  return "uint8";
	case DV_UINT16: return "uint16";
	case DV_UINT32: return "uint32";
	case DV_UINT64: return "uint64";

	case DV_INT8:  return "int8";
	case DV_INT16: return "int16";
	case DV_INT32: return "int32";
	case DV_INT64: return "int64";

	case DV_FLOAT:  return "float";
	case DV_DOUBLE: return "double";

/* deprecated
	case VAX_FLOAT:
		return "vax_float"
	case VAX_INTEGER:
		return "vax_int"
*/
	default:
		parse_error("Unknown format");
		return NULL;
	}
}

int dv_str_to_format(const char* str)
{
	int format = -1;

	for (int i=0; i<sizeof(FORMAT_STRINGS)/sizeof(char*); ++i) {
		if (!strcasecmp(FORMAT_STRINGS[i], str))
			return STR_TO_FORMAT[i];
	}

	return format;
}


// Return smallest format that can contain the values
// of the formats of both v1 and v2
int combine_var_formats(Var* v1, Var* v2)
{
	return combine_formats(V_FORMAT(v1), V_FORMAT(v2));
}

int combine_formats(int format1, int format2)
{
	int out_format = format1;

	// I think I made the assumption below that they were
	// not equal so I left out some cases
	if (format1 == format2)
		return out_format;

	switch (format1) {
	case DV_UINT8:
		if (format2 == DV_INT8) {
			out_format = DV_INT16;
		} else {
			out_format = format2;
		}
		break;
	case DV_UINT16:
		switch (format2) {
		case DV_UINT8:
			out_format = DV_UINT16;
			break;
		case DV_INT8:
		case DV_INT16:
			out_format = DV_INT32;
			break;
		default:
			out_format = format2;
		}
		break;
		
	case DV_UINT32:
		switch (format2) {
		case DV_UINT8:
		case DV_UINT16:
			out_format = DV_UINT32;
			break;
		case DV_INT8:
		case DV_INT16:
		case DV_INT32:
			out_format = DV_INT64;
			break;
		case DV_FLOAT:
			out_format = DV_DOUBLE;
			break;
		default:
			out_format = format2;
		}
		break;
	case DV_UINT64:
		switch (format2) {
		case DV_UINT8:
		case DV_UINT16:
		case DV_UINT32:
			out_format = DV_UINT64;
			break;
		default:
			out_format = DV_DOUBLE;
		}
		break;

	case DV_INT8:
		switch (format2) {
		case DV_UINT8:
			out_format = DV_INT16;
			break;
		case DV_UINT16:
			out_format = DV_INT32;
			break;
		case DV_UINT32:
			out_format = DV_INT64;
			break;
		case DV_UINT64:
			out_format = DV_DOUBLE;
			break;

		default:
			out_format = format2;
		}
		break;

	case DV_INT16:
		switch (format2) {
		case DV_UINT8:
			out_format = DV_INT16;
			break;
		case DV_UINT16:
			out_format = DV_INT32;
			break;
		case DV_UINT32:
			out_format = DV_INT64;
			break;
		case DV_UINT64:
			out_format = DV_DOUBLE;
			break;

		case DV_INT8:
			out_format = DV_INT16;
			break;

		default:
			out_format = format2;
		}
		break;

	case DV_INT32:
		switch (format2) {
		case DV_UINT8:
		case DV_UINT16:
			out_format = DV_INT32;
			break;
		case DV_UINT32:
			out_format = DV_INT64;
			break;
		case DV_UINT64:
			out_format = DV_DOUBLE;
			break;

		case DV_INT8:
		case DV_INT16:
			out_format = DV_INT32;
			break;

		case DV_FLOAT:
			out_format = DV_DOUBLE;
			break;

		default:
			out_format = format2;
		}
		break;

	case DV_INT64:
		switch (format2) {
		case DV_UINT8:
		case DV_UINT16:
		case DV_UINT32:
			out_format = DV_INT64;
			break;
		case DV_UINT64:
			out_format = DV_DOUBLE;
			break;

		case DV_INT8:
		case DV_INT16:
		case DV_INT32:
			out_format = DV_INT64;
			break;

		case DV_FLOAT:
			out_format = DV_DOUBLE;
			break;

		default:
			out_format = format2;
		}
		break;

	case DV_FLOAT:
		switch (format2) {
		case DV_UINT32:
		case DV_UINT64:
		case DV_INT32:
		case DV_INT64:
		case DV_DOUBLE:
			out_format = DV_DOUBLE;
			break;
		default:
			out_format = DV_FLOAT;
			break;
		}
		break;

	case DV_DOUBLE:
		out_format = DV_DOUBLE;
		break;
	}

	return out_format;

}

