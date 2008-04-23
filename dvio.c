/*
** File: dvio.c
** Related files: dvio.h dvio_*.c
**
** Bridging code between daVinci and iomedley
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "parser.h"
#include "iomedley.h"

#if 0
int
_iheader2iom_iheader(
    struct _iheader *h,
    struct iom_iheader *iomh
    )
{
    memset(iomh, 0, sizeof(struct iom_iheader));
    iomh->dptr = h->dptr;
    memcpy(iomh->prefix, h->prefix, sizeof(int)*3);
    memcpy(iomh->suffix, h->suffix, sizeof(int)*3);
    memcpy(iomh->size, h->size, sizeof(int)*3);
    memcpy(iomh->s_lo, h->s_lo, sizeof(int)*3);
    memcpy(iomh->s_hi, h->s_hi, sizeof(int)*3);
    memcpy(iomh->s_skip, h->s_skip, sizeof(int)*3);
    memcpy(iomh->dim, h->dim, sizeof(int)*3);
    iomh->byte_order = h->byte_order;
    iomh->corner = h->corner;
    iomh->format = vfmt2ihfmt(h->format);

	switch(iomh->format){
    case iom_BYTE:   iomh->eformat = iom_NATIVE_INT_1;        break;
    case iom_SHORT:  iomh->eformat = iom_NATIVE_INT_2;        break;
    case iom_INT:    iomh->eformat = iom_NATIVE_INT_4;        break;
    case iom_FLOAT:  iomh->eformat = iom_NATIVE_IEEE_REAL_4;  break;
    case iom_DOUBLE: iomh->eformat = iom_NATIVE_IEEE_REAL_8;  break;
	default:
		iomh->eformat = iom_EDF_INVALID;
		fprintf(stderr, "Error! Internal format %d not recognized.\n",
			iomh->format);
		break;
	}

    iomh->transposed = 0;
    iomh->org = vorg2ihorg(h->org);
    iomh->gain = h->gain;
    iomh->offset = h->offset;
    iomh->data = NULL;
    iomh->ddfname = NULL;
    return 1;
}

int
iom_iheader2_iheader(
    struct iom_iheader *iomh,
    struct _iheader *h
    )
{
    memset(h, 0, sizeof(struct _iheader));
    h->dptr = iomh->dptr;
    memcpy(h->prefix, iomh->prefix, sizeof(int)*3);
    memcpy(h->suffix, iomh->suffix, sizeof(int)*3);
    memcpy(h->size, iomh->size, sizeof(int)*3);
    memcpy(h->s_lo, iomh->s_lo, sizeof(int)*3);
    memcpy(h->s_hi, iomh->s_hi, sizeof(int)*3);
    memcpy(h->s_skip, iomh->s_skip, sizeof(int)*3);
    memcpy(h->dim, iomh->dim, sizeof(int)*3);
    h->byte_order = iomh->byte_order;
    h->corner = iomh->corner;
    h->format = ihfmt2vfmt(iomh->format);
    h->org = ihorg2vorg(h->org);
    h->gain = iomh->gain;
    h->offset = iomh->offset;
    return 1;
}
#endif /* 0 */

/*
** iom_iheader.org <-> V_ORG(v)
*/
int
ihorg2vorg(int org)
{
    int vorder = -1;
    
    switch(org){
    case iom_BSQ: vorder = BSQ; break;
    case iom_BIL: vorder = BIL; break;
    case iom_BIP: vorder = BIP; break;
    }

    return vorder;
}

int
vorg2ihorg(int vorder)
{
    int org = -1;

    switch(vorder){
    case BSQ: org = iom_BSQ; break;
    case BIL: org = iom_BIL; break;
    case BIP: org = iom_BIP; break;
    }

    return org;
}

/*
** iom_iheader.format <-> V_FORMAT(v)
*/

