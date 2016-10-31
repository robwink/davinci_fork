#include "ff_source.h"
#include "cvector.h"
#include "parser.h"

// This file contains routines to handle pushing and popping of input files.

static cvector_void source_stack;

void init_input_stack()
{
	cvec_void(&source_stack, 0, 16, sizeof(Source), NULL, NULL);
}

void push_input_stream(FILE* fptr, char* filename)
{
	Source new_source = {fptr, ((filename) ? strdup(filename) : NULL), pp_line};
	cvec_push_void(&source_stack, &new_source);
	pp_line = 0;
}

void push_input_file(char* name)
{
	FILE* fptr = NULL;
	char* fname;

	if (name == NULL) {
		fptr = stdin;
	} else if ((fptr = fopen(name, "r")) == NULL) {
		if ((fname = dv_locate_file(name)) != NULL) {
			fptr = fopen(fname, "r");
			free(fname);
		}
		if (fptr == NULL) {
			sprintf(error_buf, "Could not source file: %s", name);
			parse_error(NULL);
			return;
		}
	}

	// TODO: name is probably not so good here.
	// We'd likely rather have fname
	push_input_stream(fptr, name);
}

// Close (pop) currently opened file, and return next one on stack.
void pop_input_file()
{
	Source src;
	cvec_pop_void(&source_stack, &src);

	pp_line = src.line;
	free(src.name);
	if (fileno(src.file) != 0) {
		fclose(src.file);
	}
}

int is_file(char* name)
{
	struct stat sbuf;

	if ((stat(name, &sbuf)) != 0) {
		return 0;
	}
	return 1;
}

int input_stack_size()
{
	return source_stack.size;
}

Source* top_input_source()
{
	if (source_stack.size) return cvec_back_void(&source_stack);
	return NULL;
}

FILE* top_input_file()
{
	if (source_stack.size) return ((Source*)cvec_back_void(&source_stack))->file;
	return NULL;
}

char* top_input_filename()
{
	if (source_stack.size) return ((Source*)cvec_back_void(&source_stack))->name;
	return NULL;
}

// ff_source() - Source a script file
//
// This function pushes another file onto the file stack.  It takes
// a single STRING arg.
Var* ff_source(vfuncptr func, Var* arg)
{
	char* p;
	char* filename = NULL;
	char* fname    = NULL;

	Alist alist[2];
	alist[0]      = make_alist("filename", ID_STRING, NULL, &filename);
	alist[1].name = NULL;

	if (parse_args(func, arg, alist) == 0) return NULL;

	if (filename == NULL) {
		parse_error("%s: No filename specified.", func->name);
		return NULL;
	}
	// remove extra quotes from string.
	p = filename;
	if (*p == '"') p++;
	if (strchr(p, '"')) *(strchr(p, '"')) = '\0';

	if ((fname = dv_locate_file(filename)) == NULL) {
		parse_error("Cannot find file: %s\n", filename);
		return NULL;
	}
	push_input_file(fname);
	free(fname);

	return NULL;
}
