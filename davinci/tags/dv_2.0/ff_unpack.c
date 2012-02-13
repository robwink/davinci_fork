#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "parser.h"
#include "endian_norm.h"
#include <errno.h>

//#define FF_UNPACK_DEBUG 1

#define ARG_TEMPLATE	"template"
#define ARG_FILENAME	"filename"
#define ARG_SKIP		"skip"
#define ARG_COUNT		"count"
#define ARG_COL_NAMES	"col_names"

#define STRING				'a'
#define SIGNED_MSB_INT		'I'
#define UNSIGNED_MSB_INT	'U'
#define SIGNED_LSB_INT		'i'
#define UNSIGNED_LSB_INT	'u'

#define LSB_REAL			'r'
#define MSB_REAL			'R'

#define LSB_FLOAT			'f'
#define MSB_FLOAT			'F'
#define LSB_DOUBLE			'd'
#define MSB_DOUBLE			'D'

#define IGNORE				'x'


#define MAX_ATTRIBUTES 500



typedef unsigned char byte;

//filled in parse_template()
typedef struct
{
	char type;					//type constant from above defines
	short bytesize;				//bytesize in file (from template)
	short adj_bytesize;			//bytesize allocated (e.g. +1 for strings for null character
	short columns;				//multiplicity
	unsigned short start_byte;	//record offset
	char* col_name;				//generated name used for creating davinci structs
} column_attributes;


//this is passed around to functions for convenience
typedef struct
{
	column_attributes* attr;			//array of above
	int num_items;						//number of items (i.e. size of attr array)
	int rows;							//calculated number of rows in file based on record size
} unpack_digest;


//array of this is returned from unpack function to ff_unpack
typedef struct
{
	column_attributes *input;		//address of appropriate element in attr array above
	byte *array;			//array to actually store the column data, NULL if type is string
	char** strarray;		//array to store column data if type is string, NULL otherwise
	short numbytes;			//set equal to adj_bytesize
	char type;				//set equal to final type (after conversions in upgrade_types() )
} data;


static int convert_types(data* thedata, int num_items, int rows);

static data* unpack(char*, char *, Var*, int, int*, int*);
static unpack_digest* parse_template(char* template, column_attributes* input, Var* column_names, int*);
static int validate_template(unpack_digest* input);
static int read_data(byte*, data*, unpack_digest*, FILE*, int); 
static int calc_rows(char*, int, unpack_digest*, int);
static data* allocate_arrays(unpack_digest*);
static void compute_adj_bytes(unpack_digest*);


static int print_data(data* the_data, unpack_digest* input);
static int upgrade_types(data*, unpack_digest*);

static int resize_array(data* the_data, int rows, int columns, int type);
static int test_signed_char(data* the_data, int rows, int columns);
static int test_signed_short(data* the_data, int rows, int columns);
static int test_int_max(data* the_data, int rows, int columns);

static void clean_up(int, unpack_digest*, byte*, data*, FILE*);

static void memory_error(int error_num, int mem_size)
{
	char err_str[100];
	
	snprintf(err_str, sizeof(err_str)-1, "Error allocating memory, %d: ", mem_size);
	parse_error("%s: %s", err_str, strerror(error_num));

	return;
}

/*	premade calls for testing:

//	unpack("U4U2U2*38U2U2R4I2*9R4R4R4x6a4", "atm05555.dat", 390)
//	unpack("U4U2U2*38U2U2x4I2*9x18a4", "atm05555.dat", 390)	//without floats
//	unpack("U4U2U2U4UU2UaaaaaaaUUx8U2*4U", "obs05555.dat",420)
//	unpack("Iu2i3u4r4r8x4I2I3U4R4R8a5", "mytestfile.dat", 0)
//	unpack("Iu2i3u4r4r8x4U2U3U4R4R8a5", "secondtest.dat", 0)
//	unpack("Iu2i3u4r4r8x4U2U3U4R4R8a6*3", "strmulttest.dat", 0)

*/

