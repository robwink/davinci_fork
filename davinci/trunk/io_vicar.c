/*********************************** vicar.c **********************************/
#include "parser.h"
#include "io_vicar.h"
#include <pwd.h>

/**
 ** Vicar I/O routines
 **
 ** is_Vicar()  - detect VICAR magic cookie 
 ** LoadVicar() - Load VICAR data file
 **/

/**
 ** This detects the magic cookie for vicar files.
 ** returns:
 **             0: on failure
 **             1: on success
 **/
int
is_Vicar(FILE *fp)
{
    int len;
    char buf[16];

    rewind(fp);
    len = fread(buf, 1, strlen(VICAR_MAGIC), fp);
    return (len == strlen(VICAR_MAGIC) && !strncmp(buf, VICAR_MAGIC, len));
}

char *
get_value(char *s1, char *s2)
{
    char *p;
    int len;

    len = strlen(s2);
    for (p = s1 ; p && *p ; p++) {
        if (!strncasecmp(p, s2, len)) {
            return(p+len);
        }
    }
    return(NULL);
}

/** GetVicarHeader() - read and parse a vicar header
 **
 ** This routine returns 
 **         0 if the specified file is not a Vicar file.
 **         1 on success
 **/

int
GetVicarHeader(FILE *fp, struct _iheader *h)
{
    char *p, *q;
    int s = 0;
    int i;

    int org=-1, format=-1;
    int size[3], suffix[3], prefix[3];
	int r;

    /**
     ** Read enough to get identifying label and total label size.
     ** Get total label size and mallocate enough space to hold it.
     **/

    rewind(fp);

    p = (char *)malloc(65);
    fread(p, 1, 64, fp);
    p[64] = '\0';
    sscanf(p, "LBLSIZE=%d", &s);
    if (s <= 0) {
        free(p);
        return(0);
    }

    p = (char *)my_realloc(p, s+1);

    /**
     ** Read entire label, and parse it.
     **/

    fread(p+64, 1, s-64, fp);
    p[s] = '\0';

	r = atoi(get_value(p, "RECSIZE="));

    size[0] =   atoi(get_value(p, "NS=")); /* width */
    size[1] =   atoi(get_value(p, "NL=")); /* height */
    size[2] =   atoi(get_value(p, "NB=")); /* depth */

	suffix[0] = suffix[1] = suffix[2] = 0;
	prefix[0] = prefix[1] = prefix[2] = 0;

    s += atoi(get_value(p, "NLB="))*r;
    prefix[0] = atoi(get_value(p, "NBB="));

    if ((q = get_value(p, "ORG=")) != NULL) {
        if (!strncmp(q, "'BIL'", 5)) org = BIL;
        if (!strncmp(q, "'BSQ'", 5)) org = BSQ;
        if (!strncmp(q, "'BIP'", 5)) org = BIP;
    }
    if (org == -1) {
        parse_error( "load(): File has no org.");
        free(p);
        return(0);
    }

    if ((q = get_value(p, "FORMAT=")) != NULL) {
        if (!strncmp(q, "'BYTE'", 6)) format = BYTE;
        else if (!strncmp(q, "'HALF'", 6)) format = SHORT;
        else if (!strncmp(q, "'WORD'", 6)) format = SHORT;
        else if (!strncmp(q, "'FULL'", 6)) format = INT;
        else if (!strncmp(q, "'REAL'", 6)) format = FLOAT;
		else format = -1;
    }
    if (format == -1) {
        parse_error( "load(): File has no format");
        free(p);
        return(0);
    }

    /**
     ** Put together a proper structure from the read values.
     **/

    memset(h, '\0', sizeof(struct _iheader));

    h->dptr = s;
    h->byte_order = 4321;       /* can this be reliably determined? */
    h->format = format;
    h->org = org;
    h->gain = 1.0;
    h->offset = 0.0;
    for (i = 0 ; i < 3 ; i++) {
        h->size[orders[org][i]] = size[i];
        h->suffix[orders[org][i]] = suffix[i];
        h->prefix[orders[org][i]] = prefix[i];
    }

    free(p);
    return(1);
}

/**
 ** LoadVicar() - Load VICAR data file into Var struct
 **     FILE *fp                - opened file pointer to data file
 **     char *filename  - name of data file
 **     int   rec               - band to extract (0 means whole file)
 **/

