#include "avl.h"

typedef struct _Darray {
    int size;
    int count;
    void **data;
} Darray;

typedef struct _Aarray {
    Darray *data;
    avl_tree *tree;
} Aarray;
