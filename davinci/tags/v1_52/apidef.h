#ifndef APIDEF_H
#define APIDEF_H

#include "parser.h"

#define VOID		9    

#define CONSTBIT        64
#define VOLBIT          128
#define PTRBIT          256

#define DTMASK		0x3F

typedef struct {
    char *argname;
    int argtype;
    void *argval;
    Var *argusp;
} APIARGS;

typedef struct {
    char *apiname;
    APIARGS *apiargs;
    int argc;
    void (*apifptr)(int,APIARGS *);
} APIDEFS;

APIDEFS *api_lookup(char *);
Var *dispatch_api(APIDEFS *,Var *);

#endif /* APIDEF_H */
