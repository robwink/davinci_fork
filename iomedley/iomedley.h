#ifndef _IOMEDLEY_H_
#define _IOMEDLEY_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "io_lablib3.h"

/*
** CAUTION:
** iom_EFORMAT2STR[] in iomedley.c depends upon these values
*/
typedef enum {
	iom_EDF_INVALID = 0, /* Invalid external-format. */

	iom_LSB_INT_1   = 1,
	iom_LSB_INT_2   = 2,
	iom_LSB_INT_4   = 4,

	iom_MSB_INT_1   = 11,
	iom_MSB_INT_2   = 12,
	iom_MSB_INT_4   = 14,

	iom_MSB_IEEE_REAL_4 = 24, /* SUN REAL */
	iom_MSB_IEEE_REAL_8 = 28, /* SUN DOUBLE */

	iom_LSB_IEEE_REAL_4 = 34, /* PC REAL */
	iom_LSB_IEEE_REAL_8 = 38, /* PC DOUBLE */
    
#ifdef WORDS_BIGENDIAN

	iom_NATIVE_INT_1 = iom_MSB_INT_1,
	iom_NATIVE_INT_2 = iom_MSB_INT_2,
	iom_NATIVE_INT_4 = iom_MSB_INT_4,

	iom_NATIVE_IEEE_REAL_4 = iom_MSB_IEEE_REAL_4,
	iom_NATIVE_IEEE_REAL_8 = iom_MSB_IEEE_REAL_8,

#else /* little-endian */

	iom_NATIVE_INT_1 = iom_LSB_INT_1,
	iom_NATIVE_INT_2 = iom_LSB_INT_2,
	iom_NATIVE_INT_4 = iom_LSB_INT_4,

	iom_NATIVE_IEEE_REAL_4 = iom_LSB_IEEE_REAL_4,
	iom_NATIVE_IEEE_REAL_8 = iom_LSB_IEEE_REAL_8,

#endif /* WORDS_BIGENDIAN */

	iom_VAX_INT     = 42,
	iom_VAX_REAL_4  = 44,
	iom_VAX_REAL_8  = 48
} iom_edf;             /* daVinci I/O external data formats */

#define iom_NBYTES(ef) ((ef) > 40 ? ((ef) - 40) : ((ef) > 30 ? ((ef) - 30) : ((ef) > 20 ? ((ef) - 20) : ((ef) > 10 ? ((ef) - 10) : (ef)))))


/*
** CAUTION:
** iom_FORMAT2STR[] in iomedley.c depends upon these values
*/
typedef enum {
	iom_BYTE   = 1,
	iom_SHORT  = 2,
	iom_INT    = 3,
	iom_FLOAT  = 4,
	iom_DOUBLE = 5
} iom_idf;             /* daVinci I/O internal data formats */

#define iom_NBYTESI(ifmt) ((ifmt) == 5 ? 8 : ((ifmt) == 3 ? 4 : (ifmt)))

/**
 ** Data axis order
 ** Var->value.Sym->order
 **
 ** !!! CAUTION: these values must be 0 based.  They are used as array
 **              indices below.
 **/
 
/*
** CAUTION:
** iom_ORG2STR[] in iomedley.c depends upon these values
*/
typedef enum {
	iom_BSQ = 0,
	iom_BIL = 1,
	iom_BIP = 2
} iom_order;

#define iom_EFormat2Str(i)  iom_EFORMAT2STR[(i)]
#define iom_Format2Str(i)   iom_FORMAT2STR[(i)]
#define iom_Org2Str(i)      iom_ORG2STR[(i)]

#define iom_GetSamples(s,org)   (s)[((org) == iom_BIP ? 1 : 0)]
#define iom_GetLines(s,org)     (s)[((org) == iom_BSQ ? 1 : 2)]
#define iom_GetBands(s,org)     (s)[((org) == iom_BIP ? 0 : ((org) == iom_BIL ? 1 : 2))]

extern int iom_orders[3][3];
extern const char *iom_EFORMAT2STR[];
extern const char *iom_FORMAT2STR[];
extern const char *iom_ORG2STR[];
/* extern int iom_VERBOSE; */



struct iom_iheader {
	size_t dptr;        /* offset in bytes to first data value     */
	int prefix[3];      /* size of prefix data (bytes)             */
	int suffix[3];      /* size of suffix data (bytes)             */

	int size[3];        /* dimension of file data (pixels) (org-order)  */

	/* Sub-select relavant.                                        */
	int s_lo[3];        /* subset lower range (pixels)             */
	int s_hi[3];        /* subset upper range (pixels)             */
	int s_skip[3];      /* subset skip interval (pixels)           */

