#ifndef _DVIO_H_
#define _DVIO_H_

#include "parser.h"
#include "iomedley.h"


/*
** iheader2var()
**
** Complementry function of var2iheader()
**
** Converts an _iheader structure into a daVinci "Var."
** It works for very simple applications. The only fields
** transferred across are:
**    org     (as V_ORDER)
**    format  (as V_FORMAT)
**    dim     (as V_SIZE & V_DSIZE)
*/

Var *iom_iheader2var(struct iom_iheader *h);


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
**    dim    (from V_SIZE)
**    format (from V_FORMAT)
*/

void iom_var2iheader(Var *v, struct iom_iheader *h);


int ihorg2vorg(int org);
int vorg2ihorg(int vorder);
int ihfmt2vfmt(int ifmt);
int vfmt2ihfmt(int vfmt);
int _iheader2iom_iheader(struct _iheader *h, struct iom_iheader *iomh);
int iom_iheader2_iheader(struct iom_iheader *iomh, struct _iheader *h);


Var *dv_LoadAVIRIS(FILE *fp, char *filename, struct iom_iheader *s);
Var *dv_LoadGOES(FILE *fp, char *filename, struct iom_iheader *s);
Var *dv_LoadGRD(FILE *fp, char *filename, struct iom_iheader *s);
Var *dv_LoadIMath(FILE *fp, char *filename, struct iom_iheader *s);
Var *dv_LoadISIS(FILE *fp, char *filename, struct iom_iheader *s);
Var *dv_LoadGFX_Image(char *filename);
Var *dv_LoadPNM(FILE *fp, char *filename, struct iom_iheader *s);
Var *dv_LoadVicar(FILE *fp, char *filename, struct iom_iheader *s);

int dv_WriteGRD(Var *s, FILE *fp, char *filename);
int dv_WriteISIS(Var *s, FILE *fp, char *filename, char *title);
int dv_WriteGFX_Image(Var *ob, char *filename, char *GFX_type);
int dv_WritePGM(Var *obj, FILE *fp, char *filename);
int dv_WritePPM(Var *obj, FILE *fp, char *filename);


char *iom_locate_file(char *fname);

#endif /* _DVIO_H_ */
