#include "parser.h"
#include "dvio.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>


//#define MEM_ERR_STR(x)	snprintf(err_str, 100, "Error allocating memory, %d: ", (x))



static int print_headers(Var** data, char** keys, int count, FILE* file, char* separator);

static void memory_error(int error_num, int mem_size)
{
	char err_str[100];
	
	snprintf(err_str, sizeof(err_str)-1, "Error allocating memory, %d", mem_size);
	parse_error("%s: %s", err_str, strerror(error_num));

	return;
}	



int dv_WriteCSV(Var* the_data, char* filename, char* separator, int header, int force)
{
	int i, j, count, row, max, columns, error;
	Var** data = NULL;
	char** keys = NULL;
	FILE* file = NULL;
	struct stat filestats;

	if( separator == NULL )
		separator = "\t";


	i = stat(filename, &filestats);
	if ( i == -1 && errno != ENOENT) {
		parse_error("stat error: %s", strerror(errno) );
		return 0;
	} else if( i == 0 && force == 0) {
		parse_error("File already exists: %s\n", filename);
		return 0;
	}

	error = 0;

	if( V_TYPE(the_data) == ID_STRUCT ) {		//data is a structure so get sub structs
		count = get_struct_count(the_data);

		data = (Var**)calloc(count, sizeof(Var*));
		keys = (char**)calloc(count, sizeof(char*));
		if( data == NULL || keys == NULL) {
			memory_error(errno, count*(sizeof(Var*)+sizeof(char*)) );
			error = 1;
		}

		if( !error ) {
			for(i=0; i<count; i++) {
				if( get_struct_element(the_data, i, &keys[i], &data[i] ) == -1 ) {
					parse_error("Error retrieving structure elements at index %d\n", i);
					error = 1;
					break;
				}

				if( data[i] != NULL && V_SIZE(data[i])[2] > 1 ) {
					parse_error("Multiple bands (z>1) not supported.\nColumn %d has z dimension %d\n", 
							i, V_SIZE(data[i])[2] );
					error = 1;
					break;
				}
			}
		}
	} else {	//else data is a basic type so no sub structures
		count = 1;
		keys = (char**)calloc(1, sizeof(char*));
		data = (Var**)calloc(1, sizeof(Var*));
		if(keys == NULL || data == NULL) {
			memory_error(errno, sizeof(char*)+sizeof(Var*) );
			error = 1;
		}

		if( !error ) {
			keys[0] = V_NAME(the_data);
			data[0] = the_data;
		}
	}

	if( error ) {
		free(keys);
		free(data);
	}
		
	

	for(i=0; i<count; i++) {
		if( V_ORG(data[i]) != BSQ ) {
			parse_error("Only BSQ data format supported\nColumn %s is not BSQ\n", keys[i] );
			free(keys);
			free(data);
			return 0;
		}
	}

	file = fopen(filename, "w");
	if( file == NULL ) {
		parse_error("Error opening file \"%s\" : %s", filename, strerror(errno) );
		free(data);
		free(keys);
		return 0;
	}


/* print out column headers if requested */
	if(header) 
		print_headers(data, keys, count, file, separator);


/* calculate max number of rows */
	max = 0;
	for(i=0; i<count; i++)
		if( max < V_SIZE(data[i])[1] ) max = V_SIZE(data[i])[1];


	row = 0;
	for(row = 0; row<max; row++) {
		for(i=0; i<count; i++) {

			if( data[i] == NULL ) {		//combine
				if( i < count-1 )
					fprintf(file, "%s", separator);
				continue;
			}

			//if a column has fewer rows than others
			if( (V_TYPE(data[i]) == ID_VAL && row >= V_SIZE(data[i])[1]) || 
				( V_TYPE(data[i]) == ID_TEXT && V_TEXT(data[i]).Row <= row) ) {		

				if( i < count-1 ) {
					for(j=0; j<V_SIZE(data[i])[0]; j++)
					fprintf(file, "%s", separator);
				}
				continue;
			}


			if( V_TYPE(data[i]) == ID_VAL ) {
				columns = V_SIZE(data[i])[0];
				for(j=0; j<columns; j++) {
					switch(V_FORMAT(data[i]))
					{
						case BYTE:		fprintf(file, "%u", ((unsigned char*)V_DATA(data[i]))[row*columns+j] );	break;
						case SHORT:		fprintf(file, "%d", ((short*)V_DATA(data[i]))[row*columns+j] );			break;
						case INT:		fprintf(file, "%d", ((int*)V_DATA(data[i]))[row*columns+j] );			break;
						case FLOAT:		fprintf(file, "%G", ((float*)V_DATA(data[i]))[row*columns+j] );			break;
						case DOUBLE:	fprintf(file, "%G", ((double*)V_DATA(data[i]))[row*columns+j] );		break;

						default:
							parse_error("unknown format for ID_VAL: %d\n", V_FORMAT(data[i]));
							free(keys);
							free(data);
							fclose(file);
							return 0;

					}
					if( j+1 < V_SIZE(data[i])[0] ) fprintf(file, "%s", separator);
				}
			} else if( V_TYPE(data[i]) == ID_TEXT ) {

				if( V_TEXT(data[i]).text[j] != NULL )
					fprintf(file, "%s", V_TEXT(data[i]).text[row] );

			} else {
				keys[i] = keys[i]==NULL ? "(null)" : keys[i];
				parse_error("Unknown type: %d for column %d: %s\n", V_TYPE(data[i]), i, keys[i] );
				free(keys);
				free(data);
				fclose(file);
				return 0;
			}

			if( i<count-1 ) fprintf(file, "%s", separator);

		}	// end for (columns)

		fprintf(file, "\n");
	}	//end while (rows)

	fclose(file);
	free(data);
	free(keys);

	return 1;
}



static int print_headers(Var** data, char** keys, int count, FILE* file, char* separator)
{
	int i = 0, j = 0;
	char *key = NULL;

	for(i=0; i<count; i++) {

		key = keys[i] == NULL? "": keys[i];

		if( data[i] != NULL && V_SIZE(data[i])[0] > 1) {
			for(j=0; j<V_SIZE(data[i])[0]; j++) {
				if (strlen(key) > 0)
					fprintf(file, "%s[%d]", keys[i], j );
				if( j < V_SIZE(data[i])[0] - 1 ) fprintf(file, "%s", separator);
			}
		} else {
			fprintf(file, "%s", key);
		}

		if( i < count-1 ) fprintf(file, "%s", separator);
	}
	fprintf(file, "\n");

	return 1;
}



