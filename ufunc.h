#ifndef UFUNC_H
#define UFUNC_H

#include "parser.h"

typedef struct {
	char* text;
	char* name;
	char* body;   /* body is part of f->text.  Dont free it */
	char* argbuf; /* holder for memory used in f->args */
	char** args;
	int nargs;
	int min_args;
	int max_args;
	int ready;
	Var* tree; /* code tree */
	char* fname;
	int fline;
	avl_node_t node;

} UFUNC;

void init_ufunc_tree();
void free_ufunc_tree();
UFUNC* load_function(char*);
Var* dispatch_ufunc(UFUNC*, Var*);
void free_ufunc(UFUNC*);

#endif /* UFUNC_H */
