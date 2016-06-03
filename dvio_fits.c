#include "func.h"

#if defined(HAVE_LIBCFITSIO) && (defined(HAVE_CFITSIO_FITSIO_H) || defined(HAVE_FITSIO_H))

#ifdef HAVE_FITSIO_H
#include <fitsio.h>
#else
#include <cfitsio/fitsio.h>
// #include "/mars/u/ddoerres/work/cfitsio/fitsio.h"
#endif

#include "dvio_fits.h"
#include <regex.h>
#include <sys/types.h>

#define GET_KEY_VAL_INT(s, key, var, dflt) find_struct(s, key, &var) >= 0 ? V_INT(var) : dflt
#define GET_KEY_VAL_STRING(s, key, var, dflt) find_struct(s, key, &var) >= 0 ? V_STRING(var) : dflt

#define KW_EXT "XTENSION"
#define KW_EXT_NAME "EXTNAME"
#define KW_BITPIX "BITPIX"
#define KW_SIMPLE "SIMPLE"
#define KW_EXTEND "EXTEND"
#define KW_TFIELDS "TFIELDS"
#define KW_NAXIS "NAXIS"
#define KW_NAXIS1 "NAXIS1"
#define KW_NAXIS2 "NAXIS2"
#define KW_TTYPE "TTYPE"
#define KW_TBCOL "TBCOL"
#define KW_TFORM "TFORM"
#define KW_TUNIT "TUNIT"
#define KW_GROUPS "GROUPS"
#define KW_PCOUNT "PCOUNT"
#define KW_GCOUNT "GCOUNT"
#define VAL_EXT_TABLE "TABLE"
#define VAL_EXT_BINTABLE "BINTABLE"

#define SUBEL_DATA "data"

/* from cfitsio's putkey.c:
 primary header keywords (empty file):
 KW_SIMPLE,  // file conforms / does not conform to FITS standard
 KW_BITPIX,  // bits/pixel
 KW_NAXIS,   // number of axes; also AXISnn
 KW_EXTEND,  // file may contain extensions
 KW_GROUPS,  // whether random group records are present
 KW_PCOUNT,  // number of random group parameters
 KW_GCOUNT,  // number of group parameters

 image extension keywords:
 KW_EXT,     // IMAGE
 KW_BITPIX,  // bits/pixel
 KW_NAXIS,   // number of axes; also AXISnn
 KW_PCOUNT,  // 0
 KW_GCOUNT,  // 1

 ascii table keywords:
 KW_EXT,     // TABLE
 KW_BITPIX,  // 8
 KW_NAXIS,   // 2
 KW_NAXIS1,  // width of table in characters
 KW_NAXIS2,  // number of rows in table
 KW_PCOUNT,  // 0 - no group parameters
 KW_GCOUNT,  // 1 - one data group
 KW_TFIELDS, // number of fields in each row
 KW_TTYPE,   // TTYPEnn field name
 KW_TBCOL,   // TBCOLnn begining column of the field
 KW_TFORM,   // TFORMnn F77 format of field
 KW_TUNIT,   // UNITnn physical unit of field
 KW_EXT_NAME // optional name of table

 binary table keywords:
 KW_EXT,     // BINTABLE
 KW_BITPIX,  // 8
 KW_NAXIS,   // 2
 KW_NAXIS1,  // width of table in bytes
 KW_NAXIS2,  // number of rows in table
 KW_PCOUNT,  // initially 0 - size of variable length array heap
 KW_GCOUNT,  // 1 - one data group
 KW_TFIELDS, // number of fields in each row
 KW_TTYPE,   // TTYPEnn field name
 KW_TBCOL,   // TBCOLnn begining column of the field
 KW_TFORM,   // TFORMnn F77 format of field
 KW_TUNIT,   // UNITnn physical unit of field
 KW_EXT_NAME // optional name of table

 */

/* should the given keyword be filtered? */
int filter_kw(const char* kw)
{
	const char* fixed_kw[] = {KW_SIMPLE, KW_BITPIX, KW_NAXIS,   KW_EXTEND, KW_GROUPS,
	                          KW_PCOUNT, KW_GCOUNT, KW_TFIELDS, KW_EXT,    KW_EXT_NAME};

	const char* num_sfx_kw[] = {KW_NAXIS, KW_TTYPE, KW_TBCOL, KW_TFORM, KW_TUNIT};

	int nfixed_kw = sizeof(fixed_kw) / sizeof(char*);
	int nsfx_kw   = sizeof(num_sfx_kw) / sizeof(char*);
	int i, n, errcode;
	char errbuf[1024];
	regmatch_t pmatch[1];
	static char* regex = NULL;
	static regex_t preg;

	if (regex == NULL) {
		n = nfixed_kw * 10 + nsfx_kw * 15;
		if ((regex = (char*)calloc(sizeof(char), n)) == NULL) {
			parse_error("%s: Unable to allocate %d bytes for regex\n", "filter_kw", n);
			return 0;
		}
		memset(regex, '\0', n);

		for (i = 0; i < (sizeof(fixed_kw) / sizeof(char*)); i++) {
			if (i > 0) strcat(regex, "|");
			strcat(regex, fixed_kw[i]);
		}
		if (strlen(regex) > 0) strcat(regex, "|");
		for (i = 0; i < (sizeof(num_sfx_kw) / sizeof(char*)); i++) {
			if (i > 0) strcat(regex, "|");
			strcat(regex, num_sfx_kw[i]);
			strcat(regex, "[0-9][0-9]*");
		}
		// printf("%s\n", regex); // drd for debug
		if ((errcode = regcomp(&preg, regex, REG_EXTENDED)) != 0) {
			regerror(errcode, &preg, errbuf, sizeof(errbuf) - 1);
			parse_error("%s: Error compiling regex: \"%s\". Reason: %s\n", "filter_kw", "filter_kw",
			            regex, errbuf);

			free(regex);
			regex = NULL;
		}
	}

	if (regexec(&preg, kw, sizeof(pmatch) / sizeof(regmatch_t), pmatch, 0) == 0) {
		return 1;
	}

	return 0;
}