Var *
LoadVicar(FILE *fp, char *filename, struct _iheader *s)
{
    struct _iheader h;
    int org;
    Var *v;
    void *data;
    char hbuf[HBUFSIZE];
	int i, j;

    if (GetVicarHeader(fp, &h) == 0) {
        return(NULL);
    }

    /**
     ** If user specified a record, subset out a specific band
     **/

    if (s != NULL)  {
        /** 
         ** Set subsets.  The s struct is assumed to be in BSQ order
         **/
        org = h.org;

		for (i = 0 ; i < 3 ; i++) {
			j = orders[org][i];
			if (s->s_lo[j] > 0) h.s_lo[i] = s->s_lo[j];
			if (s->s_hi[j] > 0) h.s_hi[i] = s->s_hi[j];
			if (s->s_skip[j] > 0) h.s_skip[i] = s->s_skip[j];
			if (s->prefix[j] > 0) h.prefix[i] = s->prefix[j];
			if (s->suffix[j] > 0) h.suffix[i] = s->suffix[j];
		}
    }

    /**
     ** Put all this into a Var struct.
     **/
        
    data = read_qube_data(fileno(fp), &h);
    v = iheader2var(&h);

    V_DATA(v) = data;

    sprintf(hbuf, "%s: VICAR %s image: %dx%dx%d, %d bits",
            filename, Org2Str(h.org),
            GetSamples(h.dim, h.org), 
            GetLines(h.dim, h.org), 
            GetBands(h.dim, h.org), 
            NBYTES(h.format)*8);
    if (VERBOSE > 1) { 
        fprintf(stderr, hbuf);
        fprintf(stderr, "\n");
        fflush(stderr);
    }
    V_TITLE(v) = strdup(hbuf);

    return(v);
}


Var *
iheader2var(struct _iheader *h)
{
    Var *v;
    int i;

    v = new(Var);

    V_TYPE(v) = ID_VAL;
    V_FORMAT(v) = h->format;
    V_ORDER(v) = h->org;
    V_DSIZE(v) = 1;
    for (i = 0 ; i < 3 ; i++) {
        V_SIZE(v)[i] = (((h->dim)[i] -1)/h->s_skip[i])+1;
        V_DSIZE(v) *= V_SIZE(v)[i];
    }
    return(v);
}


/**
 ** WriteVicar() - Write data in VICAR format
 **/

/*

LBLSIZE=340             FORMAT='BYTE'  TYPE='IMAGE'  BUFSIZ=24576  DIM=3  EOL=0  RECSIZE=10  ORG='BSQ'  NL=10  NS=10  NB=11  N1=10  N2=10  N3=11  N4=0  NBB=0  NLB=0  HOST='SUN-SOLR'  INTFMT='HIGH'  REALFMT='IEEE'  BHOST='VAX-VMS'  BINTFMT='LOW'  BREALFMT='VAX'  BLTYPE=''  TASK='LABEL'  USER='gorelick'  DAT_TIM='Tue Apr 20 16:13:57 1999'

*/

