#include "parser.h"
/**
 ** This is a simple ASCII output function
 **/

int WriteAscii(Var* s, char* filename, int force)
{
	size_t dsize;
	int format;
	int ival;
	double dval;
	int i, j, k;
	int x, y, z;
	size_t pos;
	FILE* fp;

	if (!force && file_exists(filename)) {
		parse_error("File %s already exists.", filename);
		return 0;
	}

	if ((fp = fopen(filename, "w")) == NULL) {
		parse_error("Unable to open file: %s\n", filename);
		return 0;
	}

	if (V_TYPE(s) == ID_STRING) {
		fwrite(V_STRING(s), strlen(V_STRING(s)), 1, fp);
		fclose(fp);
		return (1);
	} else if (V_TYPE(s) == ID_TEXT) {
		char cr = '\n';
		for (i = 0; i < V_TEXT(s).Row; i++) {
			fwrite(V_TEXT(s).text[i], sizeof(char), strlen(V_TEXT(s).text[i]), fp);
			fwrite(&cr, sizeof(char), 1, fp);
		}
		fclose(fp);
		return (1);
	}

	dsize  = V_DSIZE(s);
	format = V_FORMAT(s);

	x = GetX(s);
	y = GetY(s);
	z = GetZ(s);

	for (k = 0; k < z; k++) {
		if (k) fputc('\n', fp);
		for (j = 0; j < y; j++) {
			for (i = 0; i < x; i++) {
				if (i) fputc('\t', fp);
				pos = cpos(i, j, k, s);
				switch (format) {
				case DV_UINT8:
				case DV_INT16:
				case DV_INT32:
					ival = extract_int(s, pos);
					fprintf(fp, "%d", ival);
					break;
				case DV_FLOAT:
				case DV_DOUBLE:
					dval = extract_double(s, pos);
					fprintf(fp, "%.10g", dval);
					break;
				}
			}
			fputc('\n', fp);
		}
	}
	fclose(fp);
	return (1);
}


// NOTE(rswinkle): used in ff.c and ff_text.c
int dv_getline(char** ptr, FILE* fp)
{
	static char* line = NULL;
	static size_t len = 0;

	if (fp == NULL && line != NULL) {
		free(line);
		line = NULL;
		*ptr = NULL;
		return 0;
	}

	if (line == NULL) {
		len  = 8192;
		line = (char*)malloc(len + 1);
	}

	if (fgets(line, len, fp) == NULL) {
		*ptr = NULL;
		return (-1);
	}
	while (strchr(line, '\n') == NULL && strchr(line, '\r') == NULL) {
		line = (char*)my_realloc(line, len * 2 + 1);
		if ((fgets(line + len - 1, len, fp)) == NULL) break;
		len = len * 2 - 1;
		if (len > 1000000) {
			fprintf(stderr, "Line is at 1000000\n");
			*ptr = NULL;
			return -1;
		}
	}
	*ptr = line;
	return (strlen(line));
}

