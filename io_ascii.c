#include "parser.h"
/**
 ** This is a simple ASCII output function
 **/

int
WriteAscii(Var *s, FILE *fp, char *filename)
{
    int dsize;
    int format;
    int ival;
    double dval;
    int d[3];
    int i;

    if (fp == NULL) {
        if ((fp = fopen(filename, "w")) == NULL) {
            return 0;
        }
    }

    dsize = V_DSIZE(s);
    format = V_FORMAT(s);
    d[0] = V_SIZE(s)[0];
    d[1] = V_SIZE(s)[1] * d[0];

    for (i = 0 ; i < V_DSIZE(s) ; i++) {
	switch (format) {
	  case BYTE:
	  case SHORT:
	  case INT:
	    ival = extract_int(s, i);
	    fprintf(fp, "%d", ival);
	    break;
	  case FLOAT:
	  case DOUBLE:
	    dval = extract_double(s, i);
	    fprintf(fp, "%.10g", dval);
	    break;
	}
	if (((i+1) % d[0]) == 0) {
	    fputc('\n', fp);
	} else {
	    fputc('\t', fp);
	}
	if (((i+1) % d[1]) == 0 && (i+1) != dsize) {
	    fputc('\n', fp);
	}
    }
    fclose(fp);
    return(1);
}
