#include "endian_norm.h"
#include "parser.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//#define FF_UNPACK_DEBUG 1

#define ARG_TEMPLATE "template"
#define ARG_FILENAME "filename"
#define ARG_SKIP "skip"
#define ARG_COUNT "count"
#define ARG_COL_NAMES "col_names"

#define ARG_STRUCT "struct"
#define ARG_FORCE "force"

#define STRING 'a'
#define SIGNED_MSB_INT 'I'
#define UNSIGNED_MSB_INT 'U'
#define SIGNED_LSB_INT 'i'
#define UNSIGNED_LSB_INT 'u'

#define LSB_REAL 'r'
#define MSB_REAL 'R'

#define LSB_FLOAT 'f'
#define MSB_FLOAT 'F'
#define LSB_DOUBLE 'd'
#define MSB_DOUBLE 'D'

#define IGNORE 'x'

#define MAX_ATTRIBUTES 500

// filled in parse_template()
typedef struct {
	char type;                 // type constant from above defines
	short bytesize;            // bytesize in file (from template)
	short adj_bytesize;        // bytesize allocated (e.g. +1 for strings for null character
	short columns;             // multiplicity
	unsigned short start_byte; // record offset
	char* col_name;            // generated name used for creating davinci structs
} column_attributes;

// this is passed around to functions for convenience
typedef struct {
	column_attributes* attr; // array of above
	int num_items;           // number of items (i.e. size of attr array)
	int rows;                // calculated number of rows in file based on record size
} unpack_digest;

// array of this is returned from unpack function to ff_unpack
typedef struct {
	column_attributes* input; // address of appropriate element in attr array above
	u8* array;                // array to actually store the column data, NULL if type is string
	char** strarray;          // array to store column data if type is string, NULL otherwise
	short numbytes;           // set equal to adj_bytesize
	char type;                // set equal to final type
} data;

static int convert_types(data* thedata, int num_items, int rows);

static data* unpack(char*, char*, Var*, int, int*, int*);
static unpack_digest* parse_template(char* template, column_attributes* input, Var* column_names, int*);
static int validate_template(unpack_digest* input);
static int read_data(u8*, data*, unpack_digest*, FILE*, int);
static int calc_rows(char*, int, unpack_digest*, int);
static data* allocate_arrays(unpack_digest*);
static void compute_adj_bytes(unpack_digest*);


#ifdef FF_UNPACK_DEBUG
static int print_data(data* the_data, unpack_digest* input);
#endif
//static int upgrade_types(data*, unpack_digest*);

static void clean_up(int, unpack_digest*, u8*, data*, FILE*);

static int pack(Var* struct_to_pack, data* thedata, char** template_in, char* filename, int numData, int rows, int offset);
static data* parse_struct(Var*, Var*, int*, int*);
static data* format_data(data*, unpack_digest*);
static int write_data(u8* buffer, data* the_data, unpack_digest* input, int row);
static void cleanup_data(data*, int);
static int pack_row(data* the_data, unpack_digest* digest, int row, u8* buffer);

/*premade calls for testing:

//unpack("U4U2U2*38U2U2R4I2*9R4R4R4x6a4", "atm05555.dat", 390)
//unpack("U4U2U2*38U2U2x4I2*9x18a4", "atm05555.dat", 390)	//without floats
//unpack("U4U2U2U4UU2UaaaaaaaUUx8U2*4U", "obs05555.dat",420)
//unpack("Iu2i3u4r4r8x4I2I3U4R4R8a5", "mytestfile.dat", 0)
//unpack("Iu2i3u4r4r8x4U2U3U4R4R8a5", "secondtest.dat", 0)
//unpack("Iu2i3u4r4r8x4U2U3U4R4R8a6*3", "strmulttest.dat", 0)

*/

Var* ff_unpack(vfuncptr func, Var* arg)
{
	char* template    = NULL;
	char* filename    = NULL;
	int hdr_length    = 0;
	Var* result       = NULL;
	int rows          = -1;
	Var* column_names = NULL;

	int i = 0;

	Alist alist[6];
	alist[0]      = make_alist(ARG_TEMPLATE, ID_STRING, NULL, &template);
	alist[1]      = make_alist(ARG_FILENAME, ID_STRING, NULL, &filename);
	alist[2]      = make_alist(ARG_SKIP, DV_INT32, NULL, &hdr_length);
	alist[3]      = make_alist(ARG_COUNT, DV_INT32, NULL, &rows);
	alist[4]      = make_alist(ARG_COL_NAMES, ID_UNK, NULL, &column_names);
	alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return NULL;

	if (template == NULL) {
		parse_error("%s not specified: %s()", ARG_TEMPLATE, func->name);
		return NULL;
	}

	if (filename == NULL) {
		parse_error("%s not specified: %s()", ARG_FILENAME, func->name);
		return NULL;
	}

	if (hdr_length < 0) {
		parse_error("%s < 0: %s()", ARG_SKIP, func->name);
		return NULL;
	}

	int num_items  = 0;
	data* reg_data = unpack(template, filename, column_names, hdr_length, &num_items, &rows);

	if (reg_data == NULL || rows <= 0 || num_items <= 0) return NULL;

	/* Converting types to appropriate davinci types (constants ie DV_INT16, DV_UINT8 etc.) */
	if (!convert_types(reg_data, num_items, rows)) {
		free(reg_data);
		return NULL;
	}

	// Creating davinci struct and looping through data creating appropriate substructures
	result = new_struct(0);

	int columns = 0;
	for (i = 0; i < num_items; i++) {
		columns = reg_data[i].input->columns;

		if (reg_data[i].type != STRING)
			add_struct(result, reg_data[i].input->col_name,
			           newVal(BSQ, columns, rows, 1, reg_data[i].type, reg_data[i].array));
		else
			add_struct(result, reg_data[i].input->col_name, newText(rows, reg_data[i].strarray));
	}

	// freeing old data/containers.  Actual data is still allocated and used by new davinci
	// structure
	for (i = 0; i < num_items; i++) free(reg_data[i].input->col_name);

	free(reg_data);

	return result;
}

/* Convert my type constants to appropriate davinci type constants
 * e.g. SIGNED_LSB_INT && numbytes == 2-> DV_INT16
 */
static int convert_types(data* thedata, int num_items, int rows)
{
	int i = 0;

	for (i = 0; i < num_items; i++) {
		switch (thedata[i].type) {
		case SIGNED_LSB_INT:
		case SIGNED_MSB_INT:
			switch (thedata[i].numbytes) {
			case 1: thedata[i].type = DV_INT8; break;
			case 2: thedata[i].type = DV_INT16; break;
			case 3:
			case 4: thedata[i].type = DV_INT32; break;
			default:
				thedata[i].type = DV_INT64; break;
			}
			break;

		case UNSIGNED_LSB_INT:
		case UNSIGNED_MSB_INT:
			switch (thedata[i].numbytes) {
			case 1: thedata[i].type = DV_UINT8; break;
			case 2: thedata[i].type = DV_UINT16; break;
			case 3:
			case 4: thedata[i].type = DV_UINT32; break;
			default:
				thedata[i].type = DV_UINT64; break;
			}
			break;

		case LSB_FLOAT:
		case MSB_FLOAT:
			thedata[i].type = DV_FLOAT; break;

		case LSB_DOUBLE:
		case MSB_DOUBLE:
			thedata[i].type = DV_DOUBLE; break;

		case STRING:
			break;

		default:
			parse_error("In convert_types(): Unknown type: %c\n", thedata[i].type);
			return 0;
			break;
		}
	}

	return 1;
}

