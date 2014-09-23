#include "parser.h"
#include "dvio.h"
#include "iomedley.h"


Var *
dv_LoadVicar(
    FILE *fp,
    char *filename,
    struct iom_iheader *s
    )
{
    struct iom_iheader h;
    Var *v;
    void *data;
    char hbuf[HBUFSIZE];

    if (iom_isVicar(fp) == 0){ return NULL; }
    
    if (iom_GetVicarHeader(fp, filename, &h) == 0) { return(NULL); }

    /**
     ** If user specified a record, subset out a specific band
     **/

    if (s != NULL)  {
        /** 
         ** Set subsets.  The s struct is assumed to be in BSQ order
         **/
        iom_MergeHeaderAndSlice(&h, s);
    }

    /**
     ** Put all this into a Var struct.
     **/
    
    data = iom_read_qube_data(fileno(fp), &h);
    v = iom_iheader2var(&h);

    V_DATA(v) = data;

    sprintf(hbuf, "%s: VICAR %s image: %dx%dx%d, %d bits",
            filename, iom_Org2Str(h.org),
            iom_GetSamples(h.dim, h.org), 
            iom_GetLines(h.dim, h.org), 
            iom_GetBands(h.dim, h.org), 
            iom_NBYTESI(h.format)*8);
    if (VERBOSE > 1) {
        parse_error(hbuf);
    }
    iom_cleanup_iheader(&h);

    return(v);
}

int
dv_WriteVicar(Var *ob, char *filename, int force)
{
    struct iom_iheader h;
    int status;

    var2iom_iheader(ob, &h);

	if (VERBOSE > 1){
        fprintf(stderr, "Writing %s: %dx%dx%d VICAR file.\n",
                filename, h.size[0], h.size[1], h.size[2]);
	}

    status = iom_WriteVicar(filename, V_DATA(ob), &h, force);
    iom_cleanup_iheader(&h);

    if (status == 0){
        parse_error("Writing VICAR file %s failed.\n", filename);
        return 0;
    }
    
    return  1;
}


/**
 ** Check for a specific element in the header.
 **/

int
dv_LoadVicarHeader(FILE *fp, char *filename, int rec, char *element, Var **var)
{
	Var *v= NULL;
    char *p, *q, *str;
    int s = 0;

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
				v = newVar();
				V_TYPE(v) = ID_VAL;
				V_DSIZE(v) = V_SIZE(v)[0] = V_SIZE(v)[1] = V_SIZE(v)[2] = 1;
				V_ORG(v) = BSQ;
				V_FORMAT(v) = INT;
				V_DATA(v) = calloc(1, sizeof(int));
				V_INT(v) = atoi(q);
		} else {
			v = newVar();
			V_TYPE(v) = ID_STRING;
			V_STRING(v) = strdup(q);
		}
	}
	free(p);
	*var = v;
	return(1);
}
