#include "parser.h"
#include "func.h"

#include "hdf5.h"



Var *
make_struct(int ac, Var **av)
{
    Var *o;
    char **names;
    Var **data;
    int i;
    char *zero;

    o = new(Var);
    V_TYPE(o) = ID_VSTRUCT;
    V_STRUCT(o).count = ac;
    names = V_STRUCT(o).names = calloc(ac, sizeof(char *));
    data = V_STRUCT(o).data = calloc(ac, sizeof(Var *));

    for (i = 0 ; i < ac ; i++) {
        zero = calloc(1,1);
        /*
        ** check for duplicate names here
        */
        names[i] = strdup(V_NAME(av[i]));
        data[i] = newVal(BSQ, 1,1,1, BYTE, zero);
        mem_claim(data[i]);
    }
    return(o);
}

Var *
ff_struct(vfuncptr func, Var * arg)
{
    Var *obj = NULL, *compress = NULL, *normalize = NULL;
    int type = 0;
    int x,y,z, n, i, j, dsize, low, high, v;
    int *data;
    char *ptr = NULL;

    int ac;
    Var **av;

    make_args(&ac, &av, func, arg);
    return(make_struct(ac-1, av+1));
}

WriteHDF5(hid_t parent, char *name, Var *v)
{
    hid_t dataset, datatype, dataspace, aid2, attr, child;
    hsize_t size[3];
    int org;
    int top = 0;
    int i;
	Var *e;

    if (V_NAME(v) != NULL) {
        if ((e = eval(v)) != NULL) v = e;
    }
    if (v == NULL || V_TYPE(v) == ID_UNK) {
        parse_error("Can't find variable");
        return(NULL);
    }
        

    if (parent < 0) {
        top = 1;
        parent = H5Fcreate(name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }

    switch (V_TYPE(v)) {
    case ID_VSTRUCT:

        /*
        ** member is a structure.  We need to create a sub-group
        ** with this name.
        */
		if (top == 0) {
			child = H5Gcreate(parent, name, 0);
		} else {
			child = parent;
		}
        for (i = 0 ; i < V_STRUCT(v).count ; i++) {
            WriteHDF5(child, V_STRUCT(v).names[i], V_STRUCT(v).data[i]);
        }
        if (top == 0) H5Gclose(child);
        break;

    case ID_VAL:

        /*
        ** member is a value.  Create a dataset
        */
        for (i = 0 ; i < 3 ; i++) {
            size[i] = V_SIZE(v)[i];
        }
        org = V_ORG(v);
        dataspace = H5Screate_simple(3, size, NULL);
        switch(V_FORMAT(v)) {
        case BYTE:  datatype = H5Tcopy(H5T_NATIVE_UCHAR); break;
        case SHORT: datatype = H5Tcopy(H5T_NATIVE_SHORT); break;
        case INT:   datatype = H5Tcopy(H5T_NATIVE_INT); break;
        case FLOAT: datatype = H5Tcopy(H5T_NATIVE_FLOAT); break;
        case DOUBLE: datatype = H5Tcopy(H5T_NATIVE_DOUBLE); break;
        }
        dataset = H5Dcreate(parent, name, datatype, dataspace, H5P_DEFAULT);
        H5Dwrite(dataset, datatype, 
                 H5S_ALL, H5S_ALL, H5P_DEFAULT, V_DATA(v));
        aid2 = H5Screate(H5S_SCALAR);
        attr = H5Acreate(dataset, "org", H5T_NATIVE_INT, aid2, H5P_DEFAULT);

        H5Awrite(attr, H5T_NATIVE_INT, &org);
        H5Sclose(aid2);
        H5Aclose(attr);

        H5Tclose(datatype);
        H5Sclose(dataspace);
        H5Dclose(dataset);
        break;

    }
    if (top) H5Fclose(parent);
    return(NULL);
}

Var *
LoadHDF5(char *filename)
{
}
