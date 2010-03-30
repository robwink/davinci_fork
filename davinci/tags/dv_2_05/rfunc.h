
typedef struct Args_registration ArgsRegister;
typedef Var *(*Rfunc)(ArgsRegister *, void *);

typedef struct _Args {
    char *name;			/* argument name */
    int type; 			/* type of argument */
    void *options;		/* list of options if constrained */
    size_t offset;		/* position in struct */
    int filled;			/* has this argument been filled */
} Args;


struct Args_registration {
    char *name;			/* name of function */
    Rfunc func;		/* function to call */
    Args *args;	/* array of args */
    int size;			/* how large a struct do we allocate */
    void *data;			/* any extra data to be passed */
};


Var * dispatch_rfunc(ArgsRegister *r, Var *arg) ;
void * MakeArgs(ArgsRegister *r, Var *arg);