int Write_FITS_Image(fitsfile* fptr, Var* obj)
{
	char* swapData;
	char* data;
	int naxis;
	long naxes[3];
	int bitpix;
	int datatype;
	int* size;
	int i;
	int status = 0;
	int fits_type;
	long fpixel[3] = {1, 1, 1};
	long sz;
	long x, y, z, n;

	VarType2FitsType(obj, &bitpix, &datatype);

	size  = V_SIZE(obj);
	naxis = 0;
	for (i = 0; i < 3; i++) {
		if (size[i]) {
			naxis++;
			naxes[i] = size[i];
		} else
			naxes[i] = 0;
	}
	fits_create_img(fptr, bitpix, naxis, naxes, &status);
	QUERY_FITS_ERROR(status, " creating image", 0);

	for (i = 0; i < 3; i++)
		if (!(naxes[i]))
			size[i] = 1; // I don't think this is needed, but just in case V_SIZE() returns a 0 in
			             // one of the size slots

	/*
	 * bitpix could be negative in theory, so abs()
	 * There are 8 bits per byte, so the 8 is hard coded here
	 */
	sz = abs(bitpix) / 8;

	n        = (long)((size_t)size[0]) * ((size_t)size[1]) * ((size_t)size[2]);
	swapData = (char*)calloc(n, sz);
	data     = V_DATA(obj);
	for (z = 0; z < size[2]; z++) {
		for (y = 0; y < size[1]; y++) {

			/*
			 * There is an inclination to get memcpy() backwards
			 * void *memcpy(void *dest, const void *src, size_t n);
			 */
			memcpy(swapData + ((size[0] * y * sz) + (z * size[1] * size[0] * sz)),
			       data + ((z * size[1] * size[0] * sz) + size[0] * sz * (size[1] - 1 - y)), size[0] * sz);
			// memcpy( swap +          ((X* y*sz) + (z*     Y*      X* sz)), base+((z*     Y*      X
			// *sz)+     X* sz*(     Y -1-y)),       X* sz );
		}
	}

	V_DATA(obj) = swapData;
	fits_write_pix(fptr, datatype, fpixel, ((size_t)size[0]) * ((size_t)size[1]) * ((size_t)size[2]),
	               (void*)V_DATA(obj), &status);
	QUERY_FITS_ERROR(status, " writing pixel data", 0);
	V_DATA(obj) = data;
	free(swapData);
	return (1);
}

Var* Read_FITS_Image(fitsfile* fptr)
{
	char* data;
	char* swapData;
	int format = 0;
	int fits_type;
	int i;
	int dim;
	int sz;
	long size[3] = {0, 0, 0};
	long x, y, z;
	long fpixel[3] = {1, 1, 1};
	int datatype   = 0;
	int status     = 0;
	long n;

	fits_get_img_dim(fptr, &dim, &status);
	QUERY_FITS_ERROR(status, " getting image dimension", NULL);

	if (dim > 3) {
		parse_error("Data objects of greater than 3 dimensions are not handled.");
		return (NULL);
	}

	if (dim == 0) {
		parse_error("Warning: image has dimension 0, there is no image to read");
		return (NULL);
	}

	fits_get_img_type(fptr, &fits_type, &status);
	QUERY_FITS_ERROR(status, " getting image type", NULL);

	fits_get_img_size(fptr, dim, size, &status);
	QUERY_FITS_ERROR(status, " getting image size", NULL);

	for (i = 0; i < 3; i++)
		if (!size[i]) {
			size[i]   = 1;
			fpixel[i] = 0;
		}

	switch (fits_type) {

	case BYTE_IMG:
		format   = BYTE;
		datatype = TBYTE;
		break;

	case SHORT_IMG:
		format   = SHORT;
		datatype = TSHORT;
		break;

	case LONG_IMG:
		format   = INT;
		datatype = TINT;
		break;

	case FLOAT_IMG:
		format   = FLOAT;
		datatype = TFLOAT;
		break;

	case DOUBLE_IMG:
		format   = DOUBLE;
		datatype = TDOUBLE;
		break;
	}

	n        = (long)((size_t)size[0]) * ((size_t)size[1]) * ((size_t)size[2]);
	data     = (char*)calloc(n, NBYTES(format));
	swapData = (char*)calloc(n, NBYTES(format));
	sz       = NBYTES(format);

	fits_read_pix(fptr, datatype, fpixel, n, NULL, (void*)data, NULL, &status);
	QUERY_FITS_ERROR(status, " reading image pixels", NULL);

	for (z = 0; z < size[2]; z++) {
		for (y = 0; y < size[1]; y++) {

			/*
			 * There is an inclination to get memcpy() backwards
			 * void *memcpy(void *dest, const void *src, size_t n);
			 */

			memcpy(swapData + ((size[0] * y * sz) + (z * size[1] * size[0] * sz)),
			       data + ((z * size[1] * size[0] * sz) + size[0] * sz * (size[1] - 1 - y)), size[0] * sz);
			// memcpy( swap +          ((X* y*sz) + (z*     Y*      X* sz)), base+((z*     Y*      X
			// *sz)+     X* sz*(     Y -1-y)),       X* sz );
		}
	}
	free(data);
	return (newVal(BSQ, size[0], size[1], size[2], format, swapData));
}

static const char* get_col_type_name(int coltype)
{
	switch (coltype) {
	case TSTRING: return "TSTRING";
	case TBYTE: return "TBYTE";
	case TSHORT: return "TSHORT";
	case TINT: return "TINT";
	case TLONG: return "TLONG";
	case TFLOAT: return "TFLOAT";
	case TDOUBLE: return "TDOUBLE";
	case TUINT: return "TUINT";
	case TUSHORT: return "TUSHORT";
	case TULONG: return "TULONG";
	case TLOGICAL: return "TLOGICAL";
	case TCOMPLEX: return "TCOMPLEX";
	case TDBLCOMPLEX: return "TDBLCOMPLEX";
	}
	return "(unknown)";
}

static char* null_safe_strdup(char* s)
{
	if (s == NULL) return s;

	return strdup(s);
}

