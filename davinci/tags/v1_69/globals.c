/* 
 ** These symbols were moved from main.c in order to create
 ** a windows dll easier.
*/
#include <stdio.h>
#include <setjmp.h>
#include "parser.h"

int interactive = 1;
int continuation = 0;
int in_comment = 0;
int in_string = 0;

int debug = 0;
char pp_input_buf[8192];
FILE *lfile = NULL;
FILE *pfp = NULL;

int SCALE = 6;
int VERBOSE = 2;
int DEPTH = 2;

int allocs = 0;
Var *VZERO;

/**
 ** This is stuff from the old input.c
 **/
int pp_count = 0;
int pp_line = 0;
int indent = 0;


Var *curnode = NULL;

void quit(void);

void 
make_sym(Var * v, int format, char *str)
{
    V_FORMAT(v) = format;
    V_DSIZE(v) = 1;
    V_SIZE(v)[0] = V_SIZE(v)[1] = V_SIZE(v)[2] = 1;
    V_ORG(v) = BSQ;
    V_DATA(v) = calloc(1, NBYTES(format));

    switch (format) {
    case INT:
        *((int *) (V_DATA(v))) = strtol(str, NULL, 10);
        break;
    case FLOAT:{
        double d;
        d = strtod(str, NULL);
        if (d > MAXFLOAT || d < MINFLOAT && d != 0) {
            free(V_DATA(v));
            V_DATA(v) = calloc(1, NBYTES(DOUBLE));
            V_FORMAT(v) = DOUBLE;
            *((double *) (V_DATA(v))) = d;
        } else {
            *((float *) (V_DATA(v))) = d;
        }
    }
    }
}


/*
** This is similar to parse_buffer, but it doesn't print the stack
** or clean up the scope
*/
Var *
eval_buffer(char *buf)
{
    int i,j;
    extern char *yytext;
    Var *v = NULL;
    void *parent_buffer;
    void *buffer;
    Var *node;

    parent_buffer = (void *) get_current_buffer();
    buffer = (void *) yy_scan_string(buf);

    while((i = yylex()) != 0) {
        /*
        ** if this is a function definition, do no further parsing yet.
        */
        j = yyparse(i, (Var *)yytext);
        if (j == -1) quit();

        if (j == 1 && curnode != NULL) {
            node = curnode;
            evaluate(node);
            /* // v = pop(scope_tos()); */
            free_tree(node);
        }
    }

    yy_delete_buffer((struct yy_buffer_state *)buffer);
    if (parent_buffer) yy_switch_to_buffer(parent_buffer);
    return(v);
}


void
quit(void)
{
    char cmd[256];
    char *path = getenv("TMPDIR");

    if (interactive) {
        printf("\n");
#if defined(USE_X11_EVENTS) && defined(HAVE_LIBREADLINE)
    /* JAS FIX */
        rl_callback_handler_remove();
#endif
    }

    /**
    ** clean up temporary directory
    **/
    rmrf(path);
    exit(0);
}


void 
yyerror(char *s)
{
    extern int pp_count;
    extern int pp_line;

    printf("***%*s^ ", pp_count, " ");
    printf("%s, line %d\n", s, pp_line);
}

int 
yywrap()
{
    return (1);
}



char *
unquote(char *name)
{
    char *p = name;

    if (name == NULL) return(NULL);
    if (*name == 0) return(name);
    if (*name == '"') {
        p++;
        name++;
        while (*name) {
            if (*name == '"' && *(name - 1) != '\\')
                break;
            name++;
        }
        *name = '\0';
    } else if (*name == '\'') {
        p++;
        name++;
        while (*name) {
            if (*name == '\'' && *(name - 1) != '\\')
                break;
            name++;
        }
        *name = '\0';
    }
    return (p);
}

char *
unescape(char *str)
{
    char *p = str;
    char *q = str;

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