	/* Set by read_qube_data() once the data read is successful.   */
	/* It is derived from sub-selects.                             */
	int dim[3];         /* sub-selected (or final dimension) (pixels) (org-order) */

	size_t corner;      /* size of 1 whole plane */

	int byte_order;     /* byteorder of data - don't use                 */

	iom_edf eformat;    /* extrnal format of data                  */
                    	/*   -- comes from iom_edf enum above      */
                    	/* this is what the file says it has       */

	int format;         /* data format (INT, FLOAT, etc)           */
	/*   -- comes from iom_idf enum above    */
                    	/* this is what read_qube_data() returns   */

	int transposed;     /* IMath data is transposed                */

	int org;            /* data organization                       */

	float gain, offset; /* data multiplier and additive offset     */

	unsigned char *data;         /* non-NULL if all of the image is loaded  */

	char *ddfname;      /* detached data-file name (if any)        */
                    	/* see io_isis.c                           */
};




/* Stolen from vanilla/header.h */

typedef char *iom_cptr;
void iom_swp(iom_cptr pc1, iom_cptr pc2);

// NOTE(gorelick): These macros used to return the transformed value.
// They no longer do that.  If you assign this function-looking thing
// to a value, you're (probably) only going to get the last byte.

#ifdef WORDS_BIGENDIAN

#define iom_MSB8(s) 	
#define iom_MSB4(s) 	
#define iom_MSB2(s) 	

#define iom_LSB8(s) 	(iom_swp(&(((iom_cptr)(s))[0]), &(((iom_cptr)(s))[7])), \
                         iom_swp(&(((iom_cptr)(s))[1]), &(((iom_cptr)(s))[6])), \
                         iom_swp(&(((iom_cptr)(s))[2]), &(((iom_cptr)(s))[5])), \
                         iom_swp(&(((iom_cptr)(s))[3]), &(((iom_cptr)(s))[4])))

#define iom_LSB4(s) 	(iom_swp(&(((iom_cptr)(s))[0]), &(((iom_cptr)(s))[3])), \
                         iom_swp(&(((iom_cptr)(s))[1]), &(((iom_cptr)(s))[2])))

#define iom_LSB2(s) 	(iom_swp(&(((iom_cptr)(s))[0]), &(((iom_cptr)(s))[1])))

#else /* little endian */

#define iom_MSB8(s) 	(iom_swp(&(((iom_cptr)(s))[0]), &(((iom_cptr)(s))[7])), \
                         iom_swp(&(((iom_cptr)(s))[1]), &(((iom_cptr)(s))[6])), \
                         iom_swp(&(((iom_cptr)(s))[2]), &(((iom_cptr)(s))[5])), \
                         iom_swp(&(((iom_cptr)(s))[3]), &(((iom_cptr)(s))[4])))

#define iom_MSB4(s) 	(iom_swp(&(((iom_cptr)(s))[0]), &(((iom_cptr)(s))[3])), \
                         iom_swp(&(((iom_cptr)(s))[1]), &(((iom_cptr)(s))[2])))

#define iom_MSB2(s) 	(iom_swp(&(((iom_cptr)(s))[0]), &(((iom_cptr)(s))[1])))

#define iom_LSB8(s) 	
#define iom_LSB4(s) 	
#define iom_LSB2(s) 	

#endif /* WORDS_BIGENDIAN */



/*
** Returns data size in number of items. Item size depending upon
** the internal data format.
**
** Byte-size can be calculated by multiplying the returned
** value with iom_NBYTESI(h->format).
**
*/
size_t iom_iheaderDataSize(struct iom_iheader *h);

/*
** Returns the number of bytes per data item for the internal
** data format stored in the iom_iheader "h".
*/
int iom_iheaderItemBytesI(struct iom_iheader *h);


int iom_is_compressed(FILE * fp);
FILE *iom_uncompress(FILE * fp, const char *fname);
char *iom_uncompress_with_name(const char *fname);

int iom_isVicar(FILE *fp);
int iom_isIMath(FILE *fp);
int iom_isISIS(FILE *fp);
int iom_isGRD(FILE *fp);
int iom_isGOES(FILE *fp);
int iom_isAVARIS(FILE *fp);
int iom_isPNM(FILE *fp);
int iom_isENVI(FILE *fp);
int iom_isBMP(FILE *);
int iom_isGIF(FILE *);
int iom_isJPEG(FILE *);
int iom_isTIFF(FILE *);
int iom_isPNG(FILE *);


/*
** GetXXXXHeader() require that the input file is already
** opened and its file pointer passed to it. The file-name
** parameter is for information purposes only.
**
** GetISISHeader() can be passed r_obj as NULL.
**
** read_qube_data() is passed a (potentially subsetted)
** _iheader structure obtained from GetXXXXHeader().
**
** A pre-loaded header can be subsetted by issuing a
** MergeHeaderAndSlice() on it and the subset-defining
** _iheader structure.
*/