/**** unpack implementation function ****/
static data* unpack(char* template, char* filename, Var* column_names, int hdr_length,
                    int* numitems, int* ret_rows)
{
	int i, k, x;
	int rec_length, max_buf;

	column_attributes* initial = NULL;
	unpack_digest* input       = NULL;
	FILE* file                 = NULL;
	u8* buffer                 = NULL;
	data* thedata              = NULL;
	void* temp_ptr             = NULL;

	if (strlen(template) == 0) {
		parse_error("Error: Template cannot be the empty string!");
		return NULL;
	}

	initial = (column_attributes*)calloc(MAX_ATTRIBUTES, sizeof(column_attributes));
	if (initial == NULL) {
		memory_error(errno, sizeof(column_attributes) * MAX_ATTRIBUTES);
		return NULL;
	}

	input = parse_template(template, initial, column_names, &rec_length);
	if (input == NULL) {
		free(initial);
		return NULL;
	}

	if (!validate_template(input)) {
		clean_up(0, input, NULL, NULL, NULL);
		return NULL;
	}

	initial = realloc(initial, sizeof(column_attributes) * input->num_items);
	if (initial == NULL) {
		memory_error(errno, sizeof(column_attributes) * input->num_items);
		clean_up(0, input, NULL, NULL, NULL);
		return NULL;
	} else {
		input->attr = initial;
	}

	file = fopen(filename, "rb");
	if (file == NULL) {
		parse_error("%s: %s", "Error opening file", strerror(errno));
		clean_up(0, input, NULL, NULL, NULL);
		return NULL;
	}

	if (fseek(file, hdr_length, SEEK_SET)) {
		parse_error("fseek error: %s", strerror(errno));
		clean_up(0, input, NULL, NULL, file);
		return NULL;
	}
#ifdef FF_UNPACK_DEBUG
	fprintf(stderr, "record length: %d\n\n", rec_length);
#endif

	// calculate number of rows
	if (!calc_rows(filename, hdr_length, input, rec_length)) {
		clean_up(0, input, NULL, NULL, file);
		return NULL;
	}

#ifdef FF_UNPACK_DEBUG
	fprintf(stderr, "calculated: %d rows; user requested: %d rows\n", input->rows, *ret_rows);
#endif
	if (input->rows <= 0) { // calculated negative or zero rows
		parse_error("Error processing file %s: computed file rows(%d)\n", filename, input->rows);
		clean_up(1, input, NULL, NULL, file); // JNN: double check: 1 or 0?
		return NULL;
	}

	if (*ret_rows >= 0) { /* user passed in the number of rows to convert */
		if (*ret_rows > input->rows) {
			parse_error("error processing file %s: requested rows (%d) > computed file rows (%d)\n",
			            filename, *ret_rows, input->rows);
			clean_up(1, input, NULL, NULL, file);
			return NULL;
		} else {
			/* set the number of rows to process to the value passed in */
			input->rows = *ret_rows;
		}
	}

	buffer = (u8*)calloc(rec_length + 2, sizeof(u8)); // not sure if extra 2 are needed but jic
	if (buffer == NULL) {
		memory_error(errno, rec_length * sizeof(u8));
		clean_up(0, input, NULL, NULL, file);
		return NULL;
	}

	compute_adj_bytes(input);

	thedata = allocate_arrays(input);
	if (thedata == NULL) {
		clean_up(1, input, buffer, NULL, file);
		return NULL;
	}

	for (i = 0; i < input->num_items; i++) {
		thedata[i].input    = &input->attr[i];
		thedata[i].type     = input->attr[i].type;
		thedata[i].numbytes = input->attr[i].adj_bytesize;
	}

	for (k = 0; k < input->rows; k++) {
		if (fread(buffer, 1, rec_length, file) != rec_length) {
			parse_error("Error reading data: %s", strerror(errno));
			clean_up(2, input, buffer, thedata, file);
			return NULL;
		}

		if (!read_data(buffer, thedata, input, file, k)) {
			clean_up(2, input, buffer, thedata, file);
			return NULL;
		}
	}

#if 0
	// see comment above upgrade_types
	// change types if necessary ie 3, 5-7 byte int types to appropriate
	if (!upgrade_types(thedata, input)) {
		clean_up(2, input, buffer, thedata, file);
		return NULL;
	}
#endif

#ifdef FF_UNPACK_DEBUG
	print_data(thedata, input);
#endif

	// set these parameters so calling function has them to loop through data
	*numitems = input->num_items;
	*ret_rows = input->rows;

	// cleanup
	free(buffer);
	free(input);
	fclose(file);

	return thedata;
}

/* clean up function called on errors */
static void clean_up(int level, unpack_digest* input, u8* buffer, data* the_data, FILE* file)
{
	int i, j;
	int num_items = input->num_items;
	int rows      = input->rows;

	if (level > 0) {
		free(input->attr);
		free(input);
	}

	if (level > 1) free(buffer);

	if (level > 2) {
		for (i = 0; i < num_items; i++) {

			if (the_data->input->type == STRING) {
				for (j = 0; j < rows; j++) free(the_data[i].strarray[j]);

				free(the_data[i].strarray);

			} else
				free(the_data[i].array);
		}
	}

	if (file) fclose(file);
}

static int validate_template(unpack_digest* input)
{
	int i;

	for (i = 0; i < input->num_items; i++) {
		switch (input->attr[i].type) {

		case STRING: break;

		case SIGNED_MSB_INT:
		case UNSIGNED_MSB_INT:
		case SIGNED_LSB_INT:
		case UNSIGNED_LSB_INT:
			if (input->attr[i].bytesize < 1 || input->attr[i].bytesize > 8) {
				parse_error("Invalid bytesize (%d) for column %d of type %c\nValid values are 1-8",
				            input->attr[i].bytesize, i + 1, input->attr[i].type);
				return 0;
			}
			break;

		case LSB_REAL:
			if (input->attr[i].bytesize == 4)
				input->attr[i].type = LSB_FLOAT;
			else if (input->attr[i].bytesize == 8)
				input->attr[i].type = LSB_DOUBLE;
			else {
				parse_error(
				    "Invalid bytesize (%d) for column %d of type %c\nValid values are 4 and 8",
				    input->attr[i].bytesize, i + 1, input->attr[i].type);
				return 0;
			}
			break;

		case MSB_REAL:
			if (input->attr[i].bytesize == 4)
				input->attr[i].type = MSB_FLOAT;
			else if (input->attr[i].bytesize == 8)
				input->attr[i].type = MSB_DOUBLE;
			else {
				parse_error(
				    "Invalid bytesize (%d) for column %d of type %c\nValid values are 4 and 8",
				    input->attr[i].bytesize, i + 1, input->attr[i].type);
				return 0;
			}
			break;

		default:
			parse_error("Invalid type %c in column %d", input->attr[i].type, i + 1);
			return 0;
			break;
		}
	}
	return 1;
}

