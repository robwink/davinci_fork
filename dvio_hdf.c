#include "parser.h"
#include "func.h"
#include <sys/stat.h>

#ifdef HAVE_LIBHDF5

#include <hdf5.h>
Var *load_hdf5(hid_t parent);

#define intswap(i) (i) = ((((i) >> 24) & 0xFF) | (((i) >> 8) & 0xFF00) | \
	       (((i) << 8) &0xFF0000) | (((i) << 24) & 0xFF000000))

char * flip_endian(unsigned char * data, unsigned int data_elem,
		   unsigned int word_size) {
  /* Ensure the data is always read and written in big endian format.
     Try not to call this on big endian machines, since it's just a
     wasterful copy operation in that case. 
  */
  unsigned char * new_buf;
  unsigned int data_size, i = 0;
  
  data_size = data_elem * word_size;
  new_buf = calloc(data_elem, word_size);
  if (new_buf == NULL) {
    parse_error("Malloc failed. (Low memory?)");
    return NULL;
  }
#ifdef WORDS_BIGENDIAN
  for (i=0; i<data_size; i++) new_buf[i] = data[i];
#else
  for (i=0; i<data_size; i+=word_size) {
    switch (word_size) {
    case 1:
      new_buf[i] = data[i];
      break;
    case 2:
      new_buf[i+1] = data[i];
      new_buf[i] = data[i+1];
      break;
    case 4:
      new_buf[i+3] = data[i];
      new_buf[i+2] = data[i+1];
      new_buf[i+1] = data[i+2];
      new_buf[i] = data[i+3];
      break;
    case 8:
      new_buf[i+7] = data[i];
      new_buf[i+6] = data[i+1];
      new_buf[i+5] = data[i+2];
      new_buf[i+4] = data[i+3];
      new_buf[i+3] = data[i+4];
      new_buf[i+2] = data[i+5];
      new_buf[i+1] = data[i+6];
      new_buf[i] = data[i+7];
      break;
    default:
      new_buf[i] = data[i];
      break;
    }
  }
#endif /* WORDS_BIGENDIAN */
  return new_buf;
}


char * var_endian(Var * v) {
  unsigned int ws = 0;
  unsigned int el = 0;
  
  el = V_SIZE(v)[0] * V_SIZE(v)[1] * V_SIZE(v)[2];
  ws = NBYTES(V_FORMAT(v));
  return flip_endian(V_DATA(v), el, ws);
}

