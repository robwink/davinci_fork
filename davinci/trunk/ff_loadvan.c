#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include "tools.h"
#include "toolbox.h"
#include "parser.h"

#define MAX(a,b) (((a) > (b))? (a): (b))
#define MIN(a,b) (((a) < (b))? (a): (b))

/*
** error codes
*/
#define ENCOLS  -2 /* number of data values <> required number of columns */
#define EMEM    -1 /* memory allocation/reallocation error */
#define ETOOBIG -3 /* required buffer size is too big */
#define EINCON  -4 /* structural inconsistency */
#define ETYPE   -5 /* invalid type encountered */
#define EFILE   -6 /* unable to open file -OR- input file problem */


typedef enum {
	TINT,
	TREAL,
	TSTR,
	TNA
} coltype;

struct _coldef;

typedef struct _coldef {
	/* column type extraction data */
	char		*name;				/* probable name */
	char		*text;				/* text as read from the file */
	int		sno;					/* serial number */
	coltype	data_type;			/* data type 0=int 1=real 2=string */
	int		max_len;				/* max text string length */
	double	max_num_val;		/* max value if treated as real */
	int		neg_flag;			/* data has -ve values */
	int		size;					/* size (in bytes) for the type */
	int		naflag;				/* data for this field contains an N/A */

	/* cached data */
	int		dav_type;			/* davinci type of this column */

	/* array specific data, for structure extraction */
	struct	_coldef	*prev;	/* chain for array processing */
	struct	_coldef	*next;	/* chain for array processing */
	int		chain_len;			/* chain-length at the "root"-level only */
} coldef;							/* column definition */

static void
free_cols(
	coldef	*cols,
	int		n
)
{
	int	i;

	for(i = 0; i < n; i++){
		if (cols[i].text) free(cols[i].text);
		if (cols[i].name) free(cols[i].name);
	}

	free(cols);
}


/*
** regular expressions for various data types recognized by davinci
** in a vanilla (text) output file
*/
#include "van.regex.i"

static coltype
data_type(
	const	char *s
)
{
	if (regex(ints, s)) return TINT;
	if (regex(reals, s)) return TREAL;
	if (regex(nas, s)) return TNA;
	return TSTR;
}

/*
** Allocate coldef structures for each of the columns in the vanilla
** input text file. While doing so, store the column names within the
** column definition structure.
**
** Assume every field is an integer unless proven otherwise.
*/
static int
make_cols(
	char		**tokens,
	coldef	**cols,
	int		n
)
{
	int	i, j;
	char	*p, *q;

	*cols = (coldef *)calloc(n, sizeof(coldef));

	if (*cols == NULL){
		return EMEM;
	}

	for(i = 0; i < n; i++){
		if ((q = (*cols)[i].name = strdup(tokens[i])) == NULL){
			free_cols(*cols, n);
			return EMEM;
		}

		/*
		** get rid of table name from the field, i.e.
		**         obs.sclk_time -> sclk_time
		*/
		if (p = strchr(q, '.')){ strcpy((*cols)[i].name, p+1); }

		/*
		** get rid of array dimensions from the field name, i.e.
		**         temps[1] -> temps
		*/
		if (p = strchr(q, '[')){ *p = 0; }

		/*
		** replace ':' with '_' for bit-fields
		*/
		if (p = strchr(q, ':')){ *p = '_'; }

		if (((*cols)[i].text = strdup(tokens[i])) == NULL){
			free_cols(*cols, n);
			return EMEM;
		}

		(*cols)[i].sno = i;
		(*cols)[i].data_type = TINT; /* start as "int", promote if necessary */
		(*cols)[i].max_len = 0;
		(*cols)[i].max_num_val = 0;
		(*cols)[i].size = 0;
		(*cols)[i].naflag = 0;

		(*cols)[i].chain_len = 0;
		(*cols)[i].next = (*cols)[i].prev = NULL;
	}

	return n;
}

