#include "cvector.h"
#include "func.h"
#include "parser.h"
#include <sys/stat.h>

#ifdef HAVE_LIBHDF5

#define HDF5_COMPRESSION_LEVEL 6

#include <hdf5.h>

typedef struct callback_data {
	Var* parent_var;
	cvector_void addresses;
	int hdf_old;
} callback_data;

Var* load_hdf5(hid_t parent, callback_data* cb_data);

void make_valid_identifier(char* id)
{
	if (!id || !id[0]) return;

	if (!isalpha(id[0]) && id[0] != '_') {
		id[0] = '_';
	}

	char* p = &id[1];
	while (*p) {
		if (!isalnum(*p) && *p != '_') *p = '_';
		++p;
	}
}

void WriteHDF5(hid_t parent, char* name, Var* v, int hdf_old)
{
	hid_t dataset, datatype, dataspace, aid2, attr, child, plist;
	hsize_t size[3];
	int org;
	int top = 0;
	int i;
	hsize_t length;
	Var* e;
	int lines, bsize, index;
	char* temp_data;
	unsigned char buf[256];

	if (V_NAME(v) != NULL) {
		if ((e = eval(v)) != NULL) v = e;
	}
	if (v == NULL || V_TYPE(v) == ID_UNK) {
		parse_error("Can't find variable");
		return;
	}

	if (parent < 0) {
		top    = 1;
		parent = H5Fcreate(name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	}

	// validate the name after above check so we don't change the filename
	make_valid_identifier(name);

	switch (V_TYPE(v)) {
	case ID_STRUCT: {
		Var* d;
		char* n;

		// member is a structure.  We need to create a sub-group
		// with this name.
		if (top == 0) {
			child = H5Gcreate(parent, name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		} else {
			child = parent;
		}
		for (i = 0; i < get_struct_count(v); i++) {
			get_struct_element(v, i, &n, &d);
			WriteHDF5(child, n, d, hdf_old);
		}
		if (top == 0) H5Gclose(child);
		break;
	}

	case ID_VAL:

		// member is a value.  Create a dataset
		for (i = 0; i < 3; i++) {
			if (!hdf_old)
				size[2 - i] = V_SIZE(v)[i];
			else
				size[i] = V_SIZE(v)[i];
		}
		org       = V_ORG(v);
		dataspace = H5Screate_simple(3, size, NULL);

		switch (V_FORMAT(v)) {
		case DV_UINT8: datatype  = H5Tcopy(H5T_NATIVE_UCHAR); break;
		case DV_INT16: datatype  = H5Tcopy(H5T_NATIVE_SHORT); break;
		case DV_UINT16: datatype = H5Tcopy(H5T_NATIVE_USHORT); break;
		case DV_INT32: datatype  = H5Tcopy(H5T_NATIVE_INT); break;
		case DV_FLOAT: datatype  = H5Tcopy(H5T_NATIVE_FLOAT); break;
		case DV_DOUBLE: datatype = H5Tcopy(H5T_NATIVE_DOUBLE); break;
		}

		// Enable chunking and compression - JAS
		plist = H5Pcreate(H5P_DATASET_CREATE);
		H5Pset_chunk(plist, 3, size);
		H5Pset_deflate(plist, HDF5_COMPRESSION_LEVEL);

		dataset = H5Dcreate(parent, name, datatype, dataspace, H5P_DEFAULT, plist, H5P_DEFAULT);
		H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, V_DATA(v));

		aid2 = H5Screate(H5S_SCALAR);

		// org could even be a byte since it's [0-2]
		attr = H5Acreate(dataset, "org", H5T_NATIVE_INT, aid2, H5P_DEFAULT, H5P_DEFAULT);
		H5Awrite(attr, H5T_NATIVE_INT, &org);

		H5Sclose(aid2);
		H5Aclose(attr);

		if (!hdf_old) {
			// value of dv_std doesn't really matter, just the existence of the attribute
			// in fact we could change below to H5S_NULL and not write any data
			aid2       = H5Screate(H5S_SCALAR);
			int dv_std = 1;
			attr = H5Acreate(dataset, "dv_std", H5T_NATIVE_INT, aid2, H5P_DEFAULT, H5P_DEFAULT);
			H5Awrite(attr, H5T_NATIVE_INT, &dv_std);

			H5Sclose(aid2);
			H5Aclose(attr);
		}

		H5Tclose(datatype);
		H5Sclose(dataspace);
		H5Dclose(dataset);
		H5Pclose(plist);
		break;

	case ID_STRING:
		// Member is a string of characters

		lines     = 1;
		length    = 1; // Set length to 1, and size to length
		dataspace = H5Screate_simple(1, &length, NULL);
		length    = strlen(V_STRING(v)) + 1; // String+NULL
		datatype  = H5Tcopy(H5T_C_S1);
		H5Tset_size(datatype, length);
		dataset = H5Dcreate(parent, name, datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, V_STRING(v));

		aid2 = H5Screate(H5S_SCALAR);
		attr = H5Acreate(dataset, "lines", H5T_NATIVE_INT, aid2, H5P_DEFAULT, H5P_DEFAULT);

		H5Awrite(attr, H5T_NATIVE_INT, &lines);
		H5Sclose(aid2);
		H5Aclose(attr);

		H5Tclose(datatype);
		H5Sclose(dataspace);
		H5Dclose(dataset);

		break;

	case ID_TEXT: {
		unsigned char* big;
		int big_size = 0;
		int stln     = 0;
		int i;

		// Pack the array into a single buffer with newlines
		for (i = 0; i < V_TEXT(v).Row; i++) {
			big_size += strlen(V_TEXT(v).text[i]) + 1; // Added 1 for \n char
		}
		big_size++; // Final NULL
		big      = (unsigned char*)calloc(big_size, sizeof(char));
		big_size = 0;
		for (i = 0; i < V_TEXT(v).Row; i++) {
			stln = strlen(V_TEXT(v).text[i]);
			memcpy((big + big_size), V_TEXT(v).text[i], stln);
			big_size += stln;
			big[big_size] = '\n';
			big_size++;
		}
		big[big_size] = '\0';
		big_size++;

		/*Now we can write out this big fat array*/
		length    = 1;
		dataspace = H5Screate_simple(1, &length, NULL);
		datatype  = H5Tcopy(H5T_C_S1);
		H5Tset_size(datatype, big_size);
		dataset = H5Dcreate(parent, name, datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, big);
		lines = V_TEXT(v).Row;
		aid2  = H5Screate(H5S_SCALAR);
		attr  = H5Acreate(dataset, "lines", H5T_NATIVE_INT, aid2, H5P_DEFAULT, H5P_DEFAULT);

		H5Awrite(attr, H5T_NATIVE_INT, &lines);

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

// Make a VAR out of a HDF5 object
static herr_t group_iter(hid_t parent, const char* name, const H5L_info_t* info, void* operator_data)
{
	H5G_stat_t buf;

	hid_t child, dataset, dataspace, mem_dataspace, datatype, attr;
	int org, type, native_type_data, size[3], dsize, i;
	int var_type;
	hsize_t datasize[3], maxsize[3];
	H5T_class_t classtype;
	Var* v = NULL;
	void *databuf, *databuf2;
	int Lines = 1;

	callback_data* cb_data = (callback_data*)operator_data;
	Var* parent_var        = cb_data->parent_var;

	H5O_info_t obj_info;

	if (H5Oget_info_by_name(parent, name, &obj_info, H5P_DEFAULT) < 0) {
		// output our own error?
		return -1;
	}

	type = obj_info.type;

	switch (type) {
	case H5O_TYPE_GROUP:
		for (i = 0; i < cb_data->addresses.size; ++i) {
			if (obj_info.addr == *CVEC_GET_VOID(&cb_data->addresses, haddr_t, i)) {
				break;
			}
		}
		if (i != cb_data->addresses.size) {
			printf("Warning: loop detected, skipping %s\n", name);
		} else {
			// add addr to list
			cvec_push_void(&cb_data->addresses, &obj_info.addr);

			child            = H5Gopen(parent, name, H5P_DEFAULT);
			v                = load_hdf5(child, cb_data);
			if (v) V_NAME(v) = (name ? strdup(name) : 0);
			H5Gclose(child);
		}
		break;

	case H5O_TYPE_DATASET:
		if ((dataset = H5Dopen(parent, name, H5P_DEFAULT)) < 0) {
			return -1;
		}

		datatype         = H5Dget_type(dataset);
		classtype        = (H5Tget_class(datatype));
		native_type_data = H5Tget_native_type(datatype, H5T_DIR_ASCEND);

		if (classtype == H5T_STRING) {
			type = ID_STRING;
		} else {
			// technically can return negative number for unsuccessful
			if (H5Tequal(native_type_data, H5T_NATIVE_UCHAR))
				type = DV_UINT8;
			else if (H5Tequal(native_type_data, H5T_NATIVE_SHORT))
				type = DV_INT16;
			else if (H5Tequal(native_type_data, H5T_NATIVE_USHORT))
				type = DV_UINT16;
			else if (H5Tequal(native_type_data, H5T_NATIVE_INT))
				type = DV_INT32;
			else if (H5Tequal(native_type_data, H5T_NATIVE_FLOAT))
				type = DV_FLOAT;
			else if (H5Tequal(native_type_data, H5T_NATIVE_DOUBLE))
				type = DV_DOUBLE;
		}

		if (type != ID_STRING) {
			org               = BSQ;
			int org_exists    = 0;
			int dv_std_exists = 0;
			if (!H5Aexists(dataset, "org")) {
				parse_error("Unable to find org attribute. Assuming %s.\n", Org2Str(org));
			} else {
				attr = H5Aopen_name(dataset, "org");
				H5Aread(attr, H5T_NATIVE_INT, &org);

				H5Aclose(attr);
				org_exists = 1;
				if (H5Aexists(dataset, "dv_std")) dv_std_exists = 1;
			}

			// HDF data sets can have arbitrary rank and we don't
			// want garbage if rank is < 3 so initialize dims to 1
			datasize[0] = datasize[1] = datasize[2] = 1;

			dataspace = H5Dget_space(dataset);
			H5Sget_simple_extent_dims(dataspace, datasize, maxsize);

			for (i = 0; i < 3; i++) {
				// HDF stores data in row-major order like C, or "along the fasted-changing
				// dimension"
				// So while we say X, Y, Z and think of that as row/column/plane and access it in
				// davinci as
				// array[x,y,z], if you declared it in C array[X][Y][Z], it's actually stored Z, Y,
				// X
				// so to get the correct representation we reverse the dimensions on read and write.
				// see the fine print Note just above 7.3.2.6 in the HDF5 User's Guide
				//
				// www.hdfgroup.org/HDF5/doc/UG/HDF5_Users_Guide-Responsive HTML5/index.html

				if (cb_data->hdf_old || (org_exists && !dv_std_exists))
					size[i] = datasize[i];
				else
					size[2 - i] = datasize[i];
			}

			dsize   = H5Sget_simple_extent_npoints(dataspace);
			databuf = calloc(dsize, NBYTES(type));

			H5Dread(dataset, native_type_data, H5S_ALL, H5S_ALL, H5P_DEFAULT, databuf);

			H5Tclose(datatype);
			H5Sclose(dataspace);
			H5Dclose(dataset);
			H5Tclose(native_type_data);

			/*
			 * // drd Bug 2208 Loading a particular hdf5 file kills davinci
			 * For the time being, there is no Dv_UINT16 type
			 * functionally available in davinci.
			 * We can promote any DV_UINT16 value to DV_INT32
			 * and not change sign
			 */
			if (type == DV_UINT16) { // promote to DV_INT32
				databuf2 = calloc(dsize, NBYTES(DV_INT32));
				for (i = 0; i < dsize; i++) {
					((int*)databuf2)[i] = (int)((unsigned short*)databuf)[i];
				}
				type = DV_INT32;
				free(databuf);
				databuf = databuf2;
			}

			v         = newVal(org, size[0], size[1], size[2], type, databuf);
			V_NAME(v) = strdup(name);

			// else type == ID_STRING
		} else {
			Lines = 1;
			if (!H5Aexists(dataset, "lines")) {
				parse_error("Unable to find lines attribute. Assuming %d.\n", Lines);
			} else {
				attr = H5Aopen_name(dataset, "lines");
				H5Aread(attr, H5T_NATIVE_INT, &Lines);
				H5Aclose(attr);
			}

			v = newVar();
			if (Lines == 1) {
				V_TYPE(v) = ID_STRING;
			} else {
				V_TYPE(v)      = ID_TEXT;
				V_TEXT(v).Row  = Lines;
				V_TEXT(v).text = (char**)calloc(Lines, sizeof(char*));
			}

			dataspace = H5Dget_space(dataset);
			H5Sget_simple_extent_dims(dataspace, datasize, maxsize);
			for (i = 0; i < 3; i++) {
				size[i] = datasize[i];
			}
			//  dsize = H5Sget_simple_extent_npoints(dataspace);
			dsize   = H5Tget_size(datatype);
			databuf = (unsigned char*)calloc(dsize, sizeof(char));
			H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, databuf);
			if (Lines <= 1) {
				V_STRING(v) = databuf;
			} else { // Now we have to dissable to the large block of text
				int lm = 0, nm = 0;
				int len   = strlen(databuf);
				int index = 0;
				// send nm through databuf looking for \n.
				while (nm < len) {
					if (((char*)databuf)[nm] == '\n') {
						V_TEXT(v).text[index] = malloc(nm - lm + 1);
						memcpy(V_TEXT(v).text[index], (char*)databuf + lm, (nm - lm + 1));
						V_TEXT(v).text[index][nm - lm] = '\0';
						index++;
						nm++; /*Next line or end*/
						lm = nm;
					} else if (nm == (len - 1)) {
						// drd Bug 2208 Loading a particular hdf5 file kills davinci
						// This is the case where we are at the end of a 'string' data set,
						// but the end is not a newline
						V_TEXT(v).text[index] = malloc(nm - lm + 2);
						memcpy(V_TEXT(v).text[index], (char*)databuf + lm, (nm - lm + 2));
						V_TEXT(v).text[index][nm - lm + 1] = '\0';
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

	// Our HDF5 lib is out of date on some platforms so we have to code
	// to the lowest common denominator or else this'd be defined
	// case H50_TYPE_NAMED_DATATYPE:
	//	break;
	default: parse_error("Unknown object type");
	}

	// restore in case we recursed for subgroup
	cb_data->parent_var = parent_var;

	if (v) {
		make_valid_identifier(V_NAME(v));
		add_struct(cb_data->parent_var, V_NAME(v), v);
	}

	if (VERBOSE > 2) {
		if (v)
			pp_print_var(v, V_NAME(v), 0, 0);
		else
			printf("Var v = NULL\n");
	}

	return 0;
}

static herr_t count_group(hid_t group, const char* name, const H5L_info_t* info, void* data)
{
	*((int*)data) += 1;
	return 0;
}

Var* LoadHDF5(char* filename, int hdf_old)
{
	Var* v;
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
	data.hdf_old = hdf_old;

	v = load_hdf5(file, &data);

	cvec_free_void(&data.addresses);

	H5Fclose(file);
	return v;
}

Var* load_hdf5(hid_t parent, callback_data* cb_data)
{
	int count   = 0;
	hsize_t idx = 0;
	herr_t ret;

	// NOTE(rswinkle) I think this is overkill.  Picking a reasonable start size (4-8) is fine
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