static int valid_letter(char letter)
{
	if (letter != STRING && letter != SIGNED_MSB_INT && letter != UNSIGNED_MSB_INT &&
	    letter != SIGNED_LSB_INT && letter != UNSIGNED_LSB_INT && letter != LSB_REAL &&
	    letter != MSB_REAL && letter != IGNORE)
		return 0;
	else
		return 1;
}

/* Parse template string */
static unpack_digest* parse_template(char* template, column_attributes* input, Var* column_names,
                                     int* record_length)
{
	int i, j = 0, j_no_mult = 0, temp = 0, offset = 0, name_length = 30, error = 0;

	char* end           = NULL;
	char* beginning     = template;
	unpack_digest* more = NULL;

	int name_count = 0;
	char** names   = NULL;

	if (column_names != NULL) {
		if (V_TYPE(column_names) == ID_TEXT) {
			name_count = V_TEXT(column_names).Row;
			names      = V_TEXT(column_names).text;
		} else if (V_TYPE(column_names) == ID_STRING) {
			name_count = 1;
			names      = (char**)calloc(1, sizeof(char*));
			if (names == NULL) {
				memory_error(errno, sizeof(char*));
				return NULL;
			}
			names[0] = V_STRING(column_names);
		} else {
			parse_error("Unrecognized type passed in for argument col_names");
			return NULL;
		}
	}

	char* tmp;
	for (i = 0; i < name_count; i++) {
		tmp = fix_name(names[i]);
		free(names[i]);
		names[i] = tmp;
	}

	while (template[0] != '\0') // loop through parsing template
	{
		if (!valid_letter(template[0])) {
			error = 1;
			break;
		}

		input[j].type       = template[0];
		input[j].start_byte = offset;
		input[j].col_name   = NULL;

		if (!isdigit(template[1])) {
			input[j].bytesize = 1;
			template          = &template[1];
		} else {
			template = &template[1];
			temp     = strtol(template, &end, 10);
			if (temp == 0 || template == end) {
				error = 1;
				break;
			}

			input[j].bytesize = temp;
			template          = end;
		}

		if (template[0] == '*') {
			template = &template[1];
			temp     = 0;
			temp     = strtol(template, &end, 10);
			if (temp == 0 || template == end) {
				error = 2;
				break;
			}

			input[j].columns = temp;

			// special case (it splits up string with multiplicity >1 into separate columns)
			if (temp > 1 && input[j].type == STRING) {
				for (i = 0; i < temp; i++) {
					input[j + i].bytesize   = input[j].bytesize;
					input[j + i].type       = STRING;
					input[j + i].columns    = 1;
					input[j + i].start_byte = offset;
					offset += input[j].bytesize;

					input[j + i].col_name = (char*)calloc(name_length, sizeof(char));
					if (input[j + i].col_name == NULL) {
						memory_error(errno, sizeof(char) * name_length);
						for (--i; i >= 0; i--) free(input[j + i].col_name);
						for (--j; j >= 0; j--) free(input[j].col_name);
						return NULL;
					}

					if (name_count == 0) {
						if (snprintf(input[j + i].col_name, name_length, "c%d_%d", j + 1, i) >= name_length) {
							parse_error("%s, c%d_%d: %s", "Column name too long", j + 1, i,
							            strerror(errno));
							return NULL;
						}
					} else {
						if (snprintf(input[j + i].col_name, name_length, "%s_%d", names[j_no_mult],
						             i) >= name_length) {
							parse_error("%s, c%d_%d: %s", "Column name too long", j + 1, i,
							            strerror(errno));
							return NULL;
						}
					}
				}

				j += (i - 1);
				offset -= input[j].bytesize; // subtract one so the normal addition at the end of
				                             // the
				// while loop doesn't mess it up
			} else {
				offset += ((temp - 1) * input[j].bytesize);
			}

			template = end;
		} else {
			input[j].columns = 1;
		}

		if (input[j].type != IGNORE) {
			if (input[j].col_name == NULL) {
				if (name_count == 0) {
					input[j].col_name = (char*)calloc(name_length, sizeof(char));
					if (input[j].col_name == NULL) {
						memory_error(errno, sizeof(char) * name_length);
						for (--j; j >= 0; j--) free(input[j].col_name);
						return NULL;
					}

					if (snprintf(input[j].col_name, name_length, "c%d", j + 1) >= name_length) {
						parse_error("%s, c%d: %s", "Column name too long", j + 1, strerror(errno));
						return NULL;
					}
				} else {
					input[j].col_name = strdup(names[j_no_mult]);
					if (input[j].col_name == NULL) {
						memory_error(errno, sizeof(char) * name_length);
						for (--j; j >= 0; j--) free(input[j].col_name);
						return NULL;
					}
				}
			}
		}

		offset += input[j].bytesize;
		if (input[j].type != IGNORE) {
			j++;         // increment j for next letter(overwrite IGNOREs)
			j_no_mult++; // increment j not counting multiplicity of strings (for if they provide
		}                // column names

		if (j >= MAX_ATTRIBUTES) {
			parse_error("Too many attributes, %d is max allowed\n", MAX_ATTRIBUTES);
			return NULL;
		}

		if (name_count != 0 && (j_no_mult >= name_count && template[0] != '\0' && template[0] != IGNORE)) {
			parse_error("Too few column names provided, you provided %d\n", name_count);
			for (--j; j >= 0; j--) free(input[j].col_name);
			return NULL;
		}
	} // end parsing while loop

	if (error) {

		fprintf(stderr, "Unexpected or missing character:  ");
		for (i = 0; i < strlen(beginning); i++) {
			if (i == template - beginning) {
				if (error == 1)
					fprintf(stderr, ">>>%c<<<", beginning[i]);
				else
					fprintf(stderr, ">>><<<%c", beginning[i]);

				error = 0;
			} else
				fprintf(stderr, "%c", beginning[i]);
		}
		if (error) fprintf(stderr, ">>><<<");

		fprintf(stderr, "\n");

		//		parse_error("Unexpected or missing character:\n%s\n%*c\n", beginning,
		//template-beginning, '^');
		return NULL;
	}

#ifdef FF_UNPACK_DEBUG
	for (i = 0; i < j; i++) fprintf(stderr, "%c%d\t", input[i].type, input[i].bytesize);
	fprintf(stderr, "\nnum_items: %d\n", j);
#endif

	more = (unpack_digest*)calloc(1, sizeof(unpack_digest));
	if (more == NULL) {
		memory_error(errno, sizeof(unpack_digest));
		return NULL;
	}

	more->attr      = input;
	more->num_items = j;
	more->rows      = -1;

	*record_length = offset;

	return more;
}