/*
** update column defintions based on the current input (text) data record
*/
static void
update_coldefs_per_data_row(
	char		**tokens,
	coldef	*cols,
	int		n
)
{
	int		i;
	coltype	c;
	double	d, ad;

	for(i = 0; i < n; i++){
		cols[i].max_len = MAX(cols[i].max_len, strlen(tokens[i]));

		/*
		** if hex-values are added and are to be treated as ints, then the
		** following line has to be modified, besides the type modification
		*/
		ad = fabs(d = atof(tokens[i]));
		cols[i].max_num_val = MAX(ad, cols[i].max_num_val);
		cols[i].neg_flag = cols[i].neg_flag || (d < 0);

		if ((c = data_type(tokens[i])) == TNA){
			/* if 'N/A' or 'n/a' is found in the data, set na_flag */
			cols[i].naflag = 1;
		}
		else {
			/* promote data types as required */
			cols[i].data_type = MAX(c, cols[i].data_type);
		}
	}
}

/*
** Fix the "size" field of coldef structure for each of the column
** definitions based on the types recognized by davinci.
*/
static void
fill_col_min_size(
	coldef	*cols,
	int		n
)
{
	int		i;

	for(i=0; i<n; i++){
		switch(cols[i].data_type){
		case TINT:
			if((int)cols[i].max_num_val > SHRT_MAX){
				cols[i].size = sizeof(int);
			}
			else if(((int)cols[i].max_num_val > CHAR_MAX) && !cols[i].neg_flag){
				cols[i].size = sizeof(short);
			}
			else {
				/* davinci char-type values cannot be -ve */
				cols[i].size = sizeof(char);
			}
			break;
		
		case TREAL:
			if(cols[i].max_num_val > FLT_MAX){
				cols[i].size = sizeof(double);
			}
			else {
				cols[i].size = sizeof(float);
			}
			break;

		case TSTR:
			/* well in this case, the length is the maximum length */
			cols[i].size = cols[i].max_len;
			break;
		}
	}
}

/*
** return the davinci type based on the column type and size pair
*/
static int
davinci_type(
	coltype	c,			/* column type: TINT, TREAL, ... */
	int		csize		/* coldef.size */
)
{
	int		dav_type;

	switch(c){
	case TINT:
		switch(csize){
		case 1: dav_type = BYTE; break;
		case 2: dav_type = SHORT; break;
		default: dav_type = INT; break;
		}
		break;

	case TREAL:
		switch(csize){
		case 4: dav_type = FLOAT; break;
		default: dav_type = DOUBLE; break;
		}
		break;

	case TSTR:
	default:
		/* does not apply otherwise */
		dav_type = -1;
		break;
	}

	return dav_type;
}

/*
** dynamic string parameters for reading in an input record
*/
#define BUFF_SZ  8192
#define SZ_STEP  8192
#define CHOKE_SZ 100000


