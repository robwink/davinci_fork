/*********************************** vicar.c **********************************/
#include "parser.h"
#include "io_vicar.h"


Var *
LoadAVIRIS(FILE *fp, char *filename, struct _iheader *s)
{
    struct _iheader h;
    int org;
    Var *v;
    void *data;
    char hbuf[HBUFSIZE];

    if (GetAVIRISHeader(fp, &h) == 0) {
        return(NULL);
    }

    if (s != NULL)  {
		int i, j;
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

    sprintf(hbuf, "%s: AVIRIS %s image: %dx%dx%d, %d bits",
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

    return(v);
}

/**
 ** GetAvirisHeader() - read and parse an aviris header
 **
 ** This routine returns 
 **         0 if the specified file is not a Vicar file.
 **         1 on success
 **/

int
GetAVIRISHeader(FILE *fp, struct _iheader *h)
{
    char *p, *q;
    int s = 0;
    int i;

    int org=-1, format=-1, label=0;
    int size[3], suffix[3];
	char buf[256];

    /**
     ** Read enough to get identifying label and total label size.
     ** Get total label size and mallocate enough space to hold it.
     **/

    rewind(fp);

	fgets(buf, 256, fp);
	if (buf[24] != '\n') return(0);

	/**
	 ** assume that this is really an AVIRIS file, in which case, decode
	 ** the buffer.
	 **/
	label = ((int *)buf)[0];
	format = ((int *)buf)[1];
	size[1] = ((int *)buf)[2];
	size[2] = ((int *)buf)[3];
	size[0] = ((int *)buf)[4];
	org = ((int *)buf)[5];
	suffix[0]  = suffix[1] = suffix[2] = 0;

	fgets(buf, 256, fp);
	if (strncmp(buf, "Label Size:",11)) {
		return(0);
	}

	/**
	 ** Right now, the only files we've seen indicate BIL=1.  
	 ** That matches davinci, so go with it.
	 **/
    if (org != 1) {
        fprintf(stderr, "io_aviris(): Unrecognized org.\n"
						"please mail this whole file to gorelick@asu.edu\n");
        return(0);
    }

	/**
	 ** Right now, the only files we've seen indicate SHORT=2.  
	 ** That matches davinci, so go with it.
	 **/
    if (format != 2) {
        fprintf(stderr, "io_aviris(): Unrecognized format.\n"
						"please mail this whole file to gorelick@asu.edu\n");
        return(0);
    }

    /**
     ** Put together a proper structure from the read values.
     **/

    memset(h, '\0', sizeof(struct _iheader));

    h->dptr = label;
    h->byte_order = 4321;       /* can this be reliably determined? */
    h->format = format;
    h->org = org;
    h->gain = 1.0;
    h->offset = 0.0;
    for (i = 0 ; i < 3 ; i++) {
        h->size[orders[org][i]] = size[i];
        h->suffix[orders[org][i]] = suffix[i];
    }

    return(1);
}


/**
 ** This (hopefully) detects AVIRIS files.
 ** returns:
 **             0: on failure
 **             1: on success
 **/
int
is_AVIRIS(FILE *fp)
{
    int len;
    char buf[256];

    rewind(fp);
	fgets(buf, 256, fp);
	if (buf[24] != '\n') return(0);

	fgets(buf, 256, fp);
	if (strcmp(buf, "Label Size:")) {
		return(0);
	}
}