static char* unquote_remove_spaces(char* s)
{
	if (s == NULL) return NULL;

	ltrim(s, "\' ");
	rtrim(s, "\' ");

	return s;
}

struct tbl_specs {
	char* tblname;
	int tbltype;
	int nfields;
	int nrows;
	char** fnames;
	char** fforms;
	char** funits;
};

static int empty_or_null_string(const char* s)
{
	return (s == NULL || strlen(s) == 0);
}

static int all_null_1d(char** array, int n)
{
	int i;

	for (i = 0; i < n; i++) {
		if (array[i] != NULL) return 0;
	}

	return 1;
}

static int init_table_specs(struct tbl_specs* t)
{
	memset(t, sizeof(struct tbl_specs), 0);
	return 0;
}

static int free_table_specs(struct tbl_specs* t)
{
	int i;

	free(t->tblname);
	for (i = 0; i < t->nfields; i++) {
		free(t->fnames[i]);
		free(t->fforms[i]);
		free(t->funits[i]);
	}
	free(t->fnames);
	free(t->fforms);
	free(t->funits);

	return 0;
}

#define CHECK_MISSING_FIELD_SPEC(t, str, key, colnum, tbl, rtn)                                \
	if (empty_or_null_string(str)) {                                                           \
		parse_error("Missing \"%s\" value for column %d in table \"%s\"\n", key, colnum, tbl); \
		free_table_specs(t);                                                                   \
		return rtn;                                                                            \
	}

static int collect_table_specs(Var* s, struct tbl_specs* t)
{
	char key[512];
	Var* d;
	int i;
	char ext_type[512];

	init_table_specs(t);

	strcpy(ext_type, GET_KEY_VAL_STRING(s, KW_EXT, d, ""));
	unquote_remove_spaces(ext_type);
	t->tbltype = (!ext_type[0] || !strcasecmp(ext_type, VAL_EXT_TABLE)) ? ASCII_TBL : BINARY_TBL;

	t->tblname = null_safe_strdup(GET_KEY_VAL_STRING(s, KW_EXT_NAME, d, NULL));
	if (t->tblname != NULL) {
		unquote_remove_spaces(t->tblname);
	} else {
		char noname[] = "TABLEITEM";
		t->tblname    = strdup(noname);
	}
	t->nfields = GET_KEY_VAL_INT(s, KW_TFIELDS, d, 0);
	t->nrows   = GET_KEY_VAL_INT(s, KW_NAXIS2, d, 0);

	t->fnames = (char**)calloc(sizeof(char*), t->nfields);
	t->fforms = (char**)calloc(sizeof(char*), t->nfields);
	t->funits = (char**)calloc(sizeof(char*), t->nfields);

	if (t->fnames == NULL || t->fforms == NULL || t->funits == NULL) {
		free_table_specs(t);
		parse_error("Unable to alloc memory for collecting table specs.\n");
		return 0;
	}

	for (i = 0; i < t->nfields; i++) {
		sprintf(key, "%s%d", KW_TTYPE, i + 1);
		t->fnames[i] = null_safe_strdup(GET_KEY_VAL_STRING(s, key, d, NULL));
		unquote_remove_spaces(t->fnames[i]);
		CHECK_MISSING_FIELD_SPEC(t, t->fnames[i], key, i + 1, t->tblname, 0);

		sprintf(key, "%s%d", KW_TFORM, i + 1);
		t->fforms[i] = null_safe_strdup(GET_KEY_VAL_STRING(s, key, d, NULL));
		unquote_remove_spaces(t->fforms[i]);
		CHECK_MISSING_FIELD_SPEC(t, t->fforms[i], key, i + 1, t->tblname, 0);

		sprintf(key, "%s%d", KW_TUNIT, i + 1);
		t->funits[i] = null_safe_strdup(GET_KEY_VAL_STRING(s, key, d, NULL));
		unquote_remove_spaces(t->funits[i]);
	}

	if (all_null_1d(t->fnames, t->nfields)) {
		free(t->fnames);
		t->fnames = NULL;
	}
	if (all_null_1d(t->fforms, t->nfields)) {
		free(t->fforms);
		t->fforms = NULL;
	}
	if (all_null_1d(t->fforms, t->nfields)) {
		free(t->fforms);
		t->fforms = NULL;
	}

	return 1;
}

static int adjust_table_specs(Var* tbldata, struct tbl_specs* t, struct tbl_specs* tFixed)
{
	Var* coldata = NULL;
	int i, colCount, j, findStructStatus;
	int* colExists;

	colExists = (int*)calloc(t->nfields, sizeof(int));

	init_table_specs(tFixed);

	colCount = 0;

	for (i = 0; i < t->nfields && tbldata != NULL; i++) {
		colExists[i]     = 1; // we start out with optimism
		findStructStatus = find_struct(tbldata, t->fnames[i], &coldata);
		if (findStructStatus == -1) {
			parse_error("WARNING! Column %s was not found--it may have been removed\n", t->fnames[i]);
			colExists[i] = 0;
		} else {
			colCount++; // This is how many columns are going to be made
		}
	}

	tFixed->tbltype = t->tbltype;
	tFixed->tblname = null_safe_strdup(t->tblname);
	tFixed->nfields = colCount;
	tFixed->nrows   = t->nrows;

	tFixed->fnames = (char**)calloc(sizeof(char*), tFixed->nfields);
	tFixed->fforms = (char**)calloc(sizeof(char*), tFixed->nfields);
	tFixed->funits = (char**)calloc(sizeof(char*), tFixed->nfields);

	if (tFixed->fnames == NULL || tFixed->fforms == NULL || tFixed->funits == NULL) {
		free_table_specs(tFixed);
		free_table_specs(t);
		parse_error("Unable to alloc memory for fixed table specs.\n");
		return 0;
	}

	j = 0;
	for (i = 0; i < t->nfields; i++) {
		if (colExists[i] == 1) {
			tFixed->fnames[j] = null_safe_strdup(t->fnames[i]);
			tFixed->fforms[j] = null_safe_strdup(t->fforms[i]);
			tFixed->funits[j] = null_safe_strdup(t->funits[i]);
			j++;
		}
	}

	return 1;
}
int fits_tbl_type_for_column_var(Var* coldata)
{
	switch (V_TYPE(coldata)) {
	case ID_STRING: return TSTRING;
	case ID_TEXT: return TSTRING;
	case ID_VAL:
		switch (V_FORMAT(coldata)) {
		case BYTE: return TBYTE;
		case SHORT: return TSHORT;
		case INT: return TINT;
		case FLOAT: return GetX(coldata) > 1 ? TCOMPLEX : TFLOAT;
		case DOUBLE: return GetX(coldata) > 1 ? TDBLCOMPLEX : TDOUBLE;
		}
	}
	return -1;
}