/* store number of bytes needed to store data (vs. size of data on disk) */
static void compute_adj_bytes(unpack_digest* input)
{
	int i;
	char a;

	int num_items                 = input->num_items;
	column_attributes* attr_array = input->attr;

	for (i = 0; i < num_items; i++) {
		a = attr_array[i].type;
		if (a == SIGNED_MSB_INT || a == UNSIGNED_MSB_INT || a == SIGNED_LSB_INT || a == UNSIGNED_LSB_INT) {
			if (attr_array[i].bytesize == 3)
				attr_array[i].adj_bytesize = 4;
			else if (attr_array[i].bytesize >= 5)
				attr_array[i].adj_bytesize = 8;
			else
				attr_array[i].adj_bytesize = attr_array[i].bytesize;
		} else if (a == STRING) {
			attr_array[i].adj_bytesize = attr_array[i].bytesize + 1;
		} else {
			attr_array[i].adj_bytesize = attr_array[i].bytesize;
		}
	}
}

/* Pretty self explanatory */
static int calc_rows(char* filename, int hdr_length, unpack_digest* input, int rec_length)
{
	// use filesize,header size, and template to determine number of rows

	struct stat filestats; // struct to get file size with fstat
	int filesize, i;
	int rows = 0, width = 0;

	if (stat(filename, &filestats) < 0) // get filesize
	{
		parse_error("stat failed in calc_rows(): %s", strerror(errno));
		return 0;
	}
	filesize    = filestats.st_size;
	rows        = (filesize - hdr_length) / (rec_length);
	input->rows = rows;

	return 1;
}

/* also pretty self explanatory */
static data* allocate_arrays(unpack_digest* digest)
{
	int i, j = 0, k = 0, mem_num = 0, err_num = 0;
	// char* err_str = "Error in allocate_arrays()";
	int num_items            = digest->num_items;
	int rows                 = digest->rows;
	column_attributes* input = digest->attr;

	data* all_data = (data*)calloc(num_items, sizeof(data));
	if (all_data == NULL) {
		memory_error(errno, sizeof(data) * num_items);
		return NULL;
	}

	for (i = 0; i < num_items; i++) {

		if (input[i].type == STRING) {
			all_data[i].strarray = (char**)calloc(rows, sizeof(char*));
			if (all_data[i].strarray == NULL) {
				err_num = errno;
				mem_num = rows * sizeof(char*);
				break;
			}

			for (k = 0; k < rows; k++) {
				all_data[i].strarray[k] = (char*)calloc(input[i].adj_bytesize, sizeof(char));
				if (all_data[i].strarray[k] == NULL) {
					err_num = errno;
					mem_num = input[i].adj_bytesize * sizeof(char);
					break;
				}
			}
			if (k < rows) break;

			k                 = 0;
			all_data[i].array = NULL;
		} else {
			all_data[i].array =
			    (u8*)calloc(input[i].columns * input[i].adj_bytesize * rows, sizeof(u8));
			if (all_data[i].array == NULL) {
				err_num = errno;
				mem_num = input[i].columns * input[i].adj_bytesize * rows * sizeof(u8);
				break;
			}

			all_data[i].strarray = NULL;
		}
	}

	/* clean up code if necessary */
	if (i < num_items) {
		for (; i >= 0; i--) {

			if (input[i].type == STRING) {
				for (--k; k >= 0; k--) free(all_data[i].strarray[k]);

				if (all_data[i].strarray != NULL) free(all_data[i].strarray);

				k = rows;
			} else {
				if (all_data[i].array != NULL) free(all_data[i].array);
			}
		}

		free(all_data);
		memory_error(err_num, mem_num);
		return NULL;
	}

	return all_data;
}

/* print function for debugging */

#ifdef FF_UNPACK_DEBUG
static int print_data(data* the_data, unpack_digest* input)
{
	int i, j, k;

	double tempdouble;
	float tempfloat;
	i64 tmpi64;
	u64 tmpu64;
	i32 tempint;
	u32 utempint;
	i16 tempshort;
	u16 utempshort;
	i8 signedbyte;

	int bytesize;
	int columns;

	int num_items = input->num_items;
	int rows      = input->rows;

	for (i = 0; i < num_items; i++) printf("%c%d\t", the_data[i].type, the_data[i].numbytes);

	printf("\n");
	for (i = 0; i < num_items; i++) printf("%s\t", the_data[i].input->col_name);

	getchar();
	for (i = 0; i < rows; i++) {
		for (j = 0; j < num_items; j++) {
			bytesize = the_data[j].numbytes;
			columns  = the_data[j].input->columns;

			for (k = 0; k < columns; k++) {
				switch (the_data[j].type) {
				case STRING:                                 // ASCII string space padded
					printf("%s\t", the_data[j].strarray[i]); //&the_data[j].array[(i*columns+k)*bytesize]);
					break;

				case MSB_DOUBLE:
				case LSB_DOUBLE:
					memcpy(&tempdouble, &the_data[j].array[(i * columns + k) * 8], 8);
					printf("%G\t", tempdouble);
					break;

				case MSB_FLOAT:
				case LSB_FLOAT:
					memcpy(&tempfloat, &the_data[j].array[(i * columns + k) * 4], 4);
					printf("%G\t", tempfloat);
					break;

				case SIGNED_MSB_INT:
				case SIGNED_LSB_INT:
					if (bytesize == 8) {
						memcpy(&tmpi64, &the_data[j].array[(i * columns + k) * 8], 8);
						printf("%ld\t", tmpi64);
					} else if (bytesize == 4) {
						memcpy(&tempint, &the_data[j].array[(i * columns + k) * 4], 4);
						printf("%d\t", tempint);
					} else if (bytesize == 2) {
						memcpy(&tempshort, &the_data[j].array[(i * columns + k) * 2], 2);
						printf("%d\t", tempshort);
					} else {
						// memcpy(&signedbyte, &the_data[j].array[i*columns[j]+k], 1);
						signedbyte = (i8)the_data[j].array[i * columns + k];
						printf("%d\t", signedbyte);
					}
					break;

				case UNSIGNED_MSB_INT:
				case UNSIGNED_LSB_INT:
					if (bytesize == 8) {
						memcpy(&tmpu64, &the_data[j].array[(i * columns + k) * 8], 8);
						printf("%lu\t", tmpu64);
					} else if (bytesize == 4) {
						memcpy(&utempint, &the_data[j].array[(i * columns + k) * 4], 4);
						printf("%u\t", utempint);
					} else if (bytesize == 2) {
						memcpy(&utempshort, &the_data[j].array[(i * columns + k) * 2], 2);
						printf("%u\t", utempshort);
					} else {
						printf("%u\t", the_data[j].array[i * columns + k]);
					}
					break;

				default:
					fprintf(stderr, "P:Unknown type: %c\n", the_data[j].type);
					return 0;
					break;
				}
			}
		}
		printf("\n");
		// getchar();
	}
	return 1;
}
#endif

