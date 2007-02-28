#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include "parser.h"

typedef enum { 
	B5WT_BYTE           = 1,
	B5WT_INTEGER        = 2,
	B5WT_LONG           = 3,
	B5WT_FLOAT          = 4,
	B5WT_DOUBLE         = 5,
	B5WT_FLOAT_COMPLEX  = 6,
	B5WT_STRING         = 7,
	B5WT_STRUCTURE      = 8,
	B5WT_DOUBLE_COMPLEX = 9,
	B5WT_PTR            = 10,
	B5WT_OBJ_REF        = 11,
	B5WT_UINTEGER       = 12,
	B5WT_ULONG          = 13,
	B5WT_LONGLONG       = 14,
	B5WT_ULONGLONG      = 15
} WordType; /* IDL word type */

static int typeSizes[B5WT_ULONGLONG+1] = {
	0,
	1,
	2,
	4,
	4,
	8,
	0,
	0,
	0,
	0,
	0,
	0,
	2,
	4,
	8,
	8
};

static int internalTypes[B5WT_LONGLONG+1] = {
	0,
	BYTE,
	SHORT,
	INT,
	FLOAT,
	DOUBLE,
	0,
	0,
	0,
	0,
	0,
	0,
	SHORT,
	INT,
	0,
	0
};


typedef enum {
	ARCH_X86 = 1,
	ARCH_SUN = 2
} archIDs; /* encoded architecture ids */

char arch_strs[2][6] = {
	"x86  \0",
	"sun  \0"
};

int arch_ids[2] = {
	ARCH_X86,
	ARCH_SUN
};


typedef struct {
	int   ndim;
	int  *dims;
	int   word_type;
	int   nel;
	int   lbl_len;
	char *arch;
	int   arch_id;
	char *text;
} B5H;


static void
free_B5H(B5H **b5h)
{
	free((*b5h)->arch);
	free((*b5h)->text);
	free((*b5h)->dims);
	free(*b5h);
	*b5h = NULL;
}

static int
get_arch_id(const char *arch)
{
	int i, n;

	n = sizeof(arch_ids)/sizeof(int);
	for(i = 0; i < n; i++){
		if (strcmp(arch_strs[i], arch) == 0){
			return arch_ids[i];
		}
	}

	return 0;
}

static void
byte_swap(char *buff, int nbytes)
{
	int i, n;
	char t;

	n = nbytes/2;
	for(i = 0; i < n; i++){
		t = buff[i];
		buff[i] = buff[nbytes-i-1];
		buff[nbytes-i-1] = t;
	}
}

