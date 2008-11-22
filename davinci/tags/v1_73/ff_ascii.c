#include "parser.h"

int 
dv_getline(char **ptr, FILE *fp)
{
  static char *line=NULL;
  static int len=0;
  
  if (fp == NULL && line != NULL) {
    free(line);
    line = NULL;
    return 0;
  }
  
  if (line == NULL) {
    len = 8192;
    line = (char *)malloc(len+1);
  }
  
  if (fgets(line, len, fp) == NULL) {
    *ptr = NULL;
    return(-1);
  }
  while (strchr(line, '\n') == NULL) {
    line = (char *)my_realloc(line, len*2+1);
    if ((fgets(line+len-1, len, fp)) == NULL) break;
    len = len*2-1;
    if (len > 1000000) {
      fprintf(stderr, "Line is at 1000000\n");
      exit(1);
    }
  }
  *ptr = line;
  return(strlen(line));
}

Var *
ff_ascii(vfuncptr func, Var *arg)
{
  char    *filename = NULL;
  Var     *v = NULL, *e = NULL, *s = NULL;
  char    *fname = NULL;
  FILE    *fp = NULL;
  char    *ptr = NULL;
  int      rlen;
  void    *data = NULL;
  unsigned char *cdata = NULL;
  short   *sdata = NULL;
  int     *idata = NULL;
  float   *fdata = NULL;
  double  *ddata = NULL;
  int      count=0;
  char    *delim = " \t";
  int      i,j,k;
  int      dsize;
  int      x=0;
  int      y=0;
  int      z=0;
  int      format=0;
  int      column=0;
  int      row=0;
  char    *formats[] = { "byte", "short", "int", "float", "double", NULL };
  char    *format_str = NULL;

  Alist alist[9];
  alist[0] = make_alist( "filename",    ID_STRING,   NULL,    &filename);
  alist[1] = make_alist( "x",    	INT,         NULL,    &x);
  alist[2] = make_alist( "y",    	INT,         NULL,    &y);
  alist[3] = make_alist( "z",    	INT,         NULL,    &z);
  alist[4] = make_alist( "format",      ID_ENUM,     formats, &format_str);
  alist[5] = make_alist( "column",      INT,         NULL,    &column);
  alist[6] = make_alist( "row",    	INT,         NULL,    &row);
  alist[7] = make_alist( "delim",       ID_STRING,   NULL,    &delim);
  alist[8].name = NULL;
  
  if (parse_args(func, arg, alist) == 0) return(NULL);
  
  if (filename == NULL) {
    parse_error("No filename specified: %s()", func->name);
    return(NULL);
  }

  if (format_str != NULL) {
    if (!strcasecmp(format_str, "byte")) format = BYTE;
    else if (!strcasecmp(format_str, "short")) format = SHORT;
    else if (!strcasecmp(format_str, "int")) format = INT;
    else if (!strcasecmp(format_str, "float")) format = FLOAT;
    else if (!strcasecmp(format_str, "double")) format = DOUBLE;
  }

  /**
   ** Got all the values.  Do something with 'em.
   **/

  if ((fname = dv_locate_file(filename)) == NULL) {
    sprintf(error_buf, "Cannot find file: %s\n", filename);
    parse_error(NULL);
    return(NULL);
  }
  if ((fp = fopen(fname, "r")) == NULL) {
    sprintf(error_buf, "Cannot open file: %s\n", fname);
    parse_error(NULL);
    return(NULL);
  }
  if (x == 0 && y == 0 && z == 0) {
    /**
     ** User wants us to determine the file size.
     **/
    z = 1;
    
    /* skip some rows */
    for (i = 0 ; i < row ; i ++) {
      dv_getline(&ptr, fp);
      if (ptr == NULL) {
	fprintf(stderr, "Early EOF, aborting.\n");
	return(NULL);
      }
    }
    
    while(dv_getline(&ptr, fp) != EOF) {
      if (ptr[0] == '\n') {
	z++;
	continue;
      }
      
      while ((ptr = strtok(ptr, delim)) != NULL)  {
	count++;
	ptr = NULL;
      }
      y++;
    }
    
    if (z == 0) z = 1;
    if (y == 0) y = 1;
    y = y/z;
    x = count/y/z;
    
    if (x*y*z != count) {
      fprintf(stderr, "Unable to determine file size.\n");
      return(NULL);
    }
    if (VERBOSE) fprintf(stderr, "Apparent file size: %dx%dx%d\n", x,y,z);
    rewind(fp);
  }

  /**
   ** Decode file with specified X, Y, Z.
   **/
  if (x == 0) x = 1;
  if (y == 0) y = 1;
  if (z == 0) z = 1;
  if (format == 0) format = INT;

  dsize = x*y*z;
  data = calloc(NBYTES(format), dsize);
  
  cdata = (unsigned char *)data;
  sdata = (short *)data;
  idata = (int *)data;
  fdata = (float *)data;
  ddata = (double *)data;
  
  count = 0;
  

  /**
   ** Skip N rows.
   **/
  for (i = 0 ; i < row ; i ++) {
    dv_getline(&ptr, fp);
    if (ptr == NULL) {
      fprintf(stderr, "Early EOF, aborting.\n");
      return(NULL);
    }
  }
  for (k = 0 ; k < z ; k++) {
    if (k) {
      /**
       ** skip to end of block
       **/
      while (dv_getline(&ptr, fp) > 1)
	;       
    }
    
    for (j = 0 ; j < y ; j++) {
      if ((rlen = dv_getline(&ptr, fp)) == -1) break;
      
      /**
       ** skip columns
       **/
      
      for (i = 0 ; i < column ; i++) {
	ptr = strtok(ptr, delim);
	ptr = NULL;
      }
      /**
       ** read X values from this line
       **/

      for (i = 0 ; i < x ; i++) {
	ptr = strtok(ptr, delim);
	if (ptr == NULL) {
	  fprintf(stderr, "Line too short\n");
	  count += x-i;
	  break;
	}
	switch (format) {
	case BYTE:
	  cdata[count++] = saturate_byte(strtol(ptr, NULL, 10));
	  break;
	case SHORT:
	  sdata[count++] = saturate_short(strtol(ptr, NULL, 10));
	  break;
	case INT:
	  idata[count++] = saturate_int(strtol(ptr, NULL, 10));
	  break;
	case FLOAT:
	  fdata[count++] = strtod(ptr, NULL);
	  break;
	case DOUBLE:
	  ddata[count++] = strtod(ptr, NULL);
	  break;
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
    fprintf(stderr, "Read ASCII file: %dx%dx%d\n", x,y,z);
  }

  fclose(fp);

  s = newVar();
  V_TYPE(s) = ID_VAL;
  V_DATA(s) = data;
  V_FORMAT(s) = format;
  V_ORG(s) = BSQ;
  V_DSIZE(s) = dsize;
  V_SIZE(s)[0] = x;
  V_SIZE(s)[1] = y;
  V_SIZE(s)[2] = z;
  
  return(s);
}
