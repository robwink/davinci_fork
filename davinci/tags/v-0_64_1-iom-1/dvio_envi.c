#include "parser.h"
#include "dvio.h"

Var *
dv_LoadENVI(FILE *fp, char *filename, struct iom_iheader *s)
{
  struct iom_iheader h;
  void *data;
  Var *v = NULL;
  char hbuf[HBUFSIZE];

  if (iom_isENVI(fp) == 0) return NULL;

  /* Reading ENVI header also reads in the data */
  if (iom_GetENVIHeader(fp, filename, &h) == 0){
    return NULL;
  }

  /**
   ** If user specified a record, subset out a specific band
   **/
  
  if (s != NULL) {
    /** 
     ** Set subsets
     **/
    iom_MergeHeaderAndSlice(&h, s);
  }

  data = iom_read_qube_data(fileno(fp), &h);

  if (data == NULL) { iom_cleanup_iheader(&h); return(NULL); }
    
  v = iom_iheader2var(&h);
  V_DATA(v) = data;

  sprintf(hbuf, "%s: ENVI %s image: %dx%dx%d, %s (%d bits)",
	  filename, iom_Org2Str(h.org),
	  iom_GetSamples(h.size, h.org), 
	  iom_GetLines(h.size, h.org), 
	  iom_GetBands(h.size, h.org), 
	  iom_Format2Str(h.format),
	  iom_NBYTESI(h.format)*8);

  if (VERBOSE > 1) { parse_error(hbuf); }
  
  iom_cleanup_iheader(&h);
  
  return(v);
}