int get_rows(Var* v)
{
	switch (V_TYPE(v)) {
	case ID_STRING: return 1;
	case ID_TEXT: return V_TEXT(v).Row;
	case ID_VAL: return GetY(v);
	}

	return -1;
}

int Write_FITS_Table(fitsfile* fptr, struct tbl_specs* t, Var* tbldata)
{
	int status           = 0;
	int findStructStatus = 0;
	char ctx[1024];
	char* colname = NULL;
	Var* coldata  = NULL;
	int colnum, i, j, k, x, ctype, nelements, elem_size;
	int* colExists;
	colExists = (int*)calloc(t->nfields, sizeof(int));

	fits_create_tbl(fptr, t->tbltype, t->nrows, t->nfields, t->fnames, t->fforms, t->funits,
	                t->tblname, &status);
	sprintf(ctx, " creating table \"%s\"", t->tblname);
	QUERY_FITS_ERROR(status, ctx, 0);

	for (i = 0; i < t->nfields && tbldata != NULL; i++) {
		colExists[i]     = 1; // we start out with optimism that the cols exist
		colnum           = i + 1;
		findStructStatus = find_struct(tbldata, t->fnames[i], &coldata);
		if (findStructStatus == -1) {
			parse_error("WARNING! Column %s was not found, this is unexpected!\n", t->fnames[i]);
			colExists[i] = 0;
			continue;
		}

		if ((ctype = fits_tbl_type_for_column_var(coldata)) < 0) {
			parse_error(
			    "WARNING! Skipping unhandled data type \"%s\" of column \"%s\" in table \"%s\"\n",
			    Format2Str(V_FORMAT(coldata)), t->fnames[i], t->tblname);
			colExists[i] = 0;
			continue;
		}
		nelements = get_rows(coldata);

		if (V_TYPE(coldata) == ID_STRING) {
			fits_write_col(fptr, ctype, colnum, 1, 1, 1, V_STRING(coldata), &status);
		} else if (V_TYPE(coldata) == ID_TEXT) {
			fits_write_col(fptr, ctype, colnum, 1, 1, nelements, (void*)V_TEXT(coldata).text, &status);
		} else if (V_TYPE(coldata) == ID_VAL) {
			if (V_ORG(coldata) == BSQ) {
				// fast write, all elements in bulk
				fits_write_col(fptr, ctype, colnum, 1, 1, nelements, V_DATA(coldata), &status);
			} else {
				// slow write, one element at a time
				x         = GetX(coldata);
				elem_size = NBYTES(V_FORMAT(coldata));
				for (j = 0; j < nelements; j++) {
					for (k = 0; k < x; k++) {
						fits_write_col(fptr, ctype, colnum, j + 1, k + 1, 1,
						               V_DATA(coldata) + cpos(k, j, 0, coldata) * elem_size, &status);
					}
				}
			}
		} else {
			parse_error(
			    "WARNING! Skipping unhandled type of structure element \"%s\" in table \"%s\"\n",
			    t->fnames[i], t->tblname);
			colExists[i] = 0;
		}
		sprintf(ctx, " writing column %d (\"%s\")", colnum, t->fnames[i]);
		QUERY_FITS_ERROR(status, ctx, 0);
	}
	/*
	 * We may be deleting columns.  If we do delete a column,
	 * the number of columns decreases by 1. So we start at the end
	 * and work toward the beginning.
	 */
	for (i = (t->nfields) - 1; i >= 0; i--) {
		if (colExists[i] == 0) {
			colnum = i + 1;
			fits_delete_col(fptr, colnum, &status);
			sprintf(ctx, " deleting column %d (\"%s\")", colnum, t->fnames[i]);
			QUERY_FITS_ERROR(status, ctx, 0);
		}
	}

	return 1;
}