void
WriteHDF5(hid_t parent, char *name, Var *v)
{
    hid_t dataset, datatype, dataspace, aid2, attr, child;
    hsize_t size[3];
    int org;
    int top = 0;
    int i;
    hsize_t length;
    Var *e;
    int lines,bsize,index;
    char * temp_data;
    unsigned char buf[256];

    if (V_NAME(v) != NULL) {
        if ((e = eval(v)) != NULL) v = e;
    }
    if (v == NULL || V_TYPE(v) == ID_UNK) {
        parse_error("Can't find variable");
        return;
    }
    

    if (parent < 0) {
        top = 1;
        parent = H5Fcreate(name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }

    switch (V_TYPE(v)) {
    case ID_STRUCT:

    {
        Var *d;
        char *n;
        /*
        ** member is a structure.  We need to create a sub-group
        ** with this name.
        */
        if (top == 0) {
            child = H5Gcreate(parent, name, 0);
        } else {
            child = parent;
        }
        for (i = 0 ; i < get_struct_count(v) ; i++) {
            get_struct_element(v, i, &n, &d);
            WriteHDF5(child, n, d);
        }
        if (top == 0) H5Gclose(child);
        break;
    }

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
        case BYTE:  datatype = H5Tcopy(H5T_STD_U8BE); break;
        case SHORT: datatype = H5Tcopy(H5T_STD_I16BE); break;
        case INT:   datatype = H5Tcopy(H5T_STD_I32BE); break;
        case FLOAT: datatype = H5Tcopy(H5T_IEEE_F32BE); break;
        case DOUBLE: datatype = H5Tcopy(H5T_IEEE_F64BE); break;
        }
        dataset = H5Dcreate(parent, name, datatype, dataspace, H5P_DEFAULT);
#ifdef WORDS_BIGENDIAN
        H5Dwrite(dataset, datatype, 
                 H5S_ALL, H5S_ALL, H5P_DEFAULT, V_DATA(v));
#else
	temp_data = var_endian(v);
	if (temp_data != NULL) {
	  H5Dwrite(dataset, datatype,
		   H5S_ALL, H5S_ALL, H5P_DEFAULT, temp_data);
	  free(temp_data);
	}
#endif

        aid2 = H5Screate(H5S_SCALAR);
        attr = H5Acreate(dataset, "org", H5T_STD_I32BE, aid2, H5P_DEFAULT);
#ifndef WORDS_BIGENDIAN
	intswap(org);
#endif
        H5Awrite(attr, H5T_STD_U32BE, &org);
        H5Sclose(aid2);
        H5Aclose(attr);

        H5Tclose(datatype);
        H5Sclose(dataspace);
        H5Dclose(dataset);
        break;


    case ID_STRING:
        /*
        ** Member is a string of characters
        */
        lines=1;
        length=1;/*Set length to 1, and size to length*/
        dataspace = H5Screate_simple(1,&length,NULL);
        length=strlen(V_STRING(v))+1;/*String+NULL*/
        datatype = H5Tcopy(H5T_C_S1);
        H5Tset_size(datatype,length);
        dataset = H5Dcreate(parent,name, datatype, dataspace, H5P_DEFAULT);
        H5Dwrite(dataset, datatype,H5S_ALL, H5S_ALL, H5P_DEFAULT, V_STRING(v));

        aid2 = H5Screate(H5S_SCALAR);
        attr = H5Acreate(dataset, "lines", H5T_STD_I32BE, aid2, H5P_DEFAULT);
#ifndef WORDS_BIGENDIAN
	intswap(lines);
#endif
        H5Awrite(attr, H5T_STD_I32BE, &lines);
        H5Sclose(aid2);
        H5Aclose(attr);


        H5Tclose(datatype);
        H5Sclose(dataspace);
        H5Dclose(dataset);

        break;

    case ID_TEXT:
    {
        unsigned char *big;
        int big_size=0;
        int stln=0;
        int i;

        /*Pack the array into a single buffer with newlines*/
        for (i=0;i<V_TEXT(v).Row;i++){
            big_size+=strlen(V_TEXT(v).text[i])+1;/* Added 1 for \n char */
        }
        big_size++; /*Final NULL*/
        big=(unsigned char *)calloc(big_size,sizeof(char));
        big_size=0;
        for (i=0;i<V_TEXT(v).Row;i++){
            stln=strlen(V_TEXT(v).text[i]);
            memcpy((big+big_size),V_TEXT(v).text[i],stln);
            big_size+=stln;
            big[big_size]='\n';
            big_size++;
        }
        big[big_size]='\0';
        big_size++;

        /*Now we can write out this big fat array*/
        length=1;
        dataspace = H5Screate_simple(1,&length,NULL);	
        datatype = H5Tcopy(H5T_C_S1);
        H5Tset_size(datatype,big_size);
        dataset = H5Dcreate(parent,name, datatype, dataspace, H5P_DEFAULT);
        H5Dwrite(dataset, datatype,H5S_ALL, H5S_ALL, H5P_DEFAULT, big);
        lines=V_TEXT(v).Row;
        aid2 = H5Screate(H5S_SCALAR);
        attr = H5Acreate(dataset, "lines", H5T_STD_I32BE, aid2, H5P_DEFAULT);
#ifndef WORDS_BIGENDIAN
	intswap(lines);
#endif
        H5Awrite(attr, H5T_STD_I32BE, &lines);

        H5Sclose(aid2);
        H5Aclose(attr);
        H5Tclose(datatype);
        H5Sclose(dataspace);
        H5Dclose(dataset);
        free(big);
	
    }
	
    break;

    }

    if (top) H5Fclose(parent);
    return;
}

