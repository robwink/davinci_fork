#include "parser.h"
#include "io_goes.h"

int
is_GOES(FILE *fp)
{
    int len;
    char buf[16];

	/**
	 ** GOES_MAGIC is 8 characters, mostly 0's
	 **/
 
    rewind(fp);
    len = fread(buf, 1, 8, fp);
    return (len == 8 && !memcmp(buf, GOES_MAGIC, 8));
}

Var *
LoadGOES(FILE *fp, char *filename, struct _iheader *s)
{
	struct goes_area g;
	struct _iheader h;
	void *data;
	Var *v;
	char hbuf[256];

	if (is_GOES(fp) == 0) {
		return(NULL);
	}

	rewind(fp);
	fread(&g, sizeof(g), 1, fp);

	memset(&h, 0, sizeof(h));
	h.org = BSQ;
	h.size[0] = g.samples;
	h.size[1] = g.lines;
	h.size[2] = 1;

	if (g.size == 1) h.format = BYTE;
	if (g.size == 2) h.format = SHORT;
	if (g.size == 4) h.format = INT;

	h.prefix[1] = h.prefix[2] = 0;
	h.prefix[0] = g.presiz;

	h.suffix[0] = h.suffix[1] = h.suffix[2];
	h.dptr = g.data_offset;

    data = read_qube_data(fileno(fp), &h);
    v = iheader2var(&h);

	V_DATA(v) = data;

    sprintf(hbuf, "%s: GOES %s image: %dx%dx%d, %d bits",
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