int
WriteVicar(Var *s, FILE *fp, char *filename)
{
    char ptr[4096];     
    int rec;
    int bands;
    int org;
    int dim;
    int len;


    memset(ptr, ' ', 4096);

    bands = GetBands(V_SIZE(s), V_ORG(s));
    org = V_ORG(s);
    dim = 3;

    /**
     ** If no depth, force BSQ org.
     **/

    if (bands == 1) {
        org = BSQ;
        dim = 2;
    }

    rec = V_SIZE(s)[0] * NBYTES(V_FORMAT(s));


    switch(V_FORMAT(s)) {
      case BYTE:        sprintf(ptr, "FORMAT='BYTE'  "); break;
      case SHORT: sprintf(ptr, "FORMAT='HALF'  "); break;
      case INT: sprintf(ptr, "FORMAT='FULL'  "); break;
      case FLOAT: sprintf(ptr, "FORMAT='REAL'  "); break;
      case DOUBLE:
        sprintf(error_buf,"Cannot write DOUBLE to VICAR file.");
        parse_error(NULL);
        return 0;
        break;
    }
    sprintf(ptr+strlen(ptr),
            "TYPE='IMAGE'  BUFSIZ=%d  DIM=%d  EOL=0  RECSIZE=%d  ",
            24576, dim, rec);


    switch(V_ORG(s)) {
      case BIL: sprintf(ptr+strlen(ptr), "ORG='BIL'  "); break;
      case BIP: sprintf(ptr+strlen(ptr), "ORG='BIP'  "); break;
      case BSQ: sprintf(ptr+strlen(ptr), "ORG='BSQ'  "); break;
    }

    sprintf(ptr+strlen(ptr), "NL=%d  NS=%d  NB=%d  ",
            GetLines(V_SIZE(s), V_ORG(s)),
            GetSamples(V_SIZE(s), V_ORG(s)),
            GetBands(V_SIZE(s), V_ORG(s)));

    if (org == BSQ) {           /* done cause we may have forced bsq */
        sprintf(ptr+strlen(ptr), "N1=%d  N2=%d  N3=%d  ",
                GetLines(V_SIZE(s), V_ORG(s)),
                GetSamples(V_SIZE(s), V_ORG(s)),
                GetBands(V_SIZE(s), V_ORG(s)));
    } else {
        sprintf(ptr+strlen(ptr), "N1=%d  N2=%d  N3=%d  ",
                V_SIZE(s)[0],
                V_SIZE(s)[1],
                V_SIZE(s)[2]);
    }

    sprintf(ptr+strlen(ptr), "N4=0  NBB=0  NLB=0  ");
    
	{
		time_t t = time(0);
		sprintf(ptr+strlen(ptr),
            "HOST='SUN-SOLR'  INTFMT='HIGH'  REALFMT='IEEE'  BHOST='VAX-VMS'  BINTFMT='LOW'  BREALFMT='VAX'  BLTYPE=''  TASK='DAVINCI'  USER='%s'  DAT_TIM='%24.24s'", getpwuid(getuid())->pw_name, ctime(&t));

	}
    /**
     ** compute final label size and stuff it in at the front.
     **/
    len = (((strlen(ptr) + 24) / rec)+1) * rec;

    if (VERBOSE > 1) {
        fprintf(stderr, "Writing %s: VICAR %s %dx%dx%d %d bit IMAGE\n",
                filename,
                Org2Str(org), 
                GetSamples(V_SIZE(s), V_ORG(s)),
                GetLines(V_SIZE(s), V_ORG(s)),
                GetBands(V_SIZE(s), V_ORG(s)),
                NBYTES(V_FORMAT(s)) * 8);
        fflush(stderr);
    }

    if (fp == NULL) {
        if ((fp = fopen(filename, "w")) == NULL) {
			return(0);
		}
    }

    fprintf(fp, "LBLSIZE=%-5d           ",len);
    fwrite(ptr, strlen(ptr), 1, fp);
    fprintf(fp, "%*s", len-strlen(ptr)-24-2, " ");
	{
		int i = 0;
		fwrite(&i, 1, 2, fp);
	}
    /**
     ** write data
     **/
    fwrite(V_DATA(s), NBYTES(V_FORMAT(s)), V_DSIZE(s), fp);
    fclose(fp);

    return(1);
}

/**
 ** Check for a specific element in the header.
 **/

int
LoadVicarHeader(FILE *fp, char *filename, int rec, char *element, Var **var)
{
	Var *v= NULL;
    char *p, *q, *str;
    int s = 0;
    int i;

    rewind(fp);

    p = (char *)malloc(65);
    fread(p, 1, 64, fp);
    p[64] = '\0';
    sscanf(p, "LBLSIZE=%d", &s);

    if (s <= 0) {
        free(p);
        return(0);
    }

    p = (char *)my_realloc(p, s+1);

    /**
     ** Read entire label, and parse it.
     **/

    fread(p+64, 1, s-64, fp);
    p[s] = '\0';

	if ((q = get_value(p, element)) != NULL) {
		while(isspace(*q) || *q == '=') q++;
		if (*q == '\'' || *q == '"') {
			/**
			 ** find matching quote
			 **/
			for (str = q+1 ; *str && *str != *q ; str++) ;
			*str = '\0';
			q++;		/* skip over starting quote */
		} else {
			str = q;
			while(!isspace(*str)) str++;
			*str = '\0';
		}

		/**
		 ** convert those we know to be ints
		 **/
		if (!strcasecmp(element, "lblsize") ||
		    !strcasecmp(element, "bufsize") ||
		    !strcasecmp(element, "dim") ||
		    !strcasecmp(element, "recsize") ||
		    !strcasecmp(element, "nl") ||
		    !strcasecmp(element, "ns") ||
		    !strcasecmp(element, "nb") ||
		    !strcasecmp(element, "n1") ||
		    !strcasecmp(element, "n2") ||
		    !strcasecmp(element, "n3") ||
		    !strcasecmp(element, "n4") ||
		    !strcasecmp(element, "nbb") ||
		    !strcasecmp(element, "nlb"))  {
				v = new(Var);
				V_TYPE(v) = ID_VAL;
				V_DSIZE(v) = V_SIZE(v)[0] = V_SIZE(v)[1] = V_SIZE(v)[2] = 1;
				V_ORG(v) = BSQ;
				V_FORMAT(v) = INT;
				V_DATA(v) = calloc(1, sizeof(int));
				V_INT(v) = atoi(q);
		} else {
			v = new(Var);
			V_TYPE(v) = ID_STRING;
			V_STRING(v) = strdup(q);
		}
	}
	free(p);
	*var = v;
	return(1);
}
