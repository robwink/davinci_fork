#include "parser.h"
/**
 ** Split a buffer into individual words, at any whitespace or separator
 **/

void
split_string(char *buf, int *argc, char ***argv, char *s)
{
    char sep[256] = " \t\n\r";
    int size = 16;
    int count = 0;
    char *ptr = buf;
    char *p;
 
    if (s) strcat(sep, s);

    *argv = (char **)calloc(size, sizeof(char *));
    while((p = strtok(ptr, sep)) != NULL) {
        (*argv)[count++] = p;
        ptr = NULL;
        if (count == size) {
            size *= 2;
            *argv = (char **)my_realloc(*argv, size*sizeof(char *));
        }
    }
    *argc = count;
}

