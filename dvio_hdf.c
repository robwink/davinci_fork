#include "parser.h"
#include "func.h"
#include "cvector.h"
#include <sys/stat.h>

#ifdef HAVE_LIBHDF5

#define HDF5_COMPRESSION_LEVEL 6

#include <hdf5.h>


typedef struct callback_data
{
	Var* parent_var;
	cvector_void addresses;
} callback_data;

Var *load_hdf5(hid_t parent, callback_data* cb_data);


void
WriteHDF5(hid_t parent, char *name, Var *v)
{
	hid_t dataset, datatype, native_datatype, dataspace, aid2, attr, child, plist;
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
			child = H5Gcreate(parent, name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
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

		//why are we always storing them as BE?  seems arbitrary to me
		//since we can read both and BE will almost always require conversion both
		//ways because who runs BE machines these days?
		//We should just store them as the native format, shorter, simpler code
		switch(V_FORMAT(v)) {
		case BYTE:
			datatype = H5Tcopy(H5T_STD_U8BE);
			native_datatype = H5Tget_native_type(H5T_STD_U8BE, H5T_DIR_ASCEND);
			break;
		case SHORT:
			datatype = H5Tcopy(H5T_STD_I16BE);
			native_datatype = H5Tget_native_type(H5T_STD_I16BE, H5T_DIR_ASCEND);
			break;
		case USHORT:
			datatype = H5Tcopy(H5T_STD_U16BE);
			native_datatype = H5Tget_native_type(H5T_STD_U16BE, H5T_DIR_ASCEND);
			break;
		case INT:
			datatype = H5Tcopy(H5T_STD_I32BE);
			native_datatype = H5Tget_native_type(H5T_STD_I32BE, H5T_DIR_ASCEND);
			break;
		case FLOAT:
			datatype = H5Tcopy(H5T_IEEE_F32BE);
			native_datatype = H5Tget_native_type(H5T_IEEE_F32BE, H5T_DIR_ASCEND);
			break;
		case DOUBLE:
			datatype = H5Tcopy(H5T_IEEE_F64BE);
			native_datatype = H5Tget_native_type(H5T_IEEE_F64BE, H5T_DIR_ASCEND);
			break;
		}

		/*
		** Enable chunking and compression - JAS
		*/
		plist = H5Pcreate(H5P_DATASET_CREATE);
		H5Pset_chunk(plist, 3, size);
		H5Pset_deflate(plist, HDF5_COMPRESSION_LEVEL);

		dataset = H5Dcreate(parent, name, datatype, dataspace, H5P_DEFAULT, plist, H5P_DEFAULT);
		H5Dwrite(dataset, native_datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, V_DATA(v));
		H5Tclose(native_datatype);

		aid2 = H5Screate(H5S_SCALAR);

		//again why BE?
		attr = H5Acreate(dataset, "org", H5T_STD_I32BE, aid2, H5P_DEFAULT, H5P_DEFAULT);

		native_datatype = H5Tget_native_type(H5T_STD_I32BE, H5T_DIR_ASCEND);
		H5Awrite(attr, native_datatype, &org);
		H5Tclose(native_datatype);

		H5Sclose(aid2);
		H5Aclose(attr);

		H5Tclose(datatype);
		H5Sclose(dataspace);
		H5Dclose(dataset);
		H5Pclose(plist);
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
		dataset = H5Dcreate(parent,name, datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Dwrite(dataset, datatype,H5S_ALL, H5S_ALL, H5P_DEFAULT, V_STRING(v));

		aid2 = H5Screate(H5S_SCALAR);
		attr = H5Acreate(dataset, "lines", H5T_STD_I32BE, aid2, H5P_DEFAULT, H5P_DEFAULT);
		native_datatype = H5Tget_native_type(H5T_STD_I32BE, H5T_DIR_ASCEND);

		H5Awrite(attr, native_datatype, &lines);
		H5Tclose(native_datatype);
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
		dataset = H5Dcreate(parent,name, datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Dwrite(dataset, datatype,H5S_ALL, H5S_ALL, H5P_DEFAULT, big);
		lines=V_TEXT(v).Row;
		aid2 = H5Screate(H5S_SCALAR);
		attr = H5Acreate(dataset, "lines", H5T_STD_I32BE, aid2, H5P_DEFAULT, H5P_DEFAULT);
		native_datatype = H5Tget_native_type(H5T_STD_I32BE, H5T_DIR_ASCEND);

		H5Awrite(attr, native_datatype, &lines);
		H5Tclose(native_datatype);

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
static herr_t group_iter(hid_t parent, const char *name, const H5L_info_t *info, void *operator_data)
{
	H5G_stat_t buf;

	hid_t child, dataset, dataspace, mem_dataspace, datatype, attr;
	int org, type, native_type_attr, native_type_data, size[3], dsize, i;
	int var_type;
	hsize_t datasize[3], maxsize[3];
	H5T_class_t classtype;
	Var *v = NULL;
	void *databuf, *databuf2;
	int Lines=1;
	extern int VERBOSE;

	callback_data* cb_data = (callback_data*)operator_data;
	Var* parent_var = cb_data->parent_var;

	H5O_info_t obj_info;

	if (H5Oget_info_by_name(parent, name, &obj_info, H5P_DEFAULT) < 0) {
		//output our own error?
		return -1;
	}

	type = obj_info.type;

	switch (type)  {
	case H5O_TYPE_GROUP:
		for (i=0; i<cb_data->addresses.size; ++i) {
			if (obj_info.addr == *CVEC_GET_VOID(&cb_data->addresses, haddr_t, i)) {
				break;
			}
		}
		if (i != cb_data->addresses.size) {
			printf("Warning: loop detected, skipping %s\n", name);
		} else {
			//add addr to list
			cvec_push_void(&cb_data->addresses, &obj_info.addr);

			child = H5Gopen(parent, name, H5P_DEFAULT);
			v = load_hdf5(child, cb_data);
			if (v)
				V_NAME(v) = (name ? strdup(name) : 0);
			H5Gclose(child);
		}
		break;

	case H5O_TYPE_DATASET:
		if ((dataset = H5Dopen(parent, name, H5P_DEFAULT)) < 0) {
			return -1;
		}

		datatype = H5Dget_type(dataset);
		classtype=(H5Tget_class(datatype));
		native_type_data = H5Tget_native_type(datatype, H5T_DIR_ASCEND);

		if (classtype == H5T_STRING) {
			type = ID_STRING;
		} else {
			//technically can return negative number for unsuccessful
			if (H5Tequal(native_type_data, H5T_NATIVE_UCHAR))
				type = BYTE;
			else if (H5Tequal(native_type_data, H5T_NATIVE_SHORT))
				type = SHORT;
			else if (H5Tequal(native_type_data, H5T_NATIVE_USHORT))
				type = USHORT;
			else if (H5Tequal(native_type_data, H5T_NATIVE_INT))
				type = INT;
			else if (H5Tequal(native_type_data, H5T_NATIVE_FLOAT))
				type = FLOAT;
			else if (H5Tequal(native_type_data, H5T_NATIVE_DOUBLE))
				type = DOUBLE;
		}

		if (type != ID_STRING) {
			org = BSQ;
			if ((attr = H5Aopen_name(dataset, "org")) < 0) {
				parse_error("Unable to get org. Assuming %s.\n", Org2Str(org));
			} else {
				native_type_attr = H5Tget_native_type(H5T_STD_I32BE, H5T_DIR_ASCEND);
				H5Aread(attr, native_type_attr, &org);
				H5Aclose(attr);
				H5Tclose(native_type_attr);
			}

			//HDF data sets can have arbitrary rank and we don't
			//want garbage if rank is < 3 so initialize dims to 1
			datasize[0] = datasize[1] = datasize[2] = 1;

			dataspace = H5Dget_space(dataset);
			H5Sget_simple_extent_dims(dataspace, datasize, maxsize);

			for (i = 0 ; i < 3 ; i++) {
				size[i] = datasize[i];
			}
			//mem_dataspace = H5Screate_simple(3, datasize, NULL);

			dsize = H5Sget_simple_extent_npoints(dataspace);
			databuf = calloc(dsize, NBYTES(type));

			//H5Dread(dataset, native_type_data, H5S_ALL, mem_dataspace, H5P_DEFAULT, databuf);
			H5Dread(dataset, native_type_data, H5S_ALL, H5S_ALL, H5P_DEFAULT, databuf);

			H5Tclose(datatype);
			H5Sclose(dataspace);
			//H5Sclose(mem_dataspace);
			H5Dclose(dataset);
			H5Tclose(native_type_data);

			/*
			 * // drd Bug 2208 Loading a particular hdf5 file kills davinci
			 * For the time being, there is no USHORT type
			 * functionally available in davinci.
			 * We can promote any USHORT value to INT
			 * and not change sign
			 */
			if (type == USHORT) { // promote to INT
				databuf2 = calloc(dsize, NBYTES(INT));
				for(i = 0; i < dsize; i++) {
					((int*)databuf2)[i] = (int)((unsigned short *)databuf)[i];
				}
				type = INT;
				free(databuf);
				databuf = databuf2;

			}

			v = newVal(org, size[0], size[1], size[2], type, databuf);
			V_NAME(v) = strdup(name);

		/* else type == ID_STRING */
		} else {
			Lines = 0;
			if ((attr = H5Aopen_name(dataset, "lines")) < 0){
				parse_error("Unable to get lines. Assuming %d.\n", Lines);
			} else {
				native_type_attr = H5Tget_native_type(H5T_STD_I32BE, H5T_DIR_ASCEND);
				H5Aread(attr, native_type_attr, &Lines);
				H5Aclose(attr);
				H5Tclose(native_type_attr);
			}

			v=newVar();
			if (Lines==1) {
				V_TYPE(v)=ID_STRING;
			} else {
				V_TYPE(v)=ID_TEXT;
				V_TEXT(v).Row=Lines;
				V_TEXT(v).text=(char **)calloc(Lines,sizeof(char *));
			}

			dataspace = H5Dget_space(dataset);
			H5Sget_simple_extent_dims(dataspace, datasize, maxsize);
			for (i = 0 ; i < 3 ; i++) {
				size[i] = datasize[i];
			}
			/*  dsize = H5Sget_simple_extent_npoints(dataspace);*/
			dsize = H5Tget_size(datatype);
			databuf = (unsigned char *)calloc(dsize, sizeof(char));
			H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, databuf);
			if (Lines <= 1) {
				V_STRING(v)=databuf;
			} else { /*Now we have to dissable to the large block of text*/
				int lm = 0,nm = 0;
				int len = strlen(databuf);
				int index = 0;
				/*send nm through databuf looking for \n.*/
				while (nm < len) {
					if (((char *)databuf)[nm] == '\n'){
						V_TEXT(v).text[index]=malloc(nm-lm+1);
						memcpy(
							V_TEXT(v).text[index],
							(char *)databuf+lm,
							(nm-lm+1));
						V_TEXT(v).text[index][nm-lm] = '\0';
						index++;
						nm++;/*Next line or end*/
						lm=nm;
					}
					else if (nm == (len-1)) {
						/*
						 * drd Bug 2208 Loading a particular hdf5 file kills davinci
						 * This is the case where we are at the end of a 'string' data set,
						 * but the end is not a newline
						 *
						 */
						V_TEXT(v).text[index]=malloc(nm-lm+2);
						memcpy(V_TEXT(v).text[index], (char *)databuf+lm, (nm-lm+2));
						V_TEXT(v).text[index][nm-lm+1]='\0';
						nm++; // This increment will break out of while() loop
					} else {
						nm++;
					}
				}
			}


			H5Sclose(dataspace);
			H5Dclose(dataset);
			V_NAME(v) = strdup(name);
		}
		break;

	//Our HD5 lib is out of date or else this'd be defined
	//case H50_TYPE_NAMED_DATATYPE:
	//	break;
	default:
		parse_error("Unknown object type");
	}

	if (VERBOSE > 2) {
		if (v)
			pp_print_var(v, V_NAME(v), 0, 0);
		else
			printf("Var v = NULL\n");
	}

	//restore in case we recursed for subgroup
	cb_data->parent_var = parent_var;

	if (v) {
		add_struct(cb_data->parent_var, V_NAME(v), v);
	}
	return 0;
}


static herr_t count_group(hid_t group, const char *name, const H5L_info_t *info, void *data)
{
	*((int *)data) += 1;
	return 0;
}

Var *
LoadHDF5(char *filename)
{
	Var *v;
	hid_t group;
	hid_t file;

	if (H5Fis_hdf5(filename) == 0) {
		return NULL;
	}

	file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

	if (file < 0) return NULL;

	callback_data data;
	data.parent_var = NULL;
	cvec_void(&data.addresses, 0, 20, sizeof(haddr_t), NULL, NULL);

	v = load_hdf5(file, &data);

	cvec_free_void(&data.addresses);

	H5Fclose(file);
	return v;
}


Var *
load_hdf5(hid_t parent, callback_data* cb_data)
{
	int count = 0;
	hsize_t idx = 0;
	herr_t ret;

	//I think this is overkill.  Picking a reasonable start size (4-8) is fine
	H5Literate(parent, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, count_group, &count);

	if (count < 0) {
		parse_error("Group count < 0");
		return NULL;
	}

	cb_data->parent_var = new_struct(count);

	if ((ret = H5Literate(parent, H5_INDEX_NAME, H5_ITER_NATIVE, &idx, group_iter, cb_data)) < 0) {
		parse_error("Error reading HDF5 file\n");
	}

	return cb_data->parent_var;
}

#endif