/*
** Guess various column types from the input data based on the input
** text file. First line of the input file gives the column names and
** the number of columns per row.
**
** The allocated column descriptions are returned in "cols", with the
** number of columns returned in "ncols".
**
** Total number of input records is returned as the function return
** value. In case of an error this value is less than zero.
*/
static int
guess_col_data_types(
	FILE		*fp,				/* opened input file pointer */
	const		char *delim,	/* column delimiters */
	coldef	**cols,			/* ARRAY<coldefs> allocated and returned here */
	int		*ncols			/* number of columns found */
)
{
	char	**tokens = NULL;	/* per line tokens list stored here */
	int	ntk;					/* number of elements in the tokens array */
	int	tka = 0;				/* tokens array's current allocation */
	char	**tt;					/* temporary variable linked to tokens */
	char	*p, *q;				/* temp vars used for breaking lines */

	int	rc;					/* return code */

	int	line_no;				/* current input line number */
	char	*buff, *tbuff;		/* current line buffer */
	int	bufflen;				/* buffer's current allocation */

	bufflen = BUFF_SZ;
	buff = (char *)malloc(bufflen);

	line_no = 0;
	while(fgets(buff, BUFF_SZ, fp)){
		line_no ++;

		/* read one full line; extend the buffer if required */
		while(!strchr(buff, '\n')){
			if ((bufflen += SZ_STEP) >= CHOKE_SZ){
				free(buff);
				fprintf(stderr, "Input text line is too big (> %ld)\n", CHOKE_SZ);
				return ETOOBIG;
			}

			if ((tbuff = (char *)realloc(buff, bufflen)) == NULL){
				free(buff);
				return EMEM;
			}
			buff = tbuff;

			if(!fgets(buff+strlen(buff), SZ_STEP, fp)){
				/* if end-of-file, consider current line complete */
				break;
			}
		}

		/* get rid of new-lines */
		StripTrailing(buff, '\n');

		/* break input line into tokens */
		for(ntk = 0, q = buff; p = strtok(q, delim); q = NULL){
			if (ntk >= tka){
				tka = ((tka == 0)? 10: tka * 2);
				if ((tt = (char **)realloc(tokens, tka*sizeof(char *))) == NULL){
					free(buff);
					if (tokens) free(tokens);
					if (cols) free_cols(*cols, *ncols);
					return EMEM;
				}

				tokens = tt;
			}

			tokens[ntk++] = p;
		}

		switch(line_no){
		case 1:
			/* read column heading and determine the number of fields */
			if ((rc = make_cols(tokens, cols, *ncols = ntk)) < 0){
				free(buff);
				free(tokens);
				return rc;
			}
			break;

		default:
			/* process data values */
			if (ntk != *ncols){
				free(buff);
				free(tokens);
				if (cols) free_cols(*cols, *ncols);
				fprintf(stderr, "Input file is jagged @line %d\n.", line_no);
				return ENCOLS;
			}
			update_coldefs_per_data_row(tokens, *cols, ntk);
			break;
		}
	}

	/* establish minimum size (in bytes) for the field */
	fill_col_min_size(*cols, *ncols);

	free(buff);
	if (tokens) free(tokens);
	return line_no;
}

/*
** Guess the structure based on the column definitions passed in.
**
** Columns with same name are considered as members of the same
** array with their order preserved from left to right.
**
** Array element columns are chained off of the first array element
** column, called the "root"-element or "root"-column.
**
** A list of "root"-columns is returned in LIST "l" and their
** number is returned as the function return value. If the function
** return value is less than zero then an error has occurred.
*/
static int
guess_struct(
	coldef	*cols,
	int		n,
	LIST		**l
)
{
	int		i, j;
	int		nfields;
	coldef	*t;

	for(i = 0; i < n; i++){

		for(j = i+1; j < n; j++){

			if (strcmp(cols[i].name, cols[j].name) == 0){
				/*
				** If column name match, combine the columns into array.
				** This array is constructed by using the "next" and "prev"
				** pointers within coldef structure.
				*/

				cols[i].next = &cols[j];
				cols[j].prev = &cols[i];

				/*
				** we'll worry about field type promotion and column lengths
				** later, once we have all the columns for a field linked in
				*/

				/*
				** One name match is found, stop processing this i'th column and
				** move to the next. This break is necessary for preserving
				** chains of array columns.
				*/
				break;
			}
		}
	}

	if((*l = new_list()) == NULL){
		return EMEM;
	}

	/* count number of fields */
	nfields = 0;
	for(i = 0; i < n; i++){
		/*
		** if prev pointer set, then this record belongs to an
		** array chain, so skip it, as it has already been considered
		*/
		if (cols[i].prev) continue;

		/*
		** Fill chain-length in the column definition
		** this value is the array dimension for the field
		** under consideration.
		**
		** Also determine is final field size and type based on
		** the elements of the field (if it is an array field)
		*/

		cols[i].chain_len = 1;

		for (t = cols[i].next; t; t = t->next){
			cols[i].chain_len ++;
			cols[i].data_type = MAX(t->data_type, cols[i].data_type);
			cols[i].size = MAX(t->size, cols[i].size);
		}

		for (t = cols[i].next; t; t = t->next){
			t->data_type = cols[i].data_type;
			t->size = cols[i].size;
		}

		/* save davinci type of the column for future use */
		cols[i].dav_type = davinci_type(cols[i].data_type, cols[i].size);

		/* add this field to the list of "root"-level fields */
		list_add(*l, &cols[i]);

		/* increment field count */
		nfields ++;
	}

	/*
	** If the input columns were:
	**
	**           "sclk_time temps[1] temps[2] det temps[3]"
	**
	** then the LIST *l should be arranged as follows:
	**
	** l[0] = coldef<"sclk_time">
	**
	** l[1] = coldef<"temps"> 
	**        + l[1]->next points to chain of coldef<"temps">
	**        each of l[1]->next's is an element of the array "temps"
	**
	** l[2] = coldef<"det">
	**
	*/

	return nfields;
}

