#include <stddef.h>
#include <string.h>

/*
** Retruns VALUE from a string of the form:
**
**       KEYWORD=VALUE
**
** where:
**   s1 is of the form "KEYWORD="
**   s2 is the string
*/

char *
get_value(char *s1, char *s2)
{
    char *p;
    int len;

    len = strlen(s2);
    for (p = s1 ; p && *p ; p++) {
        if (!strncasecmp(p, s2, len)) {
            return(p+len);
        }
    }
    return(NULL);
}