Var* Read_FITS_Table(fitsfile* fptr)
{
	long nrows = 0, repeat, width;
	int ncols = 0, status = 0, col_status = 0;
	char colname[1024];
	int colnum = 0, fmt;
	char *ptr, **pptr, **qqptr;
	Var *data = NULL, *element;
	int coltype, datatype;
	int x, y, z, i, j, k;
	char msg[1024];

	fits_get_num_rows(fptr, &nrows, &status); // drd get number of rows, nrows
	QUERY_FITS_ERROR(status, " getting table rows", NULL);
	fits_get_num_cols(fptr, &ncols, &status);
	QUERY_FITS_ERROR(status, " getting table columns", NULL); // drd get number of cols, ncols

	fits_get_colname(fptr, CASEINSEN, "*", colname, &colnum, &col_status);
	while (col_status != COL_NOT_FOUND) {
		if (data == NULL) {
			data = new_struct(0);
		}

		x    = 0;
		y    = 0;
		z    = 0;
		pptr = NULL;
		ptr  = NULL;

		// fits_get_coltype(fptr, colnum, &coltype, &repeat, &width, &status);
		fits_get_eqcoltype(fptr, colnum, &coltype, &repeat, &width,
		                   &status); // drd this returns the 'equivalent' type of a col needed to
		                             // actually store the value
		sprintf(msg, " getting column %d type", colnum);
		QUERY_FITS_ERROR(status, msg, NULL);
		switch (coltype) {
		// ASCII tables limited to TSTRING, TSHORT, TLONG, TFLOAT
		case TSTRING:
			datatype = TSTRING;
			fmt      = ID_STRING;
			x        = repeat == 1 ? 1 : repeat / width;
			y        = nrows;
			z        = 1;
			pptr     = (char**)calloc(nrows, sizeof(char*));
			for (i = 0; i < nrows; i++) {
				pptr[i] = calloc(x * width + 1, sizeof(char));
			}
			ptr = (char*)pptr;

			// alloc pointers in the above pptr arrays to
			// offsets where individual columns would live
			qqptr = (char**)calloc(nrows * x, sizeof(char*));
			for (i = 0; i < (nrows * x); i++) {
				qqptr[i] = pptr[i / x] + width * (i % x);
			}
			break;

		case TSHORT:
			datatype = TSHORT;
			fmt      = SHORT;
			x        = repeat;
			y        = nrows;
			z        = 1;
			ptr      = calloc(x * y * z, NBYTES(fmt));
			break;

		case TFLOAT:
			datatype = TFLOAT;
			fmt      = FLOAT;
			x        = repeat;
			y        = nrows;
			z        = 1;
			ptr      = calloc(x * y * z, NBYTES(fmt));
			break;

		case TDOUBLE:
			datatype = TDOUBLE;
			fmt      = DOUBLE;
			x        = repeat;
			y        = nrows;
			z        = 1;
			ptr      = calloc(x * y * z, NBYTES(fmt));
			break;

		case TLOGICAL:
		case TBYTE:
			datatype = TBYTE;
			fmt      = BYTE;
			x        = repeat;
			y        = nrows;
			z        = 1;
			ptr      = calloc(x * y * z, NBYTES(fmt));
			break;

		case TLONG:
		case TINT:
			datatype = TINT;
			fmt      = INT;
			x        = repeat;
			y        = nrows;
			z        = 1;
			ptr      = calloc(x * y * z, NBYTES(fmt));
			break;

		case TCOMPLEX:
			datatype = TFLOAT;
			fmt      = FLOAT;
			x        = repeat;
			y        = nrows;
			z        = 2;
			ptr      = calloc(x * y * z, NBYTES(fmt));
			break;

		case TDBLCOMPLEX:
			datatype = TDOUBLE;
			fmt      = DOUBLE;
			x        = repeat;
			y        = nrows;
			z        = 2;
			ptr      = calloc(x * y * z, NBYTES(fmt));
			break;

		case TBIT:
		default:
			parse_error("Ignoring column \"%s\" of unhandled type %d\n", colname, coltype);
			datatype = 0;
			fmt      = 0;
			break;
		}

		if (ptr != NULL) {
			// parse_error("Reading col %d (\"%s\") of coltype \"%s\" (datatype \"%s\") of dim
			// %dx%dx%d\n",
			//    colnum, colname, get_col_type_name(coltype), get_col_type_name(datatype), x, y,
			//    z);

			if (datatype == TSTRING) {
				fits_read_col(fptr, datatype, colnum, 1, 1, x * y, NULL, qqptr, NULL, &status);
				sprintf(msg, " reading column %d data", colnum);
				QUERY_FITS_ERROR(status, msg, NULL);
				element = newText(nrows, pptr);
				free(qqptr);
			} else {
				fits_read_col(fptr, datatype, colnum, 1, 1, x * y * z, NULL, ptr, NULL, &status);
				sprintf(msg, " reading column %d data", colnum);
				QUERY_FITS_ERROR(status, msg, NULL);
				element = newVal(BSQ, x, y, z, fmt, ptr);
			}
			add_struct(data, colname, element);
		}

		// get next column - status must be set to COL_NOT_UNIQUE to get next column
		fits_get_colname(fptr, CASEINSEN, "*", colname, &colnum, &col_status);
	}

	return (data);
}

Var* makeVarFromFITSLabel(char* fits_value, char key_type)
{
	int* i;
	float* f;

	Var* v;

	switch (key_type) {

	case 'C': v = newString(strdup(fits_value)); break;

	case 'I':
		i  = (int*)calloc(1, sizeof(int));
		*i = atoi(fits_value);
		v  = newVal(BSQ, 1, 1, 1, INT, i);
		break;
	case 'L':
		i = (int*)calloc(1, sizeof(int));
		if (!strcmp(fits_value, "T"))
			*i = 1;
		else
			*i = 0;
		v      = newVal(BSQ, 1, 1, 1, INT, i);
		break;

	case 'F':
		f  = (float*)calloc(1, sizeof(float));
		*f = atof(fits_value);
		v  = newVal(BSQ, 1, 1, 1, FLOAT, f);
		break;

	case 'X':
		parse_error("Unclear how to parse this...keeping as string");
		v = newString(strdup(fits_value));
		break;
	}

	return (v);
}

void Parse_Name(char* card, char** name)
{

	char* p;
	char* in = strdup(card);

	if (strlen(in) > 80) {
		*name = NULL;
		free(in);
		return;
	}

	if (!(p = strstr(in, "="))) { // no key/value pair...toss it for now
		*name = NULL;
		free(in);
		return;
	}

	if (p - in >
	    8) { // yeah, it's an ='s sign, but deep in line, probably a comment...toss it for now
		*name = NULL;
		free(in);
		return;
	}

	*p = '\0';

	// Need to trim name:

	p--; // _=_ should put us at first _ location

	while (*p == ' ' && p != in) p--;
	p++;
	*p    = '\0';
	*name = strdup(in);
	free(in);
}

/*
 ** FITS file entry function.  This provides davinci with
 ** an API (well, one function anyway ;) to read FITS type files
 ** and can be called from ANY internal davinci function.
 ** The only parameter it takes is legal FITS type filename
 ** which is a path, a file name and an option extension
 ** string which is concatined onto the filename ([] encloses
 ** the extension).  A return value of NULL indicates a read
 ** failutre.  Other-wise, a Var * to a structure containing
 ** the FITS data is returned.
 */
