#ifndef STRUCT_H
#define STRUCT_H

#include "parser_types.h"

// maybe rename this struct.h and make struct.c?

Var* new_struct(int ac);
void add_struct(Var* s, const char* name, Var* exp);
Var* remove_struct(Var*, int);
int find_struct(Var*, const char* , Var**);
void free_struct(Var*);
Var* create_struct(Var* v);;

int get_struct_element(const Var* v, const int i, char** name, Var** data);
int get_struct_count(const Var* v);

Var* duplicate_struct(Var* v);

#endif