int iom_GetVicarHeader(FILE *fp, char *fname, struct iom_iheader *h);
int iom_GetIMathHeader(FILE *fp, char *fname, struct iom_iheader *h);
int iom_GetISISHeader(FILE *fp, char *fname, struct iom_iheader *h, char *msg_file, OBJDESC **r_obj);
int iom_GetGRDHeader(FILE *fp, char *fname, struct iom_iheader *h);
int iom_GetGOESHeader(FILE *fp, char *fname, struct iom_iheader *h);
int iom_GetAVIRISHeader(FILE *fp, char *fname, struct iom_iheader *h);
int iom_GetENVIHeader(FILE *fp, char *fnmae, struct iom_iheader *h);

int iom_GetBMPHeader(FILE *, char *, struct iom_iheader *);
int iom_GetGIFHeader(FILE *, char *, struct iom_iheader *);
int iom_GetJPEGHeader(FILE *, char *, struct iom_iheader *);
int iom_GetTIFFHeader(FILE *, char *, struct iom_iheader *);
int iom_GetPNMHeader(FILE *, char *, struct iom_iheader *);
int iom_GetPNGHeader(FILE *, char *, struct iom_iheader *);

/*
** The following functions do not support reading data cubes
** from them.
** Data for these files is loaded from the file while loading
** the header. It is available through the _iheader.data field.
** This data block is allocated using malloc(). It is the
** caller's responsiblity to free it when it is no longer in
** use any more by issuing cleanup_iheader().
**
** Data from these files can still be read by using
** read_qube_data() which knows how to read from data attached
** to _iheader.data.
**
*/


/*
** Unified header loader.
**
** Loads iom_iheader by reading the file header. Data items loaded
** include the size and external-format of data. A loaded iom_header can be
** used to read data from the file. The actual read is done via
** read_qube_data(). If a subsection of the raster qube is required
** one must call iom_MergeHeaderAndSlice() before calling
** read_qube_data().
*/
int iom_LoadHeader(FILE *fp, char *fname, struct iom_iheader *header);


/*
** WriteXXXX() take an already opened output file's pointer.
** The file name is for informational purposes only.
**
** These routines take their image dimensions from iom_iheader.size[3].
** User iom_var2iheader() to construct a proper iom_iheader for the
** specified variable.
**
** These routines byte-swap the data as per the specified external
** format and the current machine's endian-inclination.
*/

int iom_WriteIMath(char *fname, void *data, struct iom_iheader *h, int force_write);
int iom_WriteERS(char *fname, void *data, struct iom_iheader *h, int force_write);
int iom_WriteVicar(char *filename, void *data, struct iom_iheader *h, int force_write);
int iom_WriteISIS(char *fname, void *data, struct iom_iheader *h, int force_write, char *title);
int iom_WriteGRD(char *fname, void *data, struct iom_iheader *h, int force_write, char *title, char *pgm);
int iom_WriteRaw(char *fname, void *data, struct iom_iheader *h, int force_write);
int iom_WriteJPEG(char *fname, unsigned char *data, struct iom_iheader *h, int force_write);
int iom_WriteGIF(char *fname, unsigned char *data, struct iom_iheader *h, int force_write);
int iom_WriteTIFF(char *fname, unsigned char *data, struct iom_iheader *h, int force_write);
int iom_WriteBMP(char *fname, unsigned char *data, struct iom_iheader *h, int force_write);
int iom_WritePNG(char *fname, unsigned char *data, struct iom_iheader *h, int force_write);
int iom_WritePNM(char *fname, unsigned char *data, struct iom_iheader *h, int force_write);


/*
** Support Functions
*/

void iom_init_iheader(struct iom_iheader *h);



/*
** SetSliceInHeader() / MergeHeaderAndSlice()
**
** Merges sub-selection (subsetting/slicing) info
** into the header. So that a succeeding read_qube_data()
** returns the subset data only.
**
** The slice is passed in as an iom_iheader with the following
** fields set appropriately: s_lo, s_hi, and s_skip .
**
** When the iom_iheader is initially loaded from a raster qube
** file, a call to read_qube_data() would result in the entire
** raster to be returned. This function is used to sub-select
** and return a smaller block of data. The sub-selection
** information is attached to the iom_iheader loaded from the
** file (this is due to historic reasons). The slice dimension
** indices are 1-based (i.e. first element is 1 as compared to
** C-arrays which start at 0).
**
** The slice dimensions are specified in bsq. However, they get
** stored in "h" in org-order.
*/
void iom_MergeHeaderAndSlice(struct iom_iheader *h, struct iom_iheader *s);
void iom_SetSliceInHeader(struct iom_iheader *h, struct iom_iheader *s);