/*
** Make a VAR out of a HDF5 object
*/
herr_t group_iter(hid_t parent, const char *name, void *data)
{
    H5G_stat_t buf;
    hid_t child, dataset, dataspace, datatype, attr;
    int org, type, size[3],  dsize, i;
    int var_type;
    hsize_t datasize[3], maxsize[3];
    H5T_class_t classtype;
    Var *v;
    void *databuf, *databuf2;
    int Lines=1;
	extern int VERBOSE;

    *((Var **)data) = NULL;

    if (H5Gget_objinfo(parent, name, 1, &buf) < 0) return -1;
    type = buf.type;


    switch (type)  {
    case H5G_GROUP:
        child = H5Gopen(parent, name);
        v = load_hdf5(child);
        V_NAME(v) = (name ? strdup(name) : 0);
        H5Gclose(child);
        break;

    case H5G_DATASET:
        if ((dataset = H5Dopen(parent, name)) < 0) {
            return 0;
        }

        datatype = H5Dget_type(dataset);
        classtype=(H5Tget_class(datatype));

        if (classtype==H5T_INTEGER){
            if (H5Tequal(datatype , H5T_STD_U8BE) || H5Tequal(datatype, H5T_NATIVE_UCHAR)) 
                type = BYTE;
            else if (H5Tequal(datatype , H5T_STD_I16BE) || H5Tequal(datatype, H5T_NATIVE_SHORT)) 
                type = SHORT;
            else if (H5Tequal(datatype , H5T_STD_I32BE) || H5Tequal(datatype, H5T_NATIVE_INT))   
                type = INT;
        }
        else if (classtype==H5T_FLOAT){
            if (H5Tequal(datatype , H5T_IEEE_F32BE) || H5Tequal(datatype, H5T_NATIVE_FLOAT)) 
                type = FLOAT;
            else if (H5Tequal(datatype , H5T_IEEE_F64BE) || H5Tequal(datatype, H5T_NATIVE_DOUBLE)) 
                type = DOUBLE;
        }

        else if (classtype=H5T_STRING){
            type=ID_STRING;
        }

        if (type!=ID_STRING){
            attr = H5Aopen_name(dataset, "org");
            H5Aread(attr, H5T_STD_I32BE, &org);
#ifndef WORDS_BIGENDIAN
	    intswap(org);
#endif
            H5Aclose(attr);
        }
        else {
            attr = H5Aopen_name(dataset, "lines");
            H5Aread(attr, H5T_STD_I32BE, &Lines);
#ifndef WORDS_BIGENDIAN
	    intswap(Lines);
#endif
            H5Aclose(attr);
        }

        if ((type!=ID_STRING)){

            dataspace = H5Dget_space(dataset);
            H5Sget_simple_extent_dims(dataspace, datasize, maxsize);
            for (i = 0 ; i < 3 ; i++) {
                size[i] = datasize[i];
            }
            dsize = H5Sget_simple_extent_npoints(dataspace);
            databuf = calloc(dsize, NBYTES(type));
            H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, databuf);
#ifndef WORDS_BIGENDIAN
	    databuf2 = flip_endian(databuf, dsize, NBYTES(type));
	    free(databuf);
	    databuf = databuf2;
#endif /* WORDS_BIGENDIAN */
            H5Tclose(datatype);
            H5Sclose(dataspace);
            H5Dclose(dataset);
	    
            v = newVal(org, size[0], size[1], size[2], type, databuf);

            V_NAME(v) = strdup(name);
        }	

        else {
            v=newVar();
            if (Lines==1)
                V_TYPE(v)=ID_STRING;
            else {
                V_TYPE(v)=ID_TEXT;
                V_TEXT(v).Row=Lines;
                V_TEXT(v).text=(char **)calloc(Lines,sizeof(char *));
            }

            dataspace = H5Dget_space(dataset);
            H5Sget_simple_extent_dims(dataspace, datasize, maxsize);
            for (i = 0 ; i < 3 ; i++) {
                size[i] = datasize[i];
            }
/*       	dsize = H5Sget_simple_extent_npoints(dataspace);*/
            dsize = H5Tget_size(datatype);
            databuf = (unsigned char *)calloc(dsize, sizeof(char));
            H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, databuf);
            if (Lines==1)
                V_STRING(v)=databuf;
            else { /*Now we have to dissable to the large block of text*/
                int lm=0,nm=0;
                int len=strlen(databuf);
                int index=0;
                /*send nm through databuf looking for \n.*/
                while (nm < len) {
                    if (((char *)databuf)[nm]=='\n'){
                        V_TEXT(v).text[index]=malloc(nm-lm+1);
                        memcpy(
                            V_TEXT(v).text[index],
                            (char *)databuf+lm,
                            (nm-lm+1));
                        V_TEXT(v).text[index][nm-lm]='\0';
                        index++;
                        nm++;/*Next line or end*/
                        lm=nm;
                    }
                    else 	
                        nm++;
                }
            }
					
						
            H5Sclose(dataspace);
            H5Dclose(dataset);
            V_NAME(v) = strdup(name);
        }
    }
	if (VERBOSE > 2) {
		pp_print_var(v, V_NAME(v), 0, 0);
	}

    *((Var **)data) = v;
    return 1;
}


herr_t count_group(hid_t group, const char *name, void *data)
{
    *((int *)data) += 1;
    return(0);
}

Var *
LoadHDF5(char *filename)
{
    Var *v;
    hid_t group;
    hid_t file;

    if (H5Fis_hdf5(filename) == 0) {
        return(NULL);
    } 

    file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

    if (file < 0) return(NULL);

    group = H5Gopen(file, "/");

    v = load_hdf5(group);

    H5Gclose(group);
    H5Fclose(file);
    return(v);
}


Var *
load_hdf5(hid_t parent)
{
    Var *o, *e;
    int count = 0;
    int idx = 0;
    herr_t ret;

    H5Giterate(parent, ".", NULL, count_group, &count);

    if (count <= 0) {
        parse_error("Group count < 0");
        return(NULL);
    }
    
    o = new_struct(count);
    
    while ((ret = H5Giterate(parent, ".", &idx, group_iter, &e)) > 0)  {
        add_struct(o, V_NAME(e), e);
        V_NAME(e) = NULL;
    }
    return(o);
}

#endif