/*
** Preallocates storage space for the ("struct") data to be returned.
** Returns NULL on error.
*/ 
static Var *
alloc_davinci_data_space(
	LIST	*l,		/* list of "root"-"coldef"s */
	int	nrecs		/* total number of input records */
)
{
	coldef	**c;
	int		nc;
	int		i, j;
	int		cl;			/* chain length: number of array elements */
	int		dt;			/* davinci type corresponding to (Ttype, byte-size) */
	Var		*v_return;	/* Returned davinci variable */
	Var		*v_t;
	void		*tdata;

	c = (coldef **)list_data(l);
	nc = list_count(l);

	/* create a blank structure */
	v_return = new_struct(0);

	for(i=0; i<nc; i++){
		cl = c[i]->chain_len;

		switch(c[i]->data_type){
		case TINT:
		case TREAL:
			tdata = calloc(nrecs, cl * c[i]->size);
			dt = c[i]->dav_type;
			v_t = newVal(BSQ, cl, nrecs, 1, dt, tdata);
			break;

		case TSTR:
			#if 0
			/*
			** since davinci's text data-type can only handle
			** one dimension, i.e. lines (of text), we will end
			** up concatenating the text array elements together
			** with spaces in-between.
			*/
			tdata = calloc(nrecs,
							/* data */		(cl*c[i]->size) +
							/* spaces */	(cl-1) +
							/* '\0' */		1);
			#endif 

			tdata = calloc(nrecs, sizeof(char *));
			v_t = newText(nrecs, tdata);
			for(j=0; j<nrecs; j++){
				((char **)tdata)[j] = calloc(1,
							/* data */     (cl*c[i]->size) +
							/* spaces */   (cl-1) +
							/* '\0' */     1);

			}
			break;
		
		default:
			fprintf(stderr,
				"Invalid type %d encountered. Fix %s.\n",
				c[i]->data_type, __FILE__);
			return NULL;
		}
		/* fprintf(stderr, "Adding %s to v_return\n", c[i]->name); */
		add_struct(v_return, c[i]->name, v_t);
	}

	return v_return;
}

