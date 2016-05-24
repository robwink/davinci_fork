#ifndef FF_SOURCE_H
#define FF_SOURCE_H

#include <stdio.h>

typedef struct Source {
	FILE* file;
	char* name;
	int line;
} Source;

void push_input_file(char *name);
void push_input_stream(FILE *, char *filename);
void pop_input_file();
void init_input_stack();
int is_file(char *name);

Source* top_input_source();
FILE* top_input_file();
char* top_input_filename();
int input_stack_size();

#endif