Var *
ff_unpack(vfuncptr func, Var* arg)
{
	char* template = NULL;
	char* filename = NULL;
	int hdr_length = 0;
	Var* result = NULL;
	int rows = -1;
	Var* column_names = NULL;


	int i = 0;

	Alist alist[6];
	alist[0] = make_alist(	ARG_TEMPLATE,	ID_STRING,	NULL,	&template);
	alist[1] = make_alist(	ARG_FILENAME,	ID_STRING,	NULL,	&filename);
	alist[2] = make_alist(	ARG_SKIP,		INT,		NULL,	&hdr_length);
	alist[3] = make_alist(	ARG_COUNT,		INT,		NULL,	&rows);
	alist[4] = make_alist(	ARG_COL_NAMES,	ID_UNK,		NULL,	&column_names);
	alist[5].name = NULL;

	if( parse_args(func, arg, alist) == 0) return NULL;

	if( template == NULL ) {
    	parse_error("%s not specified: %s()", ARG_TEMPLATE, func->name);
		return NULL;
	}

	if( filename == NULL ) {
    	parse_error("%s not specified: %s()", ARG_FILENAME, func->name);
		return NULL;
	}

	if( hdr_length < 0 ) {
		parse_error("%s < 0: %s()", ARG_SKIP, func->name);
		return NULL;
	}

	int num_items = 0;
	data* reg_data = unpack(template, filename, column_names, hdr_length, &num_items, &rows);


	if( reg_data == NULL || rows <= 0 || num_items <= 0)
		return NULL;


	/* Converting types to appropriate davinci types (constants ie SHORT, BYTE etc.) */
	if( !convert_types(reg_data, num_items, rows) ) {
		free(reg_data);
		return NULL;
	}


	// Creating davinci struct and looping through data creating appropriate substructures
	result = new_struct(0);

	int columns = 0;
	for(i=0; i<num_items ; i++) {
		columns = reg_data[i].input->columns;

		if( reg_data[i].type != STRING )
			add_struct(result, reg_data[i].input->col_name,
				 newVal(BSQ, columns, rows, 1, reg_data[i].type, reg_data[i].array));
		else 
			add_struct(result, reg_data[i].input->col_name,
				 newText(rows, reg_data[i].strarray));
	}


	//freeing old data/containers.  Actual data is still allocated and used by new davinci structure
	for(i=0;i<num_items; i++)
		free( reg_data[i].input->col_name );

	free(reg_data);

	return result;
}


/* Convert my type constants to appropriate davinci type constants 
 * e.g. SIGNED_LSB_INT && numbytes == 2-> SHORT
 */