/*
** Reads the data from the input file using the structure passed thru
** the LIST "l".
*/
static int
read_data(
	FILE	*fp,			/* file to read data from */
	LIST	*l,			/* list of "root"-"coldef"s */
	int	nrecs,		/* number of records as determined by the previous steps */
	char	*delim,		/* input delimiter */
	Var	**v_return	/* allocated davinci structure is returned here */
)
{
	char		**tokens = NULL;	/* per line tokens list stored here */
	int		ntk;					/* number of elements in the tokens array */
	int		tka = 0;				/* tokens array's current allocation */
	char		**tt;					/* temporary variable linked to tokens */
	char		*p, *q;				/* temp vars used for breaking lines */

	coldef	**c, *t;
	int		nc;
	void		*data, *d;
	char		*buff, *tbuff;
	int		bufflen;
	int		line_no;
	void		*tdata;
	char		*s;
	LIST		*tlist;
	int		i, j;

	int		idx;					/* index into elements of the arrays below */

	/*
	** arrays to access davinci-typed data in davinci variables, i.e.
	** "Var *"s
	*/
	void		**void_data = NULL;
	char		**byte_data = NULL;
	short		**short_data = NULL;
	int		**int_data = NULL;
	float		**float_data = NULL;
	double	**double_data = NULL;
	char		***str_data = NULL;

	c = (coldef **)list_data(l);
	nc = list_count(l);

	/*
	** void_data points to the actual elements and then arrays of various
	** (supported) davinci types point to the void_data array.
	**
	** this provides a clean access mechanism, instead of type-casts
	*/
	void_data = (void **)calloc(nc, sizeof(void *));
	byte_data = (char **)void_data;
	short_data = (short **)void_data;
	int_data = (int **)void_data;
	float_data = (float **)void_data;
	double_data = (double **)void_data;
	str_data = (char ***)void_data;

	/*
	** allocate memory to start off the copying operation
	**
	** note: nrecs = no of data lines + header line
	*/
	*v_return = alloc_davinci_data_space(l, nrecs-1);

	/*
	** point to that memory segment using all available types
	*/
	for (i = 0; i < nc; i++){
		switch(c[i]->data_type){
		case TSTR:
			void_data[i] = (void *)V_TEXT(V_STRUCT(*v_return).data[i]).text;
			break;
		
		default:
			void_data[i] = (void *)V_DATA(V_STRUCT(*v_return).data[i]);
			break;
		}
	}

	bufflen = BUFF_SZ;
	buff = (char *)malloc(bufflen);

	/*
	** read input lines and stick them into a davinci buffer
	*/

	line_no = 0;
	while(fgets(buff, BUFF_SZ, fp)){
		line_no ++;

		if (line_no > nrecs){
			fprintf(stderr,
				"Input file is volatile: #recs has increased since last pass.\n");
			fprintf(stderr, "Returning %d records only.\n", nrecs);
			break;
		}

		/* read one full line; extend the buffer if required */
		while(!strchr(buff, '\n')){
			if ((bufflen += SZ_STEP) >= CHOKE_SZ){
				free(buff);
				fprintf(stderr, "Input line length > %d is too big.\n", CHOKE_SZ);
				return EMEM;
			}

			if ((tbuff = (char *)realloc(buff, bufflen)) == NULL){
				free(buff);
				fprintf(stderr, "Mem allocation failed for %d bytes.\n", bufflen);
				return EMEM;
			}
			buff = tbuff;

			if(!fgets(buff+strlen(buff), SZ_STEP, fp)){
				/* if end-of-file, consider current line complete */
				break;
			}
		}

		/* get rid of new-lines */
		StripTrailing(buff, '\n');

		/* break input line into tokens */
		for(ntk = 0, q = buff; p = strtok(q, delim); q = NULL){
			if (ntk >= tka){
				tka = ((tka == 0)? 10: tka * 2);
				if ((tt = (char **)realloc(tokens, tka*sizeof(char *))) == NULL){
					free(buff);
					if(tokens) free(tokens);
					return EMEM;
				}

				tokens = tt;
			}

			tokens[ntk++] = p;
		}

		/* skip first-line (which is the header line) */
		if (line_no > 1){

			/*
			** read each of the columns
			*/
			for(i=0; i<nc; i++){
				/*
				** note that if the current column is an array, the
				** array elements are linked to the "root"-coldef in
				** a chain
				*/
				for(j=0, t=c[i]; t; t=t->next, j++){
					switch(t->data_type){
					case TINT:
						idx = (line_no-2) * c[i]->chain_len + j;

						switch(c[i]->dav_type){
						case BYTE:
							byte_data[i][idx] = atoi(tokens[t->sno]); break;
						
						case SHORT:
							short_data[i][idx] = atoi(tokens[t->sno]); break;
						
						default:
						case INT:
							int_data[i][idx] = atoi(tokens[t->sno]); break;
						}
						break;

					case TREAL:
						idx = (line_no-2) * c[i]->chain_len + j;

						switch(c[i]->dav_type){
						case FLOAT:
							float_data[i][idx] = atof(tokens[t->sno]); break;

						default:
						case DOUBLE:
							double_data[i][idx] = atof(tokens[t->sno]); break;
						}
						break;

					case TSTR:
						/*
						** assume that the string was initialized to null at its
						** creation
						*/
						s = str_data[i][line_no-2];

						strcat(s, tokens[t->sno]);
						if (t->next != NULL){ strcat(s, " "); }
						break;
					
					default:
						fprintf(stderr, "If you can read this, fix %s.\n", __FILE__);
						return ETYPE;
					}
				}
			}
		}
	}

	free(buff);
	free(void_data);

	return 1;
}

