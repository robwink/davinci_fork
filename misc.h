#ifndef MISC_H
#define MISC_H

void split_string(char* buf, int* argc, char*** argv, char* s);
char* uppercase(char* s);
char* lowercase(char* s);
char* ltrim(char* s, const char* trim_chars);
char* rtrim(char* s, const char* trim_chars);


char* fix_name(const char* input_name);


//moved from util.c
char* get_value(char* s1, char* s2);


//move from globals.c
void quit(int return_code);
void make_sym(Var *, int, char *);
void yyerror(char *s);
int yywrap();
char *unquote(char *);
char* unescape(char* str);


// moved from newfunc.c
int parse_args(vfuncptr name, Var* args, Alist* alist);
int make_args(int* ac, Var*** av, vfuncptr func, Var* args);
Alist make_alist(const char* name, int type, void* limits, void* value);
Var* append_arg(Var* args, char* key, Var* val);
Var* create_args(int ac, ...);

#endif