/*
    reads data into buf, performs necessary manipulations (like byteswapping)
    then stores into correct location (loc) in the_data->array
*/
static int read_data(u8* buffer, data* the_data, unpack_digest* input, FILE* file, int row)
{
	i64 tmp_i64 = 0;
	u64 tmp_u64 = 0;

	u8* mybyte = NULL;
	u8* loc    = NULL;
	u8* buf    = NULL;

	int i, j, k;

	char letter;
	int numbytes;
	int al_bytes;
	int columns;

	for (j = 0; j < input->num_items; j++) {
		letter   = input->attr[j].type;
		numbytes = input->attr[j].bytesize;
		al_bytes = input->attr[j].adj_bytesize;
		columns  = input->attr[j].columns;

		buf = &buffer[input->attr[j].start_byte]; // skip to offset

		for (k = 0; k < columns; k++) {

			if (letter != STRING) loc = &the_data[j].array[(row * columns + k) * al_bytes];

			switch (letter) {
			case STRING: // ASCII string space padded
				memcpy(the_data[j].strarray[row], buf, numbytes);
				the_data[j].strarray[row][numbytes] = '\0';

				break;

			case LSB_DOUBLE:
			case LSB_FLOAT:
				LSB(buf, 1, numbytes);
				memcpy(loc, buf, al_bytes);

				break;

			case MSB_DOUBLE:
			case MSB_FLOAT:
				MSB(buf, 1, numbytes);
				memcpy(loc, buf, al_bytes);

				break;

			case SIGNED_MSB_INT:
			case SIGNED_LSB_INT: tmp_i64 = 0; mybyte = (u8*)&tmp_i64;

#ifdef WORDS_BIGENDIAN
				if (letter == SIGNED_LSB_INT) {
					for (i = 0; i < numbytes; i++) {
						mybyte[7 - (8 - numbytes) - i] = buf[i];
					}
					tmp_i64 = tmp_i64 >> ((8 - numbytes) * 8);
				} else {
					for (i = 0; i < numbytes; i++) {
						mybyte[i] = buf[i];
					}
					tmp_i64 = tmp_i64 >> ((8 - numbytes) * 8);
				}

				memcpy(loc, &mybyte[8 - al_bytes], al_bytes);

#else
				if (letter == SIGNED_MSB_INT) {
					for (i = 0; i < numbytes; i++) {
						mybyte[7 - i] = buf[i];
					}
					tmp_i64 = tmp_i64 >> ((8 - numbytes) * 8);
				} else {
					for (i = 0; i < numbytes; i++) {
						mybyte[7 - (numbytes - 1) + i] = buf[i];
					}
					tmp_i64 = tmp_i64 >> ((8 - numbytes) * 8);
				}
				memcpy(loc, mybyte, al_bytes);

#endif
				break;

			case UNSIGNED_MSB_INT:
			case UNSIGNED_LSB_INT: tmp_u64 = 0; mybyte = (u8*)&tmp_u64;

#ifdef WORDS_BIGENDIAN
				if (letter == UNSIGNED_LSB_INT) {
					for (i = 0; i < numbytes; i++) {
						mybyte[7 - i] = buf[i];
					}
				} else {
					for (i = 0; i < numbytes; i++) {
						mybyte[i + (8 - numbytes)] = buf[i];
					}
				}

				memcpy(loc, &mybyte[8 - al_bytes], al_bytes);

#else
				if (letter == UNSIGNED_MSB_INT) {
					for (i = 0; i < numbytes; i++) {
						mybyte[numbytes - 1 - i] = buf[i];
					}
				} else {
					for (i = 0; i < numbytes; i++) {
						mybyte[i] = buf[i];
					}
				}

				memcpy(loc, mybyte, al_bytes);

#endif

				break;

			default:
				parse_error("read_data(): Unknown type: %c\n", letter);
				return 0;
				break;

			} // end switch type

			buf = &buf[numbytes]; // skip forward for multiplicity if necessary

		} // end for columns
	}     // end for num_items

	return 1;
}



// upgrade_types() is now a no-op function since Davinci now supports all types.
// We could change this to try to fit the data in the smallest possible
// type to try to save memory
//
// ie if they specified a i8 but it fits in an i4 , or even more specific
// they specified an i8 but it fits in a u4 (but not an i4).  This downsizability
// would apply to all integer types if we really wanted
//
// but I don't think it's worth it.

#if 0
static int upgrade_types(data* the_data, unpack_digest* input)
{
	int i, j;

	char type;
	int bytesize;
	int columns;

	int fields = input->num_items;
	int rows   = input->rows;

#ifdef FF_UNPACK_DEBUG
	fprintf(stdout, "\n");
#endif

	for (i = 0; i < fields; i++) {
		type     = the_data[i].input->type;
		bytesize = the_data[i].input->bytesize;
		columns  = the_data[i].input->columns;

		switch (type) {
		case SIGNED_MSB_INT:
		case SIGNED_LSB_INT:

			break;

		case UNSIGNED_MSB_INT:
		case UNSIGNED_LSB_INT:
			break;

		default: break;
		}
	}

	return 1;
}
#endif


// ********************************************* Begin Pack Implementation Functions
// **************************************************** //

// JNN:
// ff_pack(), inverse of ff_unpack().
// @param vfuncptr func     // function
// @param Var* arg          // argument list
// @return NULL
Var* ff_pack(vfuncptr func, Var* arg)
{
	Var* toPack    = NULL; // Davinci structure to be packed
	char* template = NULL; // optional template string indicating how the data should be packed
	char* filename = NULL; // name of file to pack struct into

	//NOTE(rswinkle): What is the point of a skip parameter for pack()?  Seriously?
	int skip       = 0;    // optional, file offset before begin writing. default: none

	int count = -1;        // optional, number of rows (i.e. Davinci records) to pack. default: negative = ALL
	Var* column_names = NULL;    // optional, order in which columns (i.e. Davinci fields) should be packed
	int force = 0; // optional, force file overwrite. default: NO
	int num_items, rows;

	Alist alist[8];
	alist[0]      = make_alist(ARG_STRUCT, ID_STRUCT, NULL, &toPack);
	alist[1]      = make_alist(ARG_FILENAME, ID_STRING, NULL, &filename);
	alist[2]      = make_alist(ARG_COUNT, DV_INT32, NULL, &count);
	alist[3]      = make_alist(ARG_COL_NAMES, ID_UNK, NULL, &column_names);
	alist[4]      = make_alist(ARG_FORCE, DV_INT32, NULL, &force);
	alist[5]      = make_alist(ARG_TEMPLATE, ID_STRING, NULL, &template);
	alist[6]      = make_alist(ARG_SKIP, DV_INT32, NULL, &skip);
	alist[7].name = NULL;

	// TODO add truncate parameter to truncate at the end of current write
	// TODO add reading of data record before pack_row to keep skipped areas intact

	if (parse_args(func, arg, alist) == 0) {
		return NULL;
	}

	if (toPack == NULL) {
		parse_error("%s not specified: %s()", ARG_STRUCT, func->name);
		return NULL;
	}

	/*
	if (template == NULL) {
		parse_error("%s not specified: %s()", ARG_TEMPLATE, func->name);
		return NULL;
	}
	*/

	int free_template = 1;
	if (template) {
		if (strlen(template) == 0) {
			parse_error("Error: Template cannot be the empty string!");
			return NULL;
		}
		free_template = 0;
	}

	if (filename == NULL) {
		parse_error("%s not specified: %s()", ARG_FILENAME, func->name);
		return NULL;
	}
	if (strlen(filename) == 0) { // JIC, fopen() should perform the same operation in pack()
		parse_error("Error: Filename cannot be the empty string!");
		return NULL;
	}

	if (skip < 0) {
		parse_error("%s < 0: %s()", ARG_SKIP, func->name);
		return NULL;
	}

	if (count == 0) {
		parse_error("Count is 0, nothing to do."); // records, y-dimension
		return NULL;
	}

	if (!force && file_exists(filename)) {
		parse_error("File %s already exists.", filename);
		return NULL;
	}

	// if force == 1 remove old file
	// TODO(rswinkle): unlink()?  and force doesn't make sense with fopen("wb+") below
	if (force) {
		unlink(filename);
	}

	// parse Davinci struct into data*, also compute num_items (x-axis) and rows (y-axis)
	data* reg_data = parse_struct(toPack, column_names, &num_items, &rows);
	if (reg_data == NULL) {
		parse_error("Problem in function: %s()", func->name);
		return NULL;
	}

	// count negative, pack computed rows.
	// count zero, return error (see above).
	// count positive, pack that amount (write null chars for rows which do not exist).
	rows = count > 0 ? count : rows;
	if (pack(toPack, reg_data, &template, filename, num_items, rows, skip)) {
		parse_error("Packed %d records to to %s.", rows, filename);
	}

	cleanup_data(reg_data, num_items); // clean memory for reg_data

	// NOTE(rswinkle):
	// return the template.  This is used/needed for when the user doesn't provide one.
	// (which why should they, we already know the data types of the structure), so
	// they can get the template we generated to use to call unpack() on the file later
	Var* s       = newVar();
	V_TYPE(s)    = ID_STRING;
	V_STRING(s)  = strdup(template);

	if (free_template)
		free(template);

	return s;
}

