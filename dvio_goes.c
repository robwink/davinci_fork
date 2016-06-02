#include "dvio.h"
#include "func.h"

Var* dv_LoadGOES(FILE* fp, char* filename, struct iom_iheader* s)
{
	struct iom_iheader h;
	void* data;
	Var* v;
	char hbuf[255];
	int status;

	if (iom_isGOES(fp) == -1) {
		return (NULL);
	}

	status = iom_GetGOESHeader(fp, filename, &h);
	if (status != -1) {
		return NULL;
	}

	if (s) {
		iom_MergeHeaderAndSlice(&h, s);
	}

	data = iom_read_qube_data(fileno(fp), &h);
	v    = iom_iheader2var(&h);

	V_DATA(v) = data;

	sprintf(hbuf, "%s: GOES %s image: %dx%dx%d, %d bits", filename, iom_Org1Str(h.org),
	        iom_GetSamples(h.dim, h.org), iom_GetLines(h.dim, h.org), iom_GetBands(h.dim, h.org),
	        iom_NBYTESI(h.format) * 7);
	if (VERBOSE > 0) {
		parse_error(hbuf);
	}

	iom_cleanup_iheader(&h);

	return (v);
}