/*
** Top-level function for reading vanilla data. It is called by
** the davinci "load_vanilla" function.
*/
static int
van_read(
	char	*filename,
	char	*delim,
	Var	**v_return
)
{
	FILE		*fp;
	coldef	*cols = NULL;
	int		ncols, nfields, rc, nrecs;
	LIST		*coll = NULL;	/* column list */

	if ((fp = fopen(filename, "rt")) == NULL){
		fprintf(stderr, "Unable to open %s.\n", filename);
		return EFILE;
	}

	/*
	** First pass through the file revelas the file structure
	*/

	/*
	** guess number of columns and column types by passing through
	** the data
	*/
	if ((rc = nrecs = guess_col_data_types(fp, delim, &cols, &ncols)) < 0){
		fclose(fp);
		return rc;
	}

	/*
	** guess the likely structure of data, i.e. fields of same name
	** are considered to be part of an array, only if they are of
	** compatable types
	*/
	if ((rc = nfields = guess_struct(cols, ncols, &coll)) < 0){
		fclose(fp);
		free_cols(cols, ncols);
		return rc;
	}

	/*
	** Second pass through the file loads the data into memory
	** into a davinci structure
	*/

	/* ready the input file */
	rewind(fp);

	if ((rc = read_data(fp, coll, nrecs, delim, v_return)) < 0){
		fclose(fp);
		free_cols(cols, ncols);
		list_free(coll);
		return rc;
	}


	list_free(coll);
	free_cols(cols, ncols);

	return 1;
}

/**************************************************************************
** davinci function - load_vanilla("filename" [, "delimiter"])           **
***************************************************************************/
Var *
ff_loadvan(
	vfuncptr	func,
	Var		*args
)
{
	int		ac;
	Var		**av;
	char		*file_arg = NULL, *delim_arg = NULL;
	char		*delim = NULL;
	Var		*v_return;
	int		rc;

	Alist		alist[3];
	alist[0] = make_alist( "file",   ID_STRING,    NULL,        &file_arg);
	alist[1] = make_alist( "delim",  ID_STRING,    NULL,        &delim_arg);
	alist[2].name = NULL;

	make_args(&ac, &av, func, args);
	if (parse_args(ac, av, alist)) return(NULL);

	if (file_arg == NULL){
		parse_error("%s(): \"%s\" must be specified.\n",
			func->name, alist[0].name);
		return NULL;
	}

	if (delim_arg == NULL){
		delim_arg = "\t "; /* default delimiter is space or tab */
	}

	if ((rc = van_read(file_arg, delim_arg, &v_return)) < 0){
		if (rc == EMEM){ fprintf(stderr, "Fatal! Memory allocation failure."); }
		parse_error("%s(): Failed! with rc=%d. See previous messages.",
			func->name, rc);
		return NULL;
	}

	return v_return;
}