// used in parse_struct to clean up reg_data in case of error
static void cleanup_data(data* reg_data, int i)
{
	while (--i >= 0) {
		if (reg_data[i].input) {
			free(reg_data[i].input);
		}
	}
	free(reg_data);
}

// JNN:
// parse_struct(), function to parse Davinci structure into data* based on col_names.
// called by ff_pack()
// Note: part of code taken from parse_template()
// @param Var* toPack           // the Davinci structure to get information from
// @param Var* column_names     // the user specified order in which Davinci fields should be packed
// @param int* numData          // out address to store size of data* array (x-dimension)
// @param int* greatestNumRows  // out address to store the greatest row size of any data column
// (y-dimension)
// @return data*                // NULL if failed to parse struct
static data* parse_struct(Var* toPack, Var* column_names, int* numData, int* greatestNumRows)
{
	int name_count = 0;
	int i;
	char** names = NULL;
	data* reg_data;
	int rows;
	int dataIndex; // index of davinci struct in which data is stored
	Var* element;

	if (column_names != NULL) {
		if (V_TYPE(column_names) == ID_TEXT) {
			name_count = V_TEXT(column_names).Row;
			names      = V_TEXT(column_names).text;
		} else if (V_TYPE(column_names) == ID_STRING) {
			name_count = 1;
			names      = (char**)calloc(1, sizeof(char*));
			if (names == NULL) {
				memory_error(errno, sizeof(char*));
				return NULL;
			}
			names[0] = V_STRING(column_names);
		} else {
			parse_error("Unrecognized type passed in for argument col_names");
			return NULL;
		}
	}

	// use toPack to find num_items
	int num_items = get_struct_count(toPack); // number of elements (i.e. Davinci fields)
	if (num_items <= 0) {                     // JIC struct is empty
		parse_error("Unable to pack %d elements", num_items);
		return NULL;
	}

	// if user gave some column_names
	if (name_count > 0 && name_count > num_items) {
		parse_error("Column name count (%d) is greater than field count (%d) in struct.",
		            name_count, num_items);
		return NULL;
	}
	*numData = name_count > 0 ? name_count : num_items; // number of columns

	// use numData to allocate memory for data
	reg_data = (data*)calloc(*numData, sizeof(data));
	if (reg_data == NULL) {
		memory_error(errno, sizeof(data) * (*numData));
		return NULL;
	}

	// use toPack to find rows and fill in data
	rows             = 0;
	*greatestNumRows = 0;
	for (i = 0; i < *numData; i++) {
		if (name_count > 0) {
			if ((dataIndex = find_struct(toPack, names[i], NULL)) < 0) {
				parse_error("Unable to find column \"%s\" in struct.", names[i]);
				cleanup_data(reg_data, i); // clean memory for reg_data
				return NULL;
			}
		} else {
			dataIndex = i;
		}

		if (get_struct_element(toPack, dataIndex, NULL, &element) == 0) {
			parse_error("Unable to get element data at index %d from struct.\n", dataIndex);
			cleanup_data(reg_data, i); // clean memory for reg_data
			return NULL;
		}

		switch (V_TYPE(element)) {
		case ID_TEXT:
			rows = V_TEXT(element).Row;
			// reg_data[i].input = NULL;
			reg_data[i].input          = (column_attributes*)calloc(1, sizeof(column_attributes));
			reg_data[i].input->columns = 1; // x value = columns
			reg_data[i].array          = NULL;
			reg_data[i].strarray       = V_TEXT(element).text;
			reg_data[i].type           = ID_STRING;
			break;

		case ID_VAL:
			if (V_ORG(element) != BSQ) {
				fprintf(stderr, "Value not BSQ format.\n");
				cleanup_data(reg_data, i); // clean memory for reg_data
				return NULL;
			}
			if (V_SIZE(element)[2] != 1) { // z value
				fprintf(stderr, "Z-dimension (%d) > 1 is unsupported.\n", V_SIZE(element)[2]);
				cleanup_data(reg_data, i); // clean memory for reg_data
				return NULL;
			}
			rows = V_SIZE(element)[1]; // y value = rows
			// TODO -- how is .input used?
			reg_data[i].input          = (column_attributes*)calloc(1, sizeof(column_attributes));
			reg_data[i].input->columns = V_SIZE(element)[0]; // x value = columns
			reg_data[i].array          = V_DATA(element);
			reg_data[i].strarray       = NULL;
			reg_data[i].type = V_FORMAT(element); // DV_UINT*, DV_INT*, DV_FLOAT, DV_DOUBLE
			break;

		default:
			parse_error("Unknown element type (%d) at struct index %d.\n", V_TYPE(element), dataIndex);
			cleanup_data(reg_data, i); // clean memory for reg_data
			return NULL;
		}

		// some way to store size of strarray or array (i.e. y dimension of the element) for later
		// use
		reg_data[i].numbytes = rows;
		*greatestNumRows     = rows > *greatestNumRows ? rows : *greatestNumRows;
	}

	return reg_data;
}