Var* FITS_Read_Entry(char* fits_filename)
{
	fitsfile* fptr;
	char header_entry[FLEN_CARD];
	int status = 0;
	int i, j, qq;
	int num_objects;
	int cur_object;
	int num_header_entries;
	int object_type;
	char* name;
	char fits_value[128], fits_comment[128];

	Var* head = new_struct(0);
	Var* sub;
	Var* davinci_data;

	char obj_name[64];
	char key_type;

	fits_open_file(&fptr, fits_filename, READONLY, &status);
	fits_get_num_hdus(fptr, &num_objects, &status); // drd get number of hdu's in file

	for (i = 1; i <= num_objects; i++) { // stupid jerks think this is fortran!

		fits_movabs_hdu(fptr, i, &object_type, &status); // drd move to hdu, first is 1

		fits_get_hdrspace(
		    fptr, &num_header_entries, NULL,
		    &status); // drd return number of keywords (keyword/data pairs) not counting "END"

		sub = new_struct(0);
		sprintf(obj_name, "object_%d", i);
		for (j = 1; j <= num_header_entries; j++) {
			fits_read_record(fptr, j, header_entry,
			                 &status); // drd read the j'th record, starts at 1

			// Parse Record for the name
			Parse_Name(header_entry,
			           &name); // drd parse name from keyname = value pair, returns NULL if no '='
			if (name == NULL)  // no ='s found ... toss it for now
				continue;

			strcpy(fits_value, "");
			strcpy(fits_comment, "");
			fits_parse_value(header_entry, fits_value, fits_comment,
			                 &status); // drd input is the key (header_entry), returns fits_value or
			                           // NULL, returns comment or NULL
			if (strcmp("HISTORY", name) == 0 || strcmp("COMMENT", name) == 0) {
				key_type = 'C';
				strcpy(fits_value, fits_comment); // comments have no value
			} else {
				fits_get_keytype(fits_value, &key_type,
				                 &status); // drd input is the fits_value, returns the type
				                           // 'C', 'L', 'I', 'F' or 'X',
				                           // for character string, logical,
				                           // integer, floating point, or complex, respectively.
			}
			QUERY_FITS_ERROR(status, NULL, NULL);

			// Add to sub
			add_struct(sub, name, makeVarFromFITSLabel(fits_value, key_type));

			// Get extension name
			if (strcmp(KW_EXT_NAME, name) == 0) {

				ltrim(fits_value, "'");
				rtrim(fits_value, "'");
				strcpy(obj_name, fits_value);
			}
		}

		// Read data : we'll need a data type switch
		fits_get_hdu_type(fptr, &object_type, &status); // drd IMAGE_HDU, ASCII_TBL, or BINARY_TBL

		QUERY_FITS_ERROR(status, " getting HDU type", NULL);

		if (object_type == IMAGE_HDU)
			davinci_data = Read_FITS_Image(fptr);

		else if (object_type == ASCII_TBL || object_type == BINARY_TBL)
			davinci_data = Read_FITS_Table(fptr);

		else {
			parse_error(
			    "Unknown data object in fits file at HDU location: %d\nSkipping data portion", i);
			davinci_data = NULL;
		}

		// Add to sub
		if (davinci_data) add_struct(sub, SUBEL_DATA, davinci_data);

		// Add sub to head
		/*
		 * drd I had put in "DATATABL", but what if
		 * there are more HDU's?
		 * May revisit this later
		 */
		// drd fixed name of "DATATABL" instead of object_2
		// if (strcmp(obj_name, "object_2") == 0) {
		//	strcpy(obj_name, "DATATABL");
		//}

		/*
		 *  Bug 2224 - Problems in adding sub to head in dvio_fits.c
		 *  The next two drd Note entries
		 */

		/*
		 *
		 * drd Note
		 * An earlier developer found only examples where they were
		 * removing "'" marks here.
		 * I found some where the obj_names were shorter than 8 chars and
		 * had trailing spaces
		 *
		 */
		unquote_remove_spaces(obj_name);
		/*
		 * drd Note
		 * Then I found cases where obj_name had '.'s
		 * Davinci uses the '.'s as structure element delimiters
		 */
		for (qq = 0; qq < strlen(obj_name); qq++) {
			if (obj_name[qq] == '.') {
				obj_name[qq] = '_';
			}
		}

		add_struct(head, obj_name, sub);
	}

	fits_close_file(fptr, &status);

	return (head);
}
int VarType2FitsType(Var* obj, int* bitpix, int* datatype)
{
	switch (V_FORMAT(obj)) {
	case BYTE:
		*bitpix   = BYTE_IMG;
		*datatype = TBYTE;
		break;

	case SHORT:
		*bitpix   = SHORT_IMG;
		*datatype = TSHORT;
		break;

	case INT:
		*bitpix   = LONG_IMG; /* Yeah, I know, it says long...but it means 32-bit */
		*datatype = TINT;     /*Future's so bright...*/
		break;

	case FLOAT:
		*bitpix   = FLOAT_IMG;
		*datatype = TFLOAT;
		break;

	case DOUBLE:
		*bitpix   = DOUBLE_IMG;
		*datatype = TDOUBLE;
		break;

	default: return (1);
	}

	return (0);
}

void ScratchFITS(fitsfile* fptr, char* name)
{
	int status = 0;

	fits_close_file(fptr, &status);

	unlink(name);
}

/*
 ** Valid Object:
 ** This method takes a structure and checks for a minimum of exiting
 ** key/value pairs that would make the structure a legal label for
 ** a FITS object in a FITS file.  The int idx flags the function
 ** as to whether this is the FIRST object to go into the FITS file
 ** and therefore as some different rule checks or if its > 1st object.
 */

int ValidObject(Var* obj, int idx)
{
	return (1);
}

int Write_FITS_Record(fitsfile* fptr, Var* obj, char* obj_name)
{
	char key[9];
	char val[72];
	char card[81];
	int status = 0;
	int i;
	int len = strlen(obj_name);
	int datatype;
	int bitpix;

	/*
	 ** Currently we deal with two davinci object types:
	 **   ID_STRING
	 **   ID_VAL (of dim 1,1,1; ie Byte, Short, etc...)
	 **
	 ** If it's a string, we have to build an 80 character "card".
	 ** If it's a value, we'll let the library handle formating.
	 */

	if (V_TYPE(obj) == ID_STRING) {
		for (i = 0; i < 8; i++) {
			if (i < len)
				key[i] = obj_name[i];
			else
				key[i] = ' ';
		}
		key[8] = '\0';

		// This is currently a mindless <72 char copy...no multi-line possibilities at this point
		strncpy(val, V_STRING(obj), 70);

		val[71] = '\0'; // just in case

		sprintf(card, "%s= %s", key, val);
		// drd printf("The card: %s\n", card);

		fits_write_record(fptr, card, &status);
		QUERY_FITS_ERROR(status, NULL, 0);

		return (1);
	}

	if (V_TYPE(obj) == ID_VAL) {
		if (!strcasecmp(obj_name, "simple")) // special case for the first entry for the first obj
			datatype = TLOGICAL;
		else
			VarType2FitsType(obj, &bitpix, &datatype);

		fits_write_key(fptr, datatype, obj_name, V_DATA(obj), NULL, &status);
		QUERY_FITS_ERROR(status, NULL, 0);

		return (1);
	}

	return (1);
}