static int
read_bin5_header(const char *fname, B5H **b5h)
{
	FILE *fp = NULL;
	char *buff = NULL;
	int   buff_sz = 1;
	int   block_sz = 512;
	char *lbl_end_marker = "C_END";
	int   N = 0, nel;
	int  *dims = NULL;
	int   l_dbl_angle = 0;
	char *p, *q;
	int   data_type = -1;
	char *arch;
	int   i;
	int   text_len;
	char *text = NULL;


	if ((fp = fopen(fname, "rb")) == NULL){
		fprintf(stderr, "Unable to open %s\n", fname);
		return 0;
	}

	do {
		buff_sz += block_sz;
		if ((buff = (char *)realloc(buff, buff_sz)) == 0){
			fprintf(stderr, "Mem alloc failed %d bytes\n", buff_sz);
			return 0;
		}

		bzero(&buff[buff_sz-block_sz-1], block_sz+1);
		if (fread(&buff[buff_sz-block_sz-1], block_sz, 1, fp) != 1){
			fprintf(stderr, "Short read in file %s\n", fname);
			free(buff);
			return 0;
		}
	} while(strncmp(&buff[buff_sz-1-strlen(lbl_end_marker)],
			lbl_end_marker, strlen(lbl_end_marker)) != 0);

	/* if we reached here we have a text string ending in lbl_end_marker */

	/* get the data block size & type */
	for(q=buff, i=0; p=strtok(q," "); q=NULL, i++){
		if (i == 0){
			/* we got the number of dimensions = N */
			N = atoi(p);
			dims = (int *)calloc(sizeof(int), N);
		}
		else if (i <= N){
			/* get individual dimensions */
			dims[i-1] = atoi(p);
		}
		else if (i == (N+1)){
			/* get data type -- should be one of wordType enums */
			data_type = atoi(p);
		}
		else if (i == (N+2)){
			/* number of elements = dims[0] * dims[1] * .. * dims[N-1] */
			nel = atoi(p);
		}

		if (i == (N+2)){
			/* done with getting the data block size & type */
			break;
		}
	}

	if (i > 0){
		/* move to the start of next token */
		p = &p[strlen(p)+1];

		if ((q = strstr(p, ">>")) != NULL){
			text_len = (&buff[buff_sz-1-strlen(lbl_end_marker)-5]-&q[2]);
			text = (char *)calloc(text_len+1, 1);
			strncpy(text, &q[2], text_len);
		}
		else {
			text = strdup("");
		}
	}

	/* get the source architecture of the file -- it is the 5-chars before lbl_end_marker */
	arch = (char *)calloc(sizeof(char), 6);
	strncpy(arch, &buff[buff_sz-1-strlen(lbl_end_marker)-5], 5);

/*
	fprintf(stderr, "%s: N=%d [", fname, N);
	for(i = 0; i < N; i++){ fprintf(stderr, "%d%s", dims[i], ((i<(N-1))?"x":"")); }
	fprintf(stderr, "] word-type=%d nel=%d arch=\"%s\"\n", data_type, nel, arch);
	fprintf(stderr, "text=\"%s\"\n", text);
*/

	/* assemble data product header structure */
	(*b5h) = (B5H *)calloc(sizeof(B5H), 1);
	(*b5h)->ndim = N;
	(*b5h)->dims = dims;
	(*b5h)->word_type = data_type;
	(*b5h)->nel = nel;
	(*b5h)->lbl_len = buff_sz-1;
	(*b5h)->arch = arch;
	(*b5h)->arch_id = get_arch_id(arch);
	(*b5h)->text = text;

	fclose(fp);

	return 1;
}

static int
cpos_n(int n, int *dim, int *pos)
{
	int cpos = 0;
	int i;

	/* dim:[X,Y,Z,W]; pos:[x,y,z,w]; cpos:x+X*(y+Y*(z+Z*(w))) */

	for(i=n-1; i>0; i--){
		cpos = dim[i-1] * (pos[i] + cpos);
	}
	cpos += pos[0];

	return cpos;
}