// pack() implementation function, inverse of unpack()
// called by ff_pack()
// @param data* thedata;            // the actual information to be packed
// @param char* template;           // string indicating how the data should be stored
// @param char* filename;           // the name of file to pack information in
// @param int numData;              // the amount of data in thedata array
// @param int rows;                 // the number of rows of data to be packed
// @param int offset;               // number of bytes to skip in file before writing
// @return int indicating success:  // 1 = success, 0 = fail
//
//
// NOTE(rswinkle): I really hate those java style comments and parameter comments in general.  Honestly
// how helpful are those?  Pick good parameter names and write clear code.
static int pack(Var* struct_to_pack, data* thedata, char** template_in, char* filename, int numData, int rows, int offset)
{
	int i, rec_length;
	column_attributes* initial = NULL; // used to parse template
	unpack_digest* digest      = NULL;
	FILE* file                 = NULL;
	u8* buffer               = NULL;

	char* template = NULL;
	char template_buf[1024] = { 0 };

	// check thedata argument, can't pack what doesn't exist
	if (thedata == NULL) {
		parse_error("Error, no input data was provided.");
		return 0;
	}

	// create an initial column_attributes to work with
	initial = (column_attributes*)calloc(MAX_ATTRIBUTES, sizeof(column_attributes));
	if (initial == NULL) {
		memory_error(errno, sizeof(column_attributes) * MAX_ATTRIBUTES);
		return 0;
	}

	// If no template provided, use the internal datatypes and native byte ordering duh
	// This does somewhat waste effort going through parse_template and validate template later
	// but data structures are filled out in the process and it'd be a pain and more code to
	// create a separate path
	if (!*template_in) {
		char* p = template_buf;
		int cols = 1;
		Var* element = NULL;
		for (i=0; i<numData; ++i) {
			cols = thedata[i].input->columns;

#ifdef WORDS_BIGENDIAN
			switch (thedata[i].type) {
			case DV_UINT8:   p[0] = UNSIGNED_MSB_INT; ++p; break;
			case DV_UINT16:  p[0] = UNSIGNED_MSB_INT, p[1] = '2'; p+=2; break;
			case DV_UINT32:  p[0] = UNSIGNED_MSB_INT, p[1] = '4'; p+=2; break;
			case DV_UINT64:  p[0] = UNSIGNED_MSB_INT, p[1] = '8'; p+=2; break;

			case DV_INT8:    p[0] = SIGNED_MSB_INT; ++p; break;
			case DV_INT16:   p[0] = SIGNED_MSB_INT, p[1] = '2'; p+=2; break;
			case DV_INT32:   p[0] = SIGNED_MSB_INT, p[1] = '4'; p+=2; break;
			case DV_INT64:   p[0] = SIGNED_MSB_INT, p[1] = '8'; p+=2; break;

			case DV_FLOAT:   p[0] = MSB_REAL, p[1] = '4'; p+=2; break;
			case DV_DOUBLE:  p[0] = MSB_REAL, p[1] = '8'; p+=2; break;

			case ID_STRING:  p[0] = STRING; ++p; break;

			}
#else
			switch (thedata[i].type) {
			case DV_UINT8:   p[0] = UNSIGNED_LSB_INT; ++p; break;
			case DV_UINT16:  p[0] = UNSIGNED_LSB_INT, p[1] = '2'; p+=2; break;
			case DV_UINT32:  p[0] = UNSIGNED_LSB_INT, p[1] = '4'; p+=2; break;
			case DV_UINT64:  p[0] = UNSIGNED_LSB_INT, p[1] = '8'; p+=2; break;

			case DV_INT8:    p[0] = SIGNED_LSB_INT; ++p; break;
			case DV_INT16:   p[0] = SIGNED_LSB_INT, p[1] = '2'; p+=2; break;
			case DV_INT32:   p[0] = SIGNED_LSB_INT, p[1] = '4'; p+=2; break;
			case DV_INT64:   p[0] = SIGNED_LSB_INT, p[1] = '8'; p+=2; break;

			case DV_FLOAT:   p[0] = LSB_REAL, p[1] = '4'; p+=2; break;
			case DV_DOUBLE:  p[0] = LSB_REAL, p[1] = '8'; p+=2; break;

			case ID_STRING:  p[0] = STRING; ++p; break;

			}
#endif
			//string is special because it has to have a byte size >= to the largest
			//string in the entire column
			if (thedata[i].type == ID_STRING) {
				get_struct_element(struct_to_pack, i, NULL, &element);
				//already know it has to be BSQ
				int max = 0, len;
				char** txt_array = V_TEXT(element).text;
				for (int j=0; j<V_TEXT(element).Row; ++j) {
					len = strlen(txt_array[j]);
					max = (len > max) ? len : max;
				}
				
				int tmp = sprintf(p, "%d", max);
				p += tmp;
			}

			if (cols > 1) {
				p[0] = '*';
				++p;
				int tmp = sprintf(p, "%d", cols);
				p += tmp;
			}
		}

		*template_in = strdup(template_buf);
	}

	template = *template_in;

	// create an unpack_digest with computed: digest->attr and ->num_items. ->rows set to -1, also
	// rec_length
	digest = parse_template(template, initial, NULL, &rec_length);
	if (digest == NULL) {
		free(initial);
		return 0;
	}

	// if (digest->num_items > numData) // user specified more columns to pack than he/she had given
	// else if (digest->num_items < numData) // user gave more data than what was indicated to pack
	if (digest->num_items != numData) {
		parse_error("Error: #columns in struct (%d) != #columns named for packing (%d)", numData,
		            digest->num_items);
		clean_up(0, digest, NULL, NULL, NULL);
		return 0;
	}

	digest->rows = rows;

	// validate the input template (checks allowable byte sizes)
	if (!validate_template(digest)) {
		clean_up(0, digest, NULL, NULL, NULL);
		return 0;
	}

	//*// reallocate the initial column_attributes to match the amount specified in digest?
	digest->attr = realloc(initial, sizeof(column_attributes) * digest->num_items);
	if (digest->attr == NULL) {
		memory_error(errno, sizeof(column_attributes) * digest->num_items);
		free(initial);
		clean_up(0, digest, NULL, NULL, NULL);
		return 0;
	}
	//*/

	// TODO(rswinkle): think about this with force.  update documentation
	file = fopen(filename, "rb+"); // open file if exists already
	if (file == NULL) {
		file = fopen(filename, "wb+"); // creates a new file to write
		if (file == NULL) {
			parse_error("%s: %s", "Error opening file", strerror(errno));
			clean_up(0, digest, NULL, NULL, NULL);
			return 0;
		}
	}

	if (fseek(file, offset, SEEK_SET)) { // skip to start
		parse_error("file %s seek error: %s", filename, strerror(errno));
		clean_up(0, digest, NULL, NULL, file);
		return 0;
	}

	buffer = (u8*)calloc(rec_length + 2, sizeof(u8)); // output buffer +2 (for '\0')
	if (buffer == NULL) {
		memory_error(errno, rec_length * sizeof(u8));
		clean_up(0, digest, NULL, NULL, file);
		return 0;
	}

	// rows calculated before entering pack()
	for (i = 0; i < rows; i++) {
		// clear the output record buffer
		memset(buffer, 0, rec_length);

		// write data into buffer
		if (!pack_row(thedata, digest, i, buffer)) {
			parse_error("Error in writing data to buffer");
			clean_up(2, digest, buffer, thedata, file);
			return 0;
		}

		// write data to file
		if (!fwrite(buffer, rec_length, 1, file)) {
			parse_error("Error writing data: %s", strerror(errno));
			clean_up(2, digest, buffer, thedata, file);
			return 0;
		}
	}

	// cleanup
	free(buffer);
	free(digest);
	fclose(file);

	return 1; // return success
}