static int convert_types(data* thedata, int num_items, int rows)
{
	int i = 0;

	for(i=0;i<num_items;i++) {
		switch(thedata[i].type)
		{
			case SIGNED_LSB_INT:
			case SIGNED_MSB_INT:
				if( thedata[i].numbytes == 2 )
					thedata[i].type = SHORT;
				else
					thedata[i].type = INT;
				break;

			case UNSIGNED_LSB_INT:
			case UNSIGNED_MSB_INT:
				thedata[i].type = BYTE;
				break;

			case LSB_FLOAT:
			case MSB_FLOAT:
				thedata[i].type = FLOAT;
				break;

			case LSB_DOUBLE:
			case MSB_DOUBLE:
				thedata[i].type = DOUBLE;
				break;

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
static data* unpack(char* template, char* filename, Var* column_names, int hdr_length, int* numitems, int *ret_rows)
{

	int i,k,x;
	int rec_length, max_buf;

	column_attributes*	initial		= NULL;
	unpack_digest* 		input		= NULL;
	FILE*				file		= NULL;
	byte*				buffer		= NULL;
	data*				thedata		= NULL;
	void*				temp_ptr	= NULL;

	if( strlen(template) == 0 ) {
		parse_error("Error: Template cannot be the empty string!");
		return NULL;
	}


	initial = (column_attributes*)calloc(MAX_ATTRIBUTES, sizeof(column_attributes));
	if( initial == NULL ) {
		memory_error(errno, sizeof(column_attributes)*MAX_ATTRIBUTES );
		return NULL;
	}

	input = parse_template(template, initial, column_names, &rec_length);
	if( input == NULL ) {
		free(initial);
		return NULL;
	}

    if( !validate_template(input) ) {
        clean_up(0, input, NULL, NULL, NULL);
        return NULL;
	}



	initial = realloc(initial, sizeof(column_attributes)*input->num_items);
	if( initial == NULL ) {
		memory_error(errno, sizeof(column_attributes)*input->num_items );
		clean_up(0, input, NULL, NULL, NULL);
		return NULL;
	} else {
		input->attr = initial;
	}

	file = fopen(filename,"rb");
	if( file == NULL ) {
		parse_error("%s: %s", "Error opening file", strerror(errno));
		clean_up(0, input, NULL, NULL, NULL);
		return NULL;
	}

	if( fseek(file,hdr_length,SEEK_SET) ) {
		parse_error("fseek error: %s", strerror(errno));
		clean_up(0, input, NULL, NULL, file);
		return NULL;
	}
#ifdef FF_UNPACK_DEBUG
	fprintf(stderr, "record length: %d\n\n", rec_length);
#endif

	//calculate number of rows
	if( !calc_rows(filename, hdr_length, input, rec_length) ) {
		clean_up(0, input, NULL, NULL, file);
		return NULL;
	}

	if (*ret_rows >= 0){ /* user passed in the number of rows to convert */
		if (*ret_rows > input->rows){
			parse_error("error processing file %s: requested rows (%d) > computed file rows (%d)\n", 
				filename, *ret_rows, input->rows);
			clean_up(1, input, NULL, NULL, file);
		}
		else {
			/* set the number of rows to process to the value passed in */
			input->rows = *ret_rows;
		}
	}

	buffer = (byte*)calloc(rec_length+2, sizeof(byte));		//not sure if extra 2 are needed but jic
	if( buffer == NULL ) {
		memory_error(errno, rec_length*sizeof(byte) );
		clean_up(0, input, NULL, NULL, file);
		return NULL;
	}

	compute_adj_bytes(input);

	thedata = allocate_arrays(input);
	if( thedata == NULL ) {
		clean_up(1, input, buffer, NULL, file);
		return NULL;
	}

	for(i=0; i<input->num_items; i++) {
		thedata[i].input = &input->attr[i];
		thedata[i].type = input->attr[i].type;
		thedata[i].numbytes = input->attr[i].adj_bytesize;
	}


	for(k=0;k<input->rows;k++)
	{
		if( fread(buffer, 1, rec_length, file) != rec_length ) {
			parse_error("Error reading data: %s", strerror(errno) );
			clean_up(2, input, buffer, thedata, file);
			return NULL;
		}

		if( !read_data(buffer, thedata, input, file, k) ) {
			clean_up(2, input, buffer, thedata, file);
			return NULL;
		}
	}


// upgrade types if necessary (ie unsigned shorts and ints to ints and doubles)
	if( !upgrade_types(thedata, input) ) {
		clean_up(2, input, buffer, thedata, file);
		return NULL;
	}

#ifdef FF_UNPACK_DEBUG
	print_data(thedata, input);
#endif
	
	//set these parameters so calling function has them to loop through data
	*numitems = input->num_items;
	*ret_rows = input->rows;

	//cleanup
	free(buffer);
	free(input);
	fclose(file);

	return thedata;
}

/* clean up function called on errors */
static void clean_up(int level, unpack_digest* input, byte* buffer, data* the_data, FILE* file)
{
	int i, j;
	int num_items = input->num_items;
	int rows = input->rows;

	if( level > 0 ) {
		free(input->attr);
		free(input);
	}

	if( level > 1 )
		free(buffer);

	if( level > 2 ) {
		for(i=0; i<num_items; i++) { 

			if( the_data->input->type == STRING ) {
				for(j=0; j<rows; j++)
					free(the_data[i].strarray[j]);
					
				free(the_data[i].strarray);

			} else
				free(the_data[i].array);
		}
	}

	if( file != NULL )
		fclose(file);
}


static int validate_template(unpack_digest* input)
{
	int i;

	for(i=0; i<input->num_items; i++) {
		switch(input->attr[i].type)
		{

			case STRING:
				break;
	
			case SIGNED_MSB_INT:
			case UNSIGNED_MSB_INT:
			case SIGNED_LSB_INT:
			case UNSIGNED_LSB_INT:
				if( input->attr[i].bytesize < 1 || input->attr[i].bytesize > 4 ) {
					parse_error("Invalid bytesize (%d) for column %d of type %c\nValid values are 1-4", 
								input->attr[i].bytesize, i+1, input->attr[i].type);
					return 0;
				}
				break;
	
			case LSB_REAL:
				if( input->attr[i].bytesize == 4 ) input->attr[i].type = LSB_FLOAT;
				else if (input->attr[i].bytesize == 8 ) input->attr[i].type = LSB_DOUBLE;
				else {
					parse_error("Invalid bytesize (%d) for column %d of type %c\nValid values are 4 and 8", 
								input->attr[i].bytesize, i+1, input->attr[i].type);
					return 0;
				}
				break;
	
			case MSB_REAL:
				if( input->attr[i].bytesize == 4 ) input->attr[i].type = MSB_FLOAT;
				else if (input->attr[i].bytesize == 8 ) input->attr[i].type = MSB_DOUBLE;
				else {
					parse_error("Invalid bytesize (%d) for column %d of type %c\nValid values are 4 and 8", 
								input->attr[i].bytesize, i+1, input->attr[i].type);
					return 0;
				}
				break;
	
			default:
				parse_error("Invalid type %c in column %d", input->attr[i].type, i+1);
				return 0;
				break;
		}

	}
	return 1;
}



static int valid_letter(char letter)
{
	if( letter != STRING && letter != SIGNED_MSB_INT && letter != UNSIGNED_MSB_INT &&
		letter != SIGNED_LSB_INT && letter != UNSIGNED_LSB_INT && letter != LSB_REAL &&
		letter != MSB_REAL && letter != IGNORE )
		return 0;
	else
		return 1;
}



/* Parse template string */
static unpack_digest* parse_template(char* template, column_attributes* input, Var* column_names, int* record_length)
{
	int i, j = 0, j_no_mult = 0, temp = 0, offset = 0, name_length = 30, error = 0;

	char* end = NULL;
	char* beginning = template;
	unpack_digest* more = NULL;

	int name_count = 0;
	char** names = NULL;


	if( column_names != NULL ) {
		if( V_TYPE(column_names) == ID_TEXT ) {
			name_count = V_TEXT(column_names).Row;
			names = V_TEXT(column_names).text;
		}
		else if( V_TYPE(column_names) == ID_STRING ) {
			name_count = 1;
			names = (char**)calloc(1, sizeof(char*) );
			if( names == NULL ) {
				memory_error(errno, sizeof(char*) );
				return NULL;
			}
			names[0] = V_STRING(column_names);
		}
		else {
			parse_error("Unrecognized type passed in for argument col_names");
			return NULL;
		}
	}

	for(i=0; i<name_count; i++)
		names[i] = fix_name(names[i]);



	while( template[0] != '\0' )						//loop through parsing template
	{
		if( !valid_letter(template[0]) ) {
			error = 1;
			break;
		}

		input[j].type = template[0];
		input[j].start_byte = offset;
		input[j].col_name = NULL;

		if( !isdigit(template[1]) ) {
			input[j].bytesize = 1;
			template = &template[1];
		} else {
			template = &template[1];
			temp = strtol(template, &end, 10);
			if( temp == 0 || template == end) {
                error = 1;
				break;
            }

			input[j].bytesize = temp;
			template = end;
		}


		if(template[0] == '*')
		{
			template = &template[1];
			temp = 0;
			temp = strtol(template, &end, 10);
			if( temp == 0 || template == end) {
                error = 2;
				break;
            }

			input[j].columns = temp;

			//special case (it splits up string with multiplicity >1 into separate columns)
			if( temp > 1 && input[j].type == STRING ) {
				for(i=0; i<temp; i++) {
					input[j+i].bytesize = input[j].bytesize;
					input[j+i].type = STRING;
					input[j+i].columns = 1;
					input[j+i].start_byte = offset;
					offset += input[j].bytesize;


					input[j+i].col_name = (char*)calloc(name_length, sizeof(char));
					if ( input[j+i].col_name == NULL ) {
						memory_error(errno, sizeof(char)*name_length );
						for(--i; i>=0; i--) free(input[j+i].col_name);
						for(--j; j>=0; j--) free(input[j].col_name);
						return NULL;
					}	

					if( name_count == 0 ) {
						if( snprintf(input[j+i].col_name, name_length, "c%d_%d", j+1, i) >= name_length ) {
							parse_error("%s, c%d_%d: %s", "Column name too long", j+1, i, strerror(errno) );
							return NULL;
						}
					} else {
						if( snprintf(input[j+i].col_name, name_length, "%s_%d", names[j_no_mult], i) >= name_length ) {
							parse_error("%s, c%d_%d: %s", "Column name too long", j+1, i, strerror(errno) );
							return NULL;
						}
					}
				}

				j += (i - 1);
				offset -= input[j].bytesize; //subtract one so the normal addition at the end of the
												//while loop doesn't mess it up
			} else {
				offset += ( (temp-1)*input[j].bytesize );
			}


			template = end;
		}
		else
			input[j].columns = 1;


		if( input[j].type != IGNORE ) {
			if( input[j].col_name == NULL ) {
				if( name_count == 0 ) {
					input[j].col_name = (char*)calloc(name_length, sizeof(char));
					if( input[j].col_name == NULL ) {
						memory_error(errno, sizeof(char)*name_length );
						for(--j; j>=0; j--) free(input[j].col_name);
						return NULL;
					}
	
					if( snprintf(input[j].col_name, name_length, "c%d", j+1) >= name_length ) {
						parse_error("%s, c%d: %s", "Column name too long", j+1, strerror(errno) );
						return NULL;
					}
				} else {
					input[j].col_name = strdup(names[j_no_mult]);
					if( input[j].col_name == NULL ) {
						memory_error(errno, sizeof(char)*name_length );
						for(--j; j>=0; j--) free(input[j].col_name);
						return NULL;
					}
				}
			}
		}

		offset += input[j].bytesize;
		if( input[j].type != IGNORE ) {
			j++;						//increment j for next letter(overwrite IGNOREs)
			j_no_mult++;				//increment j not counting multiplicity of strings (for if they provide
		}								//column names

		if( j >= MAX_ATTRIBUTES ) {
			parse_error("Too many attributes, %d is max allowed\n", MAX_ATTRIBUTES);
			return NULL;
		}

		if( name_count != 0 && (j_no_mult >= name_count && template[0] != '\0' && template[0] != IGNORE ) ) {
			parse_error("Too few column names provided, you provided %d\n", name_count);
			for(--j; j>=0; j--) free(input[j].col_name);
			return NULL;
		}
	}	//end parsing while loop


	if( error ) {

		fprintf(stderr, "Unexpected or missing character:  ");
		for(i=0; i<strlen(beginning); i++) {
			if( i == template-beginning ) {
				if( error == 1 )
					fprintf(stderr, ">>>%c<<<", beginning[i]);
				else
					fprintf(stderr, ">>><<<%c", beginning[i]);

				error = 0;
			} else
				fprintf(stderr, "%c", beginning[i]);
		}
		if( error ) fprintf(stderr, ">>><<<");

		fprintf(stderr, "\n");

//		parse_error("Unexpected or missing character:\n%s\n%*c\n", beginning, template-beginning, '^');
		return NULL;
	}

#ifdef FF_UNPACK_DEBUG
	for(i=0; i<j; i++) fprintf(stderr, "%c%d\t", input[i].type, input[i].bytesize);
	fprintf(stderr, "\nnum_items: %d\n", j);
#endif

	more = (unpack_digest*)calloc(1, sizeof(unpack_digest));
	if( more == NULL ) {
		memory_error(errno, sizeof(unpack_digest));
		return NULL;
	}

	more->attr = input;
	more->num_items = j;
	more->rows = -1;

	*record_length = offset;

	return more;
}





/* store number of bytes needed to store data (vs. size of data on disk) */
static void compute_adj_bytes(unpack_digest* input)
{
	int i;
	char a;

	int num_items = input->num_items;
	column_attributes* attr_array = input->attr;

	for(i=0;i<num_items;i++)
	{
		a = attr_array[i].type;
		if(a==SIGNED_MSB_INT || a==UNSIGNED_MSB_INT || a==SIGNED_LSB_INT || a==UNSIGNED_LSB_INT) {
			if( attr_array[i].bytesize == 3 )
				attr_array[i].adj_bytesize = 4;
			else
				attr_array[i].adj_bytesize = attr_array[i].bytesize;
		} 
		else if(a == STRING)
			attr_array[i].adj_bytesize = attr_array[i].bytesize + 1;
		else
			attr_array[i].adj_bytesize = attr_array[i].bytesize;
	}
}


/* Pretty self explanatory */
static int calc_rows(char* filename, int hdr_length, unpack_digest* input, int rec_length)
{
	
	//use filesize,header size, and template to determine number of rows

	struct stat filestats;												//struct to get file size with fstat
	int filesize, i;
	int rows = 0, width = 0;

	if(stat(filename, &filestats) < 0)		//get filesize
	{
		parse_error("stat failed in calc_rows(): %s", strerror(errno) );
		return 0;
	}	
	filesize = filestats.st_size;
	
	
	rows = (filesize - hdr_length)/(rec_length);

	input->rows = rows;

	return 1;
}


/* also pretty self explanatory */
static data* allocate_arrays(unpack_digest* digest)
{
	int i, j = 0, k = 0, mem_num = 0, err_num = 0;
	//char* err_str = "Error in allocate_arrays()";
	int num_items = digest->num_items;
	int rows = digest->rows;
	column_attributes* input = digest->attr;


	data * all_data =(data*)calloc(num_items, sizeof(data));
	if ( all_data == NULL ) {
		memory_error(errno, sizeof(data)*num_items);
		return NULL;
	}

	for(i=0; i<num_items; i++) {

		if( input[i].type == STRING ) {
			all_data[i].strarray = (char**)calloc(rows, sizeof(char*));
			if ( all_data[i].strarray == NULL ) {
				err_num = errno;
				mem_num = rows*sizeof(char*);
				break;
			}

			for(k=0; k<rows; k++) {
				all_data[i].strarray[k] = (char*)calloc(input[i].adj_bytesize, sizeof(char));
				if ( all_data[i].strarray[k] == NULL ) {
					err_num = errno;
					mem_num = input[i].adj_bytesize*sizeof(char);
					break;
				}
			}
			if( k < rows ) break;

			k = 0;
			all_data[i].array = NULL;
		} else {
			all_data[i].array = (byte*)calloc(input[i].columns*input[i].adj_bytesize*rows, sizeof(byte));
			if ( all_data[i].array == NULL ) {
				err_num = errno;
				mem_num = input[i].columns*input[i].adj_bytesize*rows*sizeof(byte);
				break;
			}

			all_data[i].strarray = NULL;
		}
	}

/* clean up code if necessary */
	if( i<num_items ) {
		for(i; i>=0; i--) {

			if( input[i].type == STRING ) {
				for(--k; k>=0; k--)
					free(all_data[i].strarray[k]);

				if( all_data[i].strarray != NULL )
					free(all_data[i].strarray);

				k = rows;
			} else {
				if( all_data[i].array != NULL )
					free(all_data[i].array);
			}
		}

		free(all_data);
		memory_error(err_num, mem_num);
		return NULL;
	}


	return all_data;
}
		

/* print function for debugging */
static int print_data(data* the_data, unpack_digest* input)
{
	int i,j,k;

	double tempdouble;
	float tempfloat;
	int tempint;
	unsigned int utempint;
	short tempshort;
	unsigned short utempshort;
	char signedbyte;

	int bytesize;
	int columns;

	int num_items = input->num_items;
	int rows = input->rows;
	
	for(i=0;i<num_items;i++)
		printf("%c%d\t",the_data[i].type, the_data[i].numbytes);

	printf("\n");
	for(i=0;i<num_items;i++)
		printf("%s\t", the_data[i].input->col_name);

	getchar();
	for(i=0;i<rows;i++) {
		for(j=0;j<num_items;j++)
		{
			bytesize = the_data[j].numbytes;
			columns = the_data[j].input->columns;


			for(k=0;k<columns;k++)
			{
				switch(the_data[j].type)
				{
					case STRING:							//ASCII string space padded
						printf("%s\t",the_data[j].strarray[i]);//&the_data[j].array[(i*columns+k)*bytesize]);
						break;
					
					case MSB_DOUBLE:
					case LSB_DOUBLE:					
						memcpy(&tempdouble, &the_data[j].array[(i*columns+k)*8], 8);
						printf("%G\t",tempdouble);
						break;

					case MSB_FLOAT:
					case LSB_FLOAT:
						memcpy(&tempfloat, &the_data[j].array[(i*columns+k)*4], 4);
						printf("%G\t",tempfloat);
						break;

					case SIGNED_MSB_INT:
					case SIGNED_LSB_INT:
						if(bytesize == 4) {
							memcpy(&tempint, &the_data[j].array[(i*columns+k)*4], 4);
							printf("%d\t",tempint);
						}
						else if(bytesize == 2) {
							memcpy(&tempshort, &the_data[j].array[(i*columns+k)*2], 2);
							printf("%d\t",tempshort);
						}
						else {
							//memcpy(&signedbyte, &the_data[j].array[i*columns[j]+k], 1);
							signedbyte = (char)the_data[j].array[i*columns+k];
							printf("%d\t",signedbyte);
						}
						break;

					case UNSIGNED_MSB_INT:
					case UNSIGNED_LSB_INT:
						if(bytesize == 4) {
							memcpy(&utempint, &the_data[j].array[(i*columns+k)*4], 4);
							printf("%u\t",utempint);
						}
						else if(bytesize == 2) {
							memcpy(&utempshort, &the_data[j].array[(i*columns+k)*2], 2);
							printf("%u\t",utempshort);
						}
						else
							printf("%u\t",the_data[j].array[i*columns+k]);
						break;

					default:
						fprintf(stderr, "P:Unknown type: %c\n", the_data[j].type);
						return 0;
						break;
				}
			}
		}
		printf("\n");
		//getchar();
	}
	return 1;
}	


/* 
	reads data into buf, performs necessary manipulations (like byteswapping)
	then stores into correct location (loc) in the_data->array
*/
static int read_data(byte* buffer, data *the_data, unpack_digest* input, FILE* file, int row) 
{
	int tempint = 0;
	unsigned int utempint = 0;

	byte* mybyte = NULL;
	byte* loc = NULL;
	byte* buf = NULL;

	int i,j, k;

	char letter;
	int numbytes;
	int al_bytes;
	int columns;

	for(j=0; j<input->num_items; j++) {
		letter = input->attr[j].type;
		numbytes = input->attr[j].bytesize;
		al_bytes = input->attr[j].adj_bytesize;
		columns = input->attr[j].columns;

		buf = &buffer[input->attr[j].start_byte];	//skip to offset

		for(k=0; k<columns; k++) {

			if( letter != STRING )
				loc = &the_data[j].array[(row*columns+k)*al_bytes];

			switch(letter)
			{
				case STRING:							//ASCII string space padded
					memcpy(the_data[j].strarray[row], buf, numbytes);
					the_data[j].strarray[row][numbytes] = '\0';
					
					break;
				
				case LSB_DOUBLE:							//double precision float (LSB)
				case LSB_FLOAT:								//single precision float (LSB)
					LSB(buf, 1, numbytes);
					memcpy(loc, buf, al_bytes);

					break;

				case MSB_DOUBLE:							//double precision float (MSB)
				case MSB_FLOAT:							//single precision float (MSB)
					MSB(buf, 1, numbytes);
					memcpy(loc, buf, al_bytes);

					break;

				case SIGNED_MSB_INT:
				case SIGNED_LSB_INT:
					tempint = 0;
					mybyte = (byte*)&tempint;

				#ifdef WORDS_BIGENDIAN	
					if(letter == SIGNED_LSB_INT) {
						for(i=0;i<numbytes;i++)
							mybyte[3-(4-numbytes)-i] = buf[i];
						tempint = tempint >> ((4-numbytes)*8);
					} else {
						for(i=0;i<numbytes;i++)
							mybyte[i] = buf[i];	
						tempint = tempint >> ((4-numbytes)*8);
					}
					
					memcpy(loc, &mybyte[4-al_bytes], al_bytes);

				#else
					if(letter == SIGNED_MSB_INT) {
						for(i=0;i<numbytes;i++)
							mybyte[3-i] = buf[i];
						tempint = tempint >> ((4-numbytes)*8);
					} else {
						for(i=0;i<numbytes;i++)
							mybyte[3-(numbytes-1)+i] = buf[i];	
						tempint = tempint >> ((4-numbytes)*8);
					}
					memcpy(loc, mybyte, al_bytes);

				#endif
					break;

				case UNSIGNED_MSB_INT:
				case UNSIGNED_LSB_INT:
					utempint = 0;
					mybyte = (byte*)&utempint;

				#ifdef WORDS_BIGENDIAN
					if(letter == UNSIGNED_LSB_INT) {
						for(i=0;i<numbytes;i++)
							mybyte[3-i] = buf[i];
					} else {
						for(i=0;i<numbytes;i++)
							mybyte[i+(4-numbytes)] = buf[i];
					}

					memcpy(loc, &mybyte[4-al_bytes], al_bytes);

				#else
					if(letter == UNSIGNED_MSB_INT) {
						for(i=0;i<numbytes;i++)
							mybyte[numbytes-1-i] = buf[i];
					} else {
						for(i=0;i<numbytes;i++)
							mybyte[i] = buf[i];
					}

					memcpy(loc, mybyte, al_bytes);

				#endif

					break;

				default:
					parse_error("read_data(): Unknown type: %c\n", letter);
					return 0;
					break;

			}//end switch type

			buf = &buf[numbytes];	//skip forward for multiplicity if necessary


		}//end for columns
	}//end for num_items

	return 1;
}


/*
	resize types if necessary because davinci only supportes unsigned bytes
	and signed shorts and ints.  So this checks and possibly converts the following:
		signed bytes 	->		shorts (required conversion)
		unsigned shorts	->		ints
		unsigned ints	->		doubles (floats have inadequate precision)
*/

static int upgrade_types(data* the_data, unpack_digest* input)
{
	int i,j;

	char type;
	int bytesize;
	int columns;

	int fields = input->num_items;
	int rows = input->rows;

#ifdef FF_UNPACK_DEBUG
	fprintf(stdout, "\n");
#endif

	for(i=0;i<fields;i++)
	{
		type = the_data[i].input->type;
		bytesize = the_data[i].input->bytesize;
		columns = the_data[i].input->columns;

		switch(type)
		{	
			case SIGNED_MSB_INT:
			case SIGNED_LSB_INT:
				if( bytesize==1 ) {
					j = test_signed_char(&the_data[i], rows, columns);

					if( j<rows*columns ) {
						if( !resize_array(&the_data[i], rows, columns, 0) )
							return 0;

					} else {
						the_data[i].type = UNSIGNED_LSB_INT;
					}
				}
				break;

			case UNSIGNED_MSB_INT:
			case UNSIGNED_LSB_INT:
				if( bytesize==4 ) {
					j = test_int_max(&the_data[i], rows, columns);
					
					if( j < rows*columns ) {
						if( !resize_array(&the_data[i], rows, columns, 2) )
							return 0;

						the_data[i].type = LSB_DOUBLE;

					} else {
						the_data[i].type = SIGNED_LSB_INT;
					}					
				}

				if( bytesize == 3 )
					the_data[i].type = SIGNED_LSB_INT;

				if( bytesize==2 ) {
					j = test_signed_short(&the_data[i], rows, columns);
					
					if( j<rows*columns ) {
						if( !resize_array(&the_data[i], rows, columns, 1) )
							return 0;
					}

					the_data[i].type = SIGNED_LSB_INT;
				}	
				break;


			default:

				break;
		}
	}

	return 1;
}

static int resize_array(data* the_data, int rows, int columns, int type)
{
	int i = 0;
	int size = 0;

	short* tmpshrtarray = NULL;
	int* tmpintarray = NULL;
	double* tmpdblarray = NULL;
	void* temp_ptr = NULL;


	if( type == 0 ) size = 2;
	else if( type == 1 ) size = 4;
	else if( type == 2 ) size = 8;
	else {
		parse_error("Unrecognized type constant in resize_array(): %d", type);
		return 0;
	}

	temp_ptr = the_data->array;
	the_data->array = (byte*)realloc(the_data->array, size*rows*columns);
	if( the_data->array == NULL ) {
		memory_error(errno, size*rows*columns);
		the_data->array = (byte*)temp_ptr;
		return 0;
	}

	switch(type) 
	{
		case 0:
			tmpshrtarray = (short*)the_data->array;
			for(i=rows*columns-1; i>=0; i--)
				tmpshrtarray[i] = *((char*)(&the_data->array[i]));
			break;

		case 1:
			tmpintarray = (int*)the_data->array;
			for(i=rows*columns-1; i>=0; i--)
				tmpintarray[i] = *(unsigned short*)&the_data->array[i*2];
			break;

		case 2:
			tmpdblarray = (double*)the_data->array;
			for(i=rows*columns-1; i>=0; i--)
				tmpdblarray[i] = *(unsigned int*)&the_data->array[i*4];
			break;
	}

	the_data->numbytes = size;

	return 1;
}

static int test_signed_char(data* the_data, int rows, int columns)
{
	int i;
	char tempchar = 0;
	for(i=0; i<rows*columns; i++) {
		memcpy(&tempchar, &the_data->array[i], 1);
		if(tempchar < 0)
			break;
	}
	return i;
}

static int test_int_max(data* the_data, int rows, int columns)
{
	int i;
	unsigned int uint = 0;
	for(i=0; i<rows*columns; i++) {
		memcpy(&uint, &the_data->array[i*4], 4);
		if(uint > INT_MAX)
			break;
	}
	return i;
}

static int test_signed_short(data* the_data, int rows, int columns)
{
	int i;
	short tempshort = 0;
	for(i=0; i<rows*columns; i++) {
		memcpy(&tempshort, &the_data->array[i*2], 2);
		if(tempshort < 0)
			break;
	}
	return i;
}