/*
 ** WriteSingleStructure:
 ** This function takes a Var structure, and parses it
 ** using it's members names for the FITS label keys
 ** and member values for the key's values.  If the item
 ** contains a member named 'data', it is used to create
 ** an image object with all the associated bit/axes information.
 ** Before the object is parsed and written out the FITS
 ** files, it is validated for minimal label content and correctness.
 ** If validation fails, a non-zero value is returned, other-wise
 ** the structure is written out and a zero value is returned.
 */

/*
 * Bug 2218 - Davinci does not quite correctly add items to a FITS file
 *
 * Code did not account for the differences between the first (primary) header
 * and added secondary items.  This has been remedied.  Text Tables, Binary Tables,
 * even image files can now be added to davinci FITS structures,
 * and these can be written out. Note that Davinci strips comments and history.
 * Key/Value pairs are retained.
 * Calls to the cfitsio library does add "automatic" comment and history information
 *
 */
int WriteSingleStructure(fitsfile* fptr, Var* obj, int index)
{
	Var *element        = NULL, *d;
	static Var* lastVar = NULL;
	int i;
	static int wroteDataItem = 0;
	static int headerOnly    = 0;
	int count;
	char* obj_name;
	int has_data_subel = 0;
	int has_tbl_ext    = 0;
	char ext_type[512];
	struct tbl_specs t;
	struct tbl_specs tFixed;
	static int xx  = 0;
	int hduType    = 0;
	int hduStatus  = 0;
	int numObjects = 0;
	char msg[1024];

	if (!ValidObject(obj, index)) return (1);

	// determine if the structure contains image / table data
	has_data_subel = (find_struct(obj, SUBEL_DATA, &element) >= 0);

	strcpy(ext_type, GET_KEY_VAL_STRING(obj, KW_EXT, d, ""));
	unquote_remove_spaces(ext_type);
	has_tbl_ext =
	    (strcasecmp(ext_type, VAL_EXT_TABLE) == 0 || strcasecmp(ext_type, VAL_EXT_BINTABLE) == 0);

	// write the data object first
	if (has_data_subel || has_tbl_ext) {
		if (has_tbl_ext) {
			if (!collect_table_specs(obj, &t)) return (1);

			if (!adjust_table_specs(element, &t, &tFixed)) return (1);

			if (!Write_FITS_Table(fptr, &tFixed, element)) {
				free_table_specs(&t);
				free_table_specs(&tFixed);
				return (1);
			} else {
				wroteDataItem = 1;
			}

			free_table_specs(&t);
			free_table_specs(&tFixed);
		} else {
			if (!(int)Write_FITS_Image(fptr, element)) {
				return (1);
			} else {
				wroteDataItem = 1;
			}
		}
	} else {
		headerOnly = 1;
	}

	if (wroteDataItem == 1) { // Get the rest of the header items that are not automatically put in
		count = get_struct_count(obj);
		for (i = 0; i < count; i++) {
			get_struct_element(obj, i, &obj_name, &element);
			if (strcasecmp(obj_name, SUBEL_DATA) != 0) {
				if (!filter_kw(obj_name)) {
					if (!(int)Write_FITS_Record(fptr, element, obj_name)) {
						return (1);
					}
				}
			}
		}
		fits_get_num_hdus(fptr, &numObjects, &hduStatus); // drd how many in there?
		sprintf(msg, " number of HDU's %d", numObjects);
		QUERY_FITS_ERROR(hduStatus, msg, 0);

		if ((numObjects > 1) && (lastVar != NULL) && (headerOnly == 1) &&
		    (wroteDataItem == 1)) { // Fix up the primary header, get stuff not automatically put in
			headerOnly = 0;
			fits_movabs_hdu(fptr, numObjects - 1, &hduType, &hduStatus);
			sprintf(msg, " moving to HDU index %d", index);
			QUERY_FITS_ERROR(hduStatus, msg, 0);
			count = get_struct_count(lastVar);
			for (i = 0; i < count; i++) {
				get_struct_element(lastVar, i, &obj_name, &element);
				if (strcasecmp(obj_name, SUBEL_DATA) != 0) {
					if (!filter_kw(obj_name)) {
						if (!(int)Write_FITS_Record(fptr, element, obj_name)) {
							return (1);
						}
					}
				}
			}
		}
	}
	wroteDataItem = 0;
	lastVar       = obj;
	return (0);
}

/*
 ** This is one entry point for creating a FITS file.
 ** This function takes a davinci structure as input.
 ** The object is first validated to ensure it contains
 ** the minimum label information needed for a FITS file.
 ** If the structure successfully passes validation, its
 ** contents are translated.  More than one
 ** data object can be contained in the structure.  For
 ** each data object and associated information in the
 ** structure a FITS object and associated label will
 ** be generated in the file.
 */