//TODO(rswinkle) why aren't we using our existing swapping functions?
// see endian_norm.c/h dvio_tdb.c dvio_specpr.c/h
static void copy(const void* from, void* to, int len, int swap)
{
	int i;

	for (i = 0; i < len; i++) {
		((char*)to)[i] = swap ? ((const char*)from)[len - i - 1] : ((const char*)from)[i];
	}
}

static int convert_to_ext_fmt(void* from, int ffmt, char* to, int tfmt, int tolen)
{
	u64 tmpu64;
	i64 tmpi64;

	i32 tmpi32;
	u32 tmpu32;
	i16 tmpi16;
	u16 tmpu16;
	i8 tmpi8;
	u8 tmpu8;
	float f;
	double d;

	int big   = 0;
	
	// NOTE(rswinkle): Not a huge fan of alloca()
	char* str = alloca(tolen > 1024 ? tolen + 1 : 1024);

	switch (ffmt) {
	case DV_UINT8:  tmpi64 = tmpu64 = *(u8*)from; break;
	case DV_UINT16: tmpi64 = tmpu64 = *(u16*)from; break;
	case DV_UINT32: tmpi64 = tmpu64 = *(u32*)from; break;
	case DV_UINT64: tmpi64 = tmpu64 = *(u64*)from; break;

	case DV_INT8:   tmpu64 = tmpi64 = *(i8*)from; break;
	case DV_INT16:  tmpu64 = tmpi64 = *(i16*)from; break;
	case DV_INT32:  tmpu64 = tmpi64 = *(i32*)from; break;
	case DV_INT64:  tmpu64 = tmpi64 = *(i64*)from; break;
	
	case DV_FLOAT:
		f = *(float*)from;
		tmpu64 = (u64)f;
		tmpi64 = (i64)f;
		break;
	case DV_DOUBLE:
		d = *(double*)from;
		tmpu64 = (u64)d;
		tmpi64 = (i64)d;
		break;

	case ID_STRING:
		d  = atof(from);
		tmpu64 = strtoull(from, NULL, 10);
		tmpi64 = atoll(from);
		break;
	default: return 0;
	}

	// add conversions for other numeric types
	tmpu32 = tmpu64;
	tmpi32 = tmpi64;
	tmpu16 = tmpu32;
	tmpi16 = tmpi32;
	tmpu8 = tmpu32;
	tmpi8 = tmpi32;

	if (tfmt == STRING) {
		switch (ffmt) {
		case DV_UINT8:
		case DV_UINT16:
		case DV_UINT32:
		case DV_UINT64: sprintf(str, "%lu", tmpu64); break;

		case DV_INT8:
		case DV_INT16:
		case DV_INT32:
		case DV_INT64: sprintf(str, "%d", tmpi32); break;

		case DV_FLOAT:
		case DV_DOUBLE: sprintf(str, "%g", d); break;
		case ID_STRING: sprintf(str, "%s", (char*)from); break;
		default: return 0;
		}
	}

// do we need to swap data
#ifdef WORDS_BIGENDIAN
	big = 1;
#else
	big = 0;
#endif

	// write data out, u8-swapping as needed
	switch (tfmt) {
	case LSB_DOUBLE: copy(&d, to, sizeof(d), big ^ 0); break;
	case MSB_DOUBLE: copy(&d, to, sizeof(d), big ^ 1); break;
	case LSB_FLOAT: copy(&f, to, sizeof(f), big ^ 0); break;
	case MSB_FLOAT: copy(&f, to, sizeof(f), big ^ 1); break;
	case SIGNED_LSB_INT:
		switch (tolen) {
		case 1: copy(&tmpi8, to, sizeof(tmpi8), big ^ 0); break;
		case 2: copy(&tmpi16, to, sizeof(tmpi16), big ^ 0); break;
		case 4: copy(&tmpi32, to, sizeof(tmpi32), big ^ 0); break;
		case 8: copy(&tmpi64, to, sizeof(tmpi64), big ^ 0); break;
		default: return 0;
		}
		break;
	case SIGNED_MSB_INT:
		switch (tolen) {
		case 1: copy(&tmpi8, to, sizeof(tmpi8), big ^ 1); break;
		case 2: copy(&tmpi16, to, sizeof(tmpi16), big ^ 1); break;
		case 4: copy(&tmpi32, to, sizeof(tmpi32), big ^ 1); break;
		case 8: copy(&tmpi64, to, sizeof(tmpi64), big ^ 1); break;
		default: return 0;
		}
		break;
	case UNSIGNED_LSB_INT:
		switch (tolen) {
		case 1: copy(&tmpu8, to, sizeof(tmpu8), big ^ 0); break;
		case 2: copy(&tmpu16, to, sizeof(tmpu16), big ^ 0); break;
		case 4: copy(&tmpu32, to, sizeof(tmpu32), big ^ 0); break;
		case 8: copy(&tmpu64, to, sizeof(tmpu64), big ^ 0); break;
		default: return 0;
		}
		break;
	case UNSIGNED_MSB_INT:
		switch (tolen) {
		case 1: copy(&tmpu8, to, sizeof(tmpu8), big ^ 1); break;
		case 2: copy(&tmpu16, to, sizeof(tmpu16), big ^ 1); break;
		case 4: copy(&tmpu32, to, sizeof(tmpu32), big ^ 1); break;
		case 8: copy(&tmpu64, to, sizeof(tmpu64), big ^ 1); break;
		default: return 0;
		}
		break;
	case STRING: strncpy(to, str, tolen); break;
	default: return 0;
	}

	return 1;
}

// write_data(), inverse of read_data(). writes the data into a buffer
// called by pack()
// @param u8* buffer              // buffer to write data into
// @param data* the_data            // data to write into buffer
// @param unpack_digest* input      // input digest indicating how data should be written
// @param int row                   // the current row in data to write
// @return int indicating success:  // 1 = success, 0 = fail
static int pack_row(data* the_data, unpack_digest* digest, int row, u8* buffer)
{
	int i, j, k, numbytes, al_bytes, columns, start_byte;
	char letter;
	void* src_buf;
	int src_type, src_columns;

	// start for loop through unpack_digest's input
	for (j = 0; j < digest->num_items; j++) {
		letter     = digest->attr[j].type;         // char
		numbytes   = digest->attr[j].bytesize;     // int <- short
		al_bytes   = digest->attr[j].adj_bytesize; // int <- short
		columns    = digest->attr[j].columns;      // int <- short
		start_byte = digest->attr[j].start_byte;   // skip to offset

		src_type    = the_data[j].type;
		src_columns = the_data[j].input->columns;

		// loop through columns in column attributes
		for (k = 0; k < columns; k++) {
			if (k >= src_columns) // src data has fewer columns than specified
				break;

			// TODO - check for buffer overuns -- different for strings vs vals
			if (src_type == ID_STRING) {
				src_buf = the_data[j].strarray[row];
			} else {
				src_buf = the_data[j].array + row * src_columns * NBYTES(src_type) + k * NBYTES(src_type);
			}
			if (!convert_to_ext_fmt(src_buf, src_type, &buffer[start_byte + k * numbytes], letter, numbytes)) {
				fprintf(stderr, "Unable to convert column %d (index %d)\n", j, k);
				return 0;
			}
		}
	} // end for num_items

	return 1;
}