Var* ff_ascii(vfuncptr func, Var* arg)
{
	char* filename = NULL;
	Var* s         = NULL;
	char* fname    = NULL;
	FILE* fp       = NULL;
	char* ptr      = NULL;
	int rlen;

	size_t count         = 0;
	const char* delim    = " \t";
	int i, j, k;
	size_t dsize;
	int x                 = 0;
	int y                 = 0;
	int z                 = 0;

	// default format if format_not given
	int format            = DV_INT32;

	int column            = 0;
	int row               = 0;
	char* format_str      = NULL;

	void* data           = NULL;
	u8* u8data;
	u16* u16data;
	u32* u32data;
	u64* u64data;

	i8* i8data;
	i16* i16data;
	i32* i32data;
	i64* i64data;

	float* fdata;
	double* ddata;

	Alist alist[9];
	alist[0]      = make_alist("filename", ID_STRING, NULL, &filename);
	alist[1]      = make_alist("x", DV_INT32, NULL, &x);
	alist[2]      = make_alist("y", DV_INT32, NULL, &y);
	alist[3]      = make_alist("z", DV_INT32, NULL, &z);
	alist[4]      = make_alist("format", ID_ENUM, FORMAT_STRINGS, &format_str);
	alist[5]      = make_alist("column", DV_INT32, NULL, &column);
	alist[6]      = make_alist("row", DV_INT32, NULL, &row);
	alist[7]      = make_alist("delim", ID_STRING, NULL, &delim);
	alist[8].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	if (filename == NULL) {
		parse_error("No filename specified: %s()", func->name);
		return (NULL);
	}

	// format_str will never be invalid because parse_args would have failed
	// so format will never be set to -1;
	if (format_str) {
		format = dv_str_to_format(format_str);
	}

	// Got all the values.  Do something with 'em.

	if ((fname = dv_locate_file(filename)) == NULL) {
		sprintf(error_buf, "Cannot find file: %s\n", filename);
		parse_error(NULL);
		return (NULL);
	}
	if ((fp = fopen(fname, "r")) == NULL) {
		sprintf(error_buf, "Cannot open file: %s\n", fname);
		parse_error(NULL);
		return (NULL);
	}
	if (x == 0 && y == 0 && z == 0) {
		// User wants us to determine the file size.
		z = 1;

		// skip some rows
		for (i = 0; i < row; i++) {
			dv_getline(&ptr, fp);
			if (ptr == NULL) {
				fprintf(stderr, "Early EOF, aborting.\n");
				return (NULL);
			}
		}

		while (dv_getline(&ptr, fp) != EOF) {
			if (ptr[0] == '\n') {
				z++;
				continue;
			}

			while ((ptr = strtok(ptr, delim)) != NULL) {
				count++;
				ptr = NULL;
			}
			y++;
		}

		if (z == 0) z = 1;
		if (y == 0) y = 1;
		y             = y / z;
		x             = count / y / z;

		if (x * y * z != count) {
			fprintf(stderr, "Unable to determine file size.\n");
			return (NULL);
		}
		if (VERBOSE) fprintf(stderr, "Apparent file size: %dx%dx%d\n", x, y, z);
		rewind(fp);
	}

	// Decode file with specified X, Y, Z.
	if (x == 0) x           = 1;
	if (y == 0) y           = 1;
	if (z == 0) z           = 1;

	dsize = (size_t)x * y * z;
	data  = calloc(NBYTES(format), dsize);

	u8data = data;
	u16data = data;
	u32data = data;
	u64data = data;

	i8data = data;
	i16data = data;
	i32data = data;
	i64data = data;

	fdata = data;
	ddata = data;

	count = 0;

	/**
	 ** Skip N rows.
	 **/
	for (i = 0; i < row; i++) {
		dv_getline(&ptr, fp);
		if (ptr == NULL) {
			fprintf(stderr, "Early EOF, aborting.\n");
			return (NULL);
		}
	}
	for (k = 0; k < z; k++) {
		if (k) {
			// skip to end of block
			while (dv_getline(&ptr, fp) > 1)
				;
		}

		for (j = 0; j < y; j++) {
			if ((rlen = dv_getline(&ptr, fp)) == -1) break;

			// skip columns
			for (i = 0; i < column; i++) {
				ptr = strtok(ptr, delim);
				ptr = NULL;
			}

			// read X values from this line
			for (i = 0; i < x; i++) {
				ptr = strtok(ptr, delim);
				if (ptr == NULL) {
					fprintf(stderr, "Line too short\n");
					count += x - i;
					break;
				}

				// NOTE(rswinkle): the clamps on the 32 and especially 64 bit types are unnecessary
				// and pointless since they're limited by the strto* function in the first place
				// but I left them in for the sake of uniformity.
				switch (format) {
				case DV_UINT8: u8data[count++]  = clamp_u8(strtoul(ptr, NULL, 10)); break;
				case DV_UINT16: u16data[count++]  = clamp_u16(strtoul(ptr, NULL, 10)); break;
				case DV_UINT32: u32data[count++]  = clamp_u32(strtoul(ptr, NULL, 10)); break;
				case DV_UINT64: u64data[count++]  = clamp_u64(strtoull(ptr, NULL, 10)); break;

				case DV_INT8: i8data[count++]  = clamp_i8(strtol(ptr, NULL, 10)); break;
				case DV_INT16: i16data[count++]  = clamp_i16(strtol(ptr, NULL, 10)); break;
				case DV_INT32: i32data[count++]  = clamp_i32(strtol(ptr, NULL, 10)); break;
				case DV_INT64: i64data[count++]  = clamp_i64(strtoll(ptr, NULL, 10)); break;

				case DV_FLOAT: fdata[count++]  = strtod(ptr, NULL); break;
				case DV_DOUBLE: ddata[count++] = strtod(ptr, NULL); break;
				}
				ptr = NULL;
			}
		}
		if (rlen == -1) {
			fprintf(stderr, "Early EOF\n");
			break;
		}
	}

	if (VERBOSE > 1) {
		fprintf(stderr, "Read ASCII file: %dx%dx%d\n", x, y, z);
	}

	fclose(fp);

	s            = newVar();
	V_TYPE(s)    = ID_VAL;
	V_DATA(s)    = data;
	V_FORMAT(s)  = format;
	V_ORG(s)     = BSQ;
	V_DSIZE(s)   = dsize;
	V_SIZE(s)[0] = x;
	V_SIZE(s)[1] = y;
	V_SIZE(s)[2] = z;

	return (s);
}