Var* FITS_Write_Structure(char* fits_filename, Var* obj, int force)
{
	int i;
	Var* Tmp;
	int count;
	char* name;
	char* obj_name;
	fitsfile* fptr;
	int status = 0;
	char msg[1024];
	int hdutype;

	/*
	 ** All incoming FITS object are themselves placed into a structure, thus
	 ** obj should be structure of structures (at least one, anyway).  The first
	 ** portion of validation is to check that each object in obj IS of type
	 ** ID_STRUCT.  If it is, a single structure writer is called.
	 ** Slightly different rules exist for the first structure as opposed to the
	 ** rest, so that fact (whether this obj is the first or not the first) is
	 ** is signaled in the parameter list.  If any structure fails validation,
	 ** the file is scratched and nothing is written.
	 */

	if (force) {
		name = (char*)calloc(strlen(fits_filename) + 2, 1);
		strcpy(name, "!");
		strcat(name, fits_filename); /* the '!'-prefix tells the cfitsio
		                                lib to overwrite existing file */
	} else
		name = strdup(fits_filename);

	fits_create_file(&fptr, name, &status);
	sprintf(msg, " creating file %s", name);
	QUERY_FITS_ERROR(status, msg, NULL);

	count = get_struct_count(obj);

	for (i = 0; i < count; i++) {
		get_struct_element(obj, i, &obj_name, &Tmp);

		if (V_TYPE(Tmp) != ID_STRUCT) {
			parse_error("Encountered a non-FITS item in the list of items: %s\n", obj_name);
			ScratchFITS(fptr, name);
			return (NULL);
		}

		// fits_movabs_hdu(fptr, i+1, &hdutype, &status);
		// sprintf(msg," moving to HDU index %d", i+1); QUERY_FITS_ERROR(status,msg,NULL);

		if (WriteSingleStructure(fptr, Tmp, i)) {
			parse_error(
			    "Invalid items in structure labeled: %s\nCannot write this object as a FITS file\n",
			    obj_name);
			ScratchFITS(fptr, name);
			return (NULL);
		}
	}

	fits_close_file(fptr, &status);

	sprintf(msg, " closing file %s", name);
	QUERY_FITS_ERROR(status, msg, NULL);

	return (NULL);
}

/* This is the second entry for creating a FITS file.
 **	This function is called is the davinci object to
 ** written out is NOT a structure, but is instead a
 ** Var or cube object.  If this is the case, the
 ** minimum label information needed for a FITS object
 ** is generated, and the VAR is written out as the
 ** associated data.  This routine produces only a single
 ** FITS object within the FITS file.
 */

/*
 ** Thu Nov 11 15:19:16 MST 2004:
 **		We currently only output image/cube objects...no tables
 */

Var* FITS_Write_Var(char* fits_filename, Var* obj, int force)
{
	char* name;
	int status = 0;
	fitsfile* fptr;
	int naxis;
	long naxes[3];
	int bitpix;
	int datatype;
	int* size;
	int i;
	int fpixel[3] = {1, 1, 1};
	char msg[1024];

	if (V_ORG(obj) != BSQ) {
		parse_error("Only BSQ ordered objects can be written out");
		return (NULL);
	}

	if (force) {
		name = (char*)calloc(strlen(fits_filename) + 2, 1);
		strcpy(name, "!");
		strcat(name, fits_filename); /* the '!'-prefix tells the cfitsio
		 lib to overwrite existing file */
	} else
		name = strdup(fits_filename);

	fits_create_file(&fptr, name, &status);
	sprintf(msg, " creating file %s", name);
	QUERY_FITS_ERROR(status, msg, NULL);

	/*
	 ** Here we would decide if the Var was a table
	 ** or an image, and then do the right thing, but
	 ** for now, we only do images.
	 */

	if (!(int)Write_FITS_Image(fptr, obj)) {
		parse_error("An error was generated trying to write your data as a FITS image\n");
		ScratchFITS(fptr, name);
		return (NULL);
	}

	fits_close_file(fptr, &status);

	sprintf(msg, " closing file %s", name);
	QUERY_FITS_ERROR(status, msg, NULL);

	return (NULL);
}

/*
 ** Davinci wrapper function.
 ** This function takes an object, either a structure
 ** or a Var and write out a FITS object.  Different
 ** entry point routines are called depending
 ** on the objects type (Var or Structure).  Any
 ** other type is not valid, and is rejected.
 */

Var* WriteFITS(vfuncptr func, Var* arg)
{
	Var* obj       = NULL;
	char* filename = NULL;
	int force      = 0;

	char* name;

	Alist alist[4];
	alist[0]      = make_alist("obj", ID_UNK, NULL, &obj);
	alist[1]      = make_alist("filename", ID_STRING, NULL, &filename);
	alist[2]      = make_alist("force", INT, NULL, &force);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (obj == NULL) {
		parse_error("No object specified.");
		return (NULL);
	}

	if (filename == NULL) {
		parse_error("No filename specified.");
		return (NULL);
	}

	if (V_TYPE(obj) == ID_VAL)
		return (FITS_Write_Var(filename, obj, force));

	else if (V_TYPE(obj) == ID_STRUCT)
		return (FITS_Write_Structure(filename, obj, force));

	else
		parse_error("You have submitted an invalid object to be written out as a FITS file");

	return (NULL);
}

/*
 ** Davinci wrapper function.
 ** This function receives a filename with an optional extension.
 ** If extension is used, it is concatinated onto the filename
 ** which is how the cfitsio library expects it to be.
 ** The final filename is used to call the entry function.
 ** This function returns NULL if the read failed or a Var *
 ** pointing to the structure representing the contents
 ** of the fits_file.
 */
Var* ReadFITS(vfuncptr func, Var* arg)
{
	char* filename  = NULL;
	char* extension = NULL;
	char* fe        = NULL;
	Var* data       = NULL;

	Alist alist[3];
	alist[0]      = make_alist("filename", ID_STRING, NULL, &filename);
	alist[1]      = make_alist("extension", ID_STRING, NULL, &extension);
	alist[2].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (filename == NULL) {
		parse_error("Expected filename");
		return (NULL);
	}

	fe = dv_locate_file(filename);
	if (extension) {
		char* fetmp = (char*)calloc(strlen(fe) + strlen(extension) + 1, 1);
		strcpy(fetmp, fe);
		strcat(fetmp, extension);
		free(fe);
		fe = fetmp;
	}

	data = FITS_Read_Entry(fe);
	free(fe);

	if (!data) {
		parse_error("%s: Failed to load %s%s\n", func->name, filename, (extension ? extension : ""));
		return (NULL);
	}

	return (data);
}

#endif