int
ihfmt2vfmt(int ifmt)
{
    int vfmt = -1;

    switch(ifmt){
    case iom_BYTE:   vfmt = BYTE;   break;
    case iom_SHORT:  vfmt = SHORT;  break;
    case iom_INT:    vfmt = INT;    break;
    case iom_FLOAT:  vfmt = FLOAT;  break;
    case iom_DOUBLE: vfmt = DOUBLE; break;
        /* VAX_INT & VAX_FLOAT are deprecated */
    }

    return vfmt;
}

int
vfmt2ihfmt(int vfmt)
{
    int ifmt = -1;

    switch(vfmt){
    case BYTE:   ifmt = iom_BYTE;   break;
    case SHORT:  ifmt = iom_SHORT;  break;
    case INT:    ifmt = iom_INT;    break;
    case FLOAT:  ifmt = iom_FLOAT;  break;
    case DOUBLE: ifmt = iom_DOUBLE; break;
    }

    return ifmt;
}

/*
** iheader2var()
**
** Complementry function of var2iheader()
**
** Converts an _iheader structure into a daVinci "Var."
** It works for very simple applications. The only fields
** transferred across are:
**    org     (as V_ORDER)
**    dim     (as V_SIZE & V_DSIZE)
**    format  (as V_FORMAT)
*/

Var *
iom_iheader2var(struct iom_iheader *h)
{
    Var *v;
    int i;

    v = newVar();

    V_TYPE(v) = ID_VAL;
    V_FORMAT(v) = ihfmt2vfmt(h->format);
    
    /** Will not be possible now
	if (V_FORMAT(v) == VAX_FLOAT) V_FORMAT(v) = FLOAT;
	if (V_FORMAT(v) == VAX_INTEGER) V_FORMAT(v) = INT;
    */

	if (h->dim[0] < 0 || h->dim[1] < 0 || h->dim[2] < 0){
		fprintf(stderr, "One of dim[i]'s is not set properly.\n");
		fprintf(stderr, "See File: %s  Line: %d.\n", __FILE__, __LINE__);
	}
    
    V_ORDER(v) = ihorg2vorg(h->org);
    V_DSIZE(v) = 1;
    for (i = 0 ; i < 3 ; i++) {
        V_SIZE(v)[i] = (((h->dim)[i] -1)/h->s_skip[i])+1;
        V_DSIZE(v) *= V_SIZE(v)[i];
    }

    return(v);
}



/*
** var2iheader()
**
** Complementry function of iheader2var()
**
** A very simple "Var" to "struct _iheader" converter.
** It can be used for converting variable data cube
** parameters into an _iheader structure. Note that
** this is a very limited implementation and only a
** handfull of fields are filled within the _iheader
** structure. These fields are:
**    org    (from V_ORDER)
**    size   (from V_SIZE)       <------- NOTE THE DIFFERENCE
**    format (from V_FORMAT)
*/

void
var2iom_iheader(
    Var *v,
    struct iom_iheader *h
    )
{
    int i;
    
    iom_init_iheader(h);
    h->org = vorg2ihorg(V_ORDER(v));

    for (i = 0; i < 3; i++){
        h->size[i] = V_SIZE(v)[i];
    }

    h->format = vfmt2ihfmt(V_FORMAT(v));
}


/*
** locate_file() - do filename expansion
**
** On successful return a character string allocated using
** malloc() will be returned. It is the caller's responsibility
** to free it after use.
*/

char *
dv_locate_file(char *fname)
{
	char buf[2048];

	strcpy(buf, fname);
	fname = buf;

//Check if it is a remote file. Download it and make it local
#ifdef HAVE_LIBCURL		
	//Update the fname only if the load was successfull
	fname = try_remote_load(fname);
#endif
	fname = iom_expand_filename(fname);

	if (fname) 
		return(strdup(fname));
	else 
		return(NULL);
}


void
dv_set_iom_verbosity()
{
	if (VERBOSE == 1){ iom_VERBOSITY = 5; }
	else if (VERBOSE >= 2){ iom_VERBOSITY = 10; }
	else { iom_VERBOSITY = VERBOSE; }
}