/*
** ClearSliceInHeader()
**
** Clears the slice section of the header thus the next
** read_qube_data() will return the entire image.
*/
void iom_ClearSliceInHeader(struct iom_iheader *h);


/*
** detach_iheader_data()
**
** Remove the data associated with the _iheader structure (if any)
** and return it. Once the memory resident data is detached one
** must not call read_qube_data(). One can still call
** iom_cleanup_iheader() on the resulting iom_iheader.
*/
void *iom_detach_iheader_data(struct iom_iheader *h);



/*
** cleanup_iheader()
**
** Cleans the memory allocated within the _iheader structure.
*/
void iom_cleanup_iheader(struct iom_iheader *h);

/*
** Prints the image header in the specified "stream" file.
** Note that "fname" is the name associated with the "header"
** and not the "stream."
*/
void iom_PrintImageHeader(FILE *stream, char *fname, struct iom_iheader *header);

/**
 ** read_qube_data() - generalized cube reader
 **
 ** In order to read data from a raster/qube, one must have the
 ** data file's header loaded already. This is done using the
 ** iom_LoadHeader(). Such a header will cause read_qube_data()
 ** to read the entire raster/qube.
 ** 
 ** If one desires to read a portion/slice/sub-selection of the
 ** data instead, one must call iom_SetSliceInHeader() before
 ** calling read_qube_data().
 **
 ** Data is returned as a malloc'ed memory block which must be
 ** freed by the caller. It is in the org-order and not in bsq order.
 **
 ** The iom_iheader can be reused by setting a different slice
 ** or by clearing the slice altogether.
 **
 ** When the user is done with an iom_iheader, it must be disposed
 ** off properly by calling iom_cleanup_iheader().
 **/
void *iom_read_qube_data(int fd, struct iom_iheader *h);

void *iom_ReadImageSlice(FILE *fp, char *fname, struct iom_iheader *slice);

/*
** Byte-swap the data stored in "data" of size "dsize" with
** external format "eformat" and return the resulting internal
** format of the data (as one of the values from iom_idf).
*/
int iom_byte_swap_data(
	char     *data,    /* data to be modified/adjusted/swapped */
	size_t    dsize,   /* number of data elements */
	iom_edf  eformat   /* external format of the data */
    );



/**
 ** Try to expand environment variables and ~
 ** puts answer back into argument.  Make sure its big enough...
 **/
char *iom_expand_filename(const char *s);


/*----------------------------------------------------------------------
  VAX real to IEEE real format conversion from VICAR RTL.
  ----------------------------------------------------------------------*/
void iom_vax_ieee_r(float *from, float *to);

/*----------------------------------------------------------------------
  IEEE real to VAX real format conversion from VICAR RTL.
  ----------------------------------------------------------------------*/
void iom_ieee_vax_r(int *from, float *to);


void iom_long_byte_swap(unsigned char from[4], unsigned char to[4]);


int iom_Eformat2Iformat(iom_edf efmt);


/*
** THE FOLLOWING FUNCTIONS SHOULD BE MOVED TO THEIR OWN IO_XXXX.H
** FILES ONCE THE MERGE WITH IOMEDLEY IS DONE.
*/

/*
** Message verbosity control in iomedley.
*/
extern int iom_VERBOSITY;             /* default = 5 i.e. errors/warnings & progress only */

int iom_is_ok2print_sys_errors();     /* verbosity > 0 */
int iom_is_ok2print_unsupp_errors();  /* verbosity > 1 */
int iom_is_ok2print_errors();         /* verbosity > 2 */
int iom_is_ok2print_warnings();       /* verbosity > 3 */
int iom_is_ok2print_progress();       /* verbosity > 4 */
int iom_is_ok2print_details();        /* verbosity > 9 */

/* Organizational conversion function, defined in iomedley.c.
   Jim Stewart - 26 Jun 2002 */

int iom__ConvertToBIP(unsigned char *,       /* Image data */
		      struct iom_iheader *,  /* Image geometry */
		      unsigned char **       /* BIP image data output */
		      );

size_t iom_Cpos(int x, int y, int z, int org, int size[3]);
void iom_Xpos(size_t i, int org, int size[3], int *x, int *y, int *z);

int iom_isAVIRIS(FILE *);
int iom_WriteENVI(char *, void *, struct iom_iheader *, int);
iom_edf iom_ConvertISISType(char *, char *, char *);
iom_edf iomConvertISISType(char *, int);

#endif /* _IOMEDLEY_H_ */