Var *
ff_load_bin5(vfuncptr func, Var *arg)
{
	B5H    *bh = NULL;
	Var   **structs = NULL;
	int    *pos = NULL, *dim = NULL;
	int     i;
	int     blk3d_items;
	char   *blk3d;
	int     n, m, carry;
	Var    *v = NULL;
	char   *fname = NULL;
	int     sidx, tidx;
	char   *data = NULL; /* mmap'ed data from the input file */
	int     fd;
	struct  stat sbuf;
	char    item_buff[8]; /* biggest item-size we are handling */
	int     item_bytes;


	int     ac;
	Var   **av;
	Alist   alist[2];

	/* make arguments list */
	alist[0] = make_alist("filename", ID_STRING, NULL, &fname);
	alist[1].name = NULL;

	/* process arguments */
	if (parse_args(func, arg, alist) == 0){ return NULL; }
	if (fname == NULL){
		parse_error("%s: filename not specified\n", func->name);
		return NULL;
	}


	/* get bin5 file header */
	if (!read_bin5_header(fname, &bh)){
		parse_error("%s: Unable to get header from file %s\n", func->name, fname);
		return NULL;
	}

	/* make sure that we can actually read this type of data */
	item_bytes = 0;
	if (bh->word_type >= 1 && bh->word_type <= 15){
		item_bytes = typeSizes[bh->word_type];
	}
	if (item_bytes <= 0){
		parse_error("%s: Unhandled word-type %d\n", func->name, bh->word_type);
		free_B5H(&bh);
		return NULL;
	}

	/* decode the architecture */

	if ((fd = open(fname, O_RDONLY)) < 0){
		parse_error("%s: Unable to open file %s\n", func->name, fname);
		return NULL;
	}
	if (fstat(fd, &sbuf) < 0){
		parse_error("%s: Unable to stat file %s\n", func->name, fname);
		free_B5H(&bh);
		close(fd);
		return NULL;
	}
	if ((data = (char *)mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == NULL){
		parse_error("%s: Unable to mmap file %s\n", func->name, fname);
		free_B5H(&bh);
		close(fd);
		return NULL;
	}


	/**
	 ** Make the data dimension at least three. The code
	 ** below depends upon it.
	 **/
	if ((n = bh->ndim) < 3){ n = 3; }
	dim = (int *)calloc(sizeof(int), n);
	for(i = 0; i < n; i++){ dim[i] = 1; }
	for(i = 0; i < bh->ndim; i++){ dim[i] = bh->dims[i]; }

	pos = (int *)calloc(sizeof(int), n);       /* element location n-dim */
	blk3d_items = dim[n-3]*dim[n-2]*dim[n-1];  /* size in items of last-3 dims */

	/**
	 ** Dimensions other than the last three go into a hierarchy
	 ** of structures. The following array is used to manage these
	 ** structures. When structs[m] is filled up according to dim[m]
	 ** it is added to a structs[m-1] and structs[m] reinitialized.
	 **/
	structs = (Var **)calloc(sizeof(Var *), n);

	do {
		/* we fill a data block from the last three dims */
		blk3d = (char *)calloc(item_bytes, blk3d_items);

		for(pos[n-3] = 0; pos[n-3] < dim[n-3]; pos[n-3]++){
			for(pos[n-2] = 0; pos[n-2] < dim[n-2]; pos[n-2]++){
				for(pos[n-1] = 0; pos[n-1] < dim[n-1]; pos[n-1]++){
					sidx = cpos_n(n,dim,pos);
					tidx = cpos_n(3,&dim[n-3],&pos[n-3]);
					memcpy(item_buff, data + bh->lbl_len + item_bytes*sidx, item_bytes);
					#ifdef WORDS_BIGENDIAN
					if (bh->arch_id == ARCH_X86){ byte_swap(item_buff, item_bytes); }
					#else
					if (bh->arch_id == ARCH_SUN){ byte_swap(item_buff, item_bytes); }
					#endif /* WORDS_BIGENDIAN */
					memcpy(&blk3d[item_bytes*tidx], item_buff, item_bytes);
				}
			}
		}
		v = newVal(BSQ, dim[n-3], dim[n-2], dim[n-1], internalTypes[bh->word_type], blk3d);

		/**
		 ** Update dimensions other than the last three by moving
		 ** back from n-4 to n-5 to n-6 ... to 0.
		 **/
		m = n-4;
		carry = 1;
		while(m >= 0 && carry){
			if (structs[m] == NULL){ structs[m] = create_struct(NULL); }
			add_struct(structs[m], NULL, v);

			carry = 0;
			if (++pos[m] >= dim[m]){
				/* this dimension is filled up -- move back one dim */
				v = structs[m]; structs[m] = NULL;
				pos[m] = 0; m--;
				carry = 1;
			}
		}
	} while(m >= 0);

	/**
	 ** The variable named "v" contains the actual data at
	 ** the end of this loop.
	 **/

	munmap(data, sbuf.st_size);
	free_B5H(&bh);
	return(v);
}

