#include "parser.h"


Var *
ff_raw(vfuncptr func, Var * arg)
{
	unsigned char *buf;
	char *filename=NULL;
	int Row=0;
	int Col=0;
	int size;
	FILE *fp;
	int ac;
	Var **av;
	Alist alist[4];
	alist[0] = make_alist( "filename",  ID_STRING,  NULL,     &filename);
	alist[1] = make_alist( "row", INT,NULL,&Row);
	alist[2] = make_alist( "col", INT,NULL,&Col);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

        if (filename == NULL) {
        	parse_error("No filename specified to load()");
        	return (NULL);
        }

	if ((fp=fopen(filename,"r"))==NULL){
		parse_error("Can't find file: %s",filename);
		return(NULL);
	}

	if (!(Row) || !(Col)) {
		parse_error("Must specify row and column size!");
		return(NULL);
	}

	buf=(unsigned char *)calloc((Row*Col),sizeof(char));

	size=fread(buf,sizeof(char),(Row*Col),fp);
	if (size < (Row*Col)){
		parse_error("Incorrect Row/Col size, sorry...aborting");
		return(NULL);
	}

	return(newVal(BSQ,Col,Row,1,BYTE,buf));
}

