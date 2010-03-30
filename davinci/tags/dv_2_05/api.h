#include "parser.h"
#include "apidef.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_PLPLOT
/* Prototypes; sorted in ascending order */
void pl_pladv(int page);
void pl_plaxes(float x0,float y0,char *xopt,float xtick,int nxsub,char *yopt,float ytick,int nysub);
void pl_plbin(int nbin,float *x,float *y,int center);
void pl_plbop();
void pl_plbox(char *xopt,float xtick,int nxsub,char *yopt,float ytick,int nysub);
void pl_plbox3(char *xopt,char *xlabel,float xtick,int nsubx,char *yopt,char *ylabel,float ytick,int nsuby,char *zopt,char *zlabel,float ztick,int nsubz);
void pl_plcol0(int icol0);
void pl_plcol1(float col1);
void pl_plcpstrm(int iplsr,int flags);
void pl_plcross(float *x, float *y);
void pl_plend();
void pl_plend1();
void pl_plenv(float xmin,float xmax,float ymin,float ymax,int just,int axis);
void pl_pleop();
void pl_plerrx(int n,float *xmin,float *xmax,float *y);
void pl_plerry(int n,float *x,float *ymin,float *ymax);
void pl_plfamadv();
void pl_plfill(int n,float *x,float *y);
void pl_plflush();
void pl_plfont(int ifont);
void pl_plfontld(int fnt);
void pl_plgchr(float *p_def,float *p_ht);
void pl_plgcol0(int icol0,int *r,int *g,int *b);
void pl_plgcolbg(int *r,int *g,int *b);
void pl_plgdev(char *p_dev);
void pl_plgdidev(float *p_mar,float *p_aspect,float *p_jx,float *p_jy);
void pl_plgdiori(float *p_rot);
void pl_plgdiplt(float *p_xmin,float *p_ymin,float *p_xmax,float *p_ymax);
void pl_plgfam(int *p_fam,int *p_num,int *p_bmax);
void pl_plgfnam(char *fnam);
void pl_plglevel(int *p_level);
void pl_plgpage(float *p_xp,float *p_yp,int *p_xleng,int *p_yleng,int *p_xoff,int *p_yoff);
void pl_plgra();
void pl_plgspa(float *xmin,float *xmax,float *ymin,float *ymax);
void pl_plgstrm(int *p_strm);
void pl_plgver(char *p_ver);
void pl_plgwid(int *width);
void pl_plgxax(int *p_digmax,int *p_digits);
void pl_plgyax(int *p_digmax,int *p_digits);
void pl_plgzax(int *p_digmax,int *p_digits);
void pl_plhist(int n,float *data,float datmin,float datmax,int nbin,int oldwin);
void pl_plhls(float h,float l,float s);
void pl_plinit();
void pl_pljoin(float x1,float y1,float x2,float y2);
void pl_pllab(char *xlabel,char *ylabel,char *tlabel);
void pl_plline(int n,float *x,float *y);
void pl_plline3(int n,float *x,float *y,float *z);
void pl_pllsty(int lin);
void pl_plmkstrm(int *p_strm);
void pl_plmtex(char *side,float disp,float pos,float just,char *text);
void pl_plpat(int nlin,int *inc,int *del);
void pl_plpoin(int n,float *x,float *y,int code);
void pl_plpoin3(int n,float *x,float *y,float *z,int code);
void pl_plpoly3(int n,float *x,float *y,float *z,int *draw);
void pl_plprec(int setp,int prec);
void pl_plpsty(int patt);
void pl_plptex(float x,float y,float dx,float dy,float just,char *text);
void pl_plptexd(float x,float y,float dx,float dy,float just,char *text);
void pl_plreplot();
void pl_plrgb(float r,float g,float b);
void pl_plrgb1(int r,int g,int b);
void pl_plschr(float def,float scale);
void pl_plscmap0(int *r,int *g,int *b,int ncol0);
void pl_plscmap0n(int ncol0);
void pl_plscmap1(int *r,int *g,int *b,int ncol1);
void pl_plscmap1l(int itype,int npts,float *intensity,float *coord1,float *coord2,float *coord3,int *rev);
void pl_plscmap1n(int ncol1);
void pl_plscol0(int icol0,int r,int g,int b);
void pl_plscolbg(int r,int g,int b);
void pl_plscolor(int color);
void pl_plsdev(char *devname);
void pl_plsdidev(float mar,float aspect,float jx,float jy);
void pl_plsdimap(int dimxmin,int dimxmax,int dimymin,int dimymax,float dimxpmm,float dimypmm);
void pl_plsdiori(float rot);
void pl_plsdiplt(float xmin,float ymin,float xmax,float ymax);
void pl_plsdiplz(float xmin,float ymin,float xmax,float ymax);
void pl_plsesc(char esc);
void pl_plsfam(int fam,int num,int bmax);
void pl_plsfnam(char *fnam);
void pl_plsmaj(float def,float scale);
void pl_plsmin(float def,float scale);
void pl_plsori(int ori);
void pl_plspage(float xp,float yp,int xleng,int yleng,int xoff,int yoff);
void pl_plspause(int pause);
void pl_plsplw(char *title);
void pl_plsstrm(int strm);
void pl_plssub(int nx,int ny);
void pl_plssym(float def,float scale);
void pl_plstar(int nx,int ny);
void pl_plstart(char *devname,int nx,int ny);
void pl_plstyl(int nms,int *mark,int *space);
void pl_plsvpa(float xmin,float xmax,float ymin,float ymax);
void pl_plsxax(int digmax,int digits);
void pl_plsyax(int digmax,int digits);
void pl_plsym(int n,float *x,float *y,int code);
void pl_plszax(int digmax,int digits);
void pl_pltext();
void pl_plvasp(float aspect);
void pl_plvpas(float xmin,float xmax,float ymin,float ymax,float aspect);
void pl_plvpor(float xmin,float xmax,float ymin,float ymax);
void pl_plvsta();
void pl_plw3d(float basex,float basey,float height,float xmin0,float xmax0,float ymin0,float ymax0,float zmin0,float zmax0,float alt,float az);
void pl_plwid(int width);
void pl_plwind(float xmin,float xmax,float ymin,float ymax);

void plClearOpts();
char *plFindCommand(char *fn);
int plFindName(char *p);
float plGetFlt(char *s);
int plGetInt(char *s);
void plHLS_RGB(float h,float l,float s,float *p_r,float *p_g,float *p_b);
void plOptUsage();
void plRGB_HLS(float r,float g,float b,float *p_h,float *p_l,float *p_s);
void plResetOpts();
int plSetOpt(char *opt,char *optarg);
void plSetUsage(char *program_string,char *usage_string);
void pldid2pc(float *xmin,float *ymin,float *xmax,float *ymax);
void pldip2dc(float *xmin,float *ymin,float *xmax,float *ymax);
float plf2eval(int ix,int iy,void *plf2eval_data);
float plf2eval2(int ix,int iy,void *plf2eval_data);
float plf2evalr(int ix,int iy,void *plf2eval_data);
void plgesc(char *p_esc);
void plsError(int *errcode,char *errmsg);
void plsxwin(int window_id);
/* Wrappers: */
APIARGS pl_pladv_args[]={
	{ "pl_pladv",VOID },
	{ "page",INT } };
void
pl_pladv_wrapper(int ac,APIARGS *av){
	pl_pladv(*((int *)av[1].argval));	/* int page */

}

APIARGS pl_plaxes_args[]={
	{ "pl_plaxes",VOID },
	{ "x0",FLOAT },
	{ "y0",FLOAT },
	{ "xopt",CONSTBIT | BYTE  | PTRBIT },
	{ "xtick",FLOAT },
	{ "nxsub",INT },
	{ "yopt",CONSTBIT | BYTE  | PTRBIT },
	{ "ytick",FLOAT },
	{ "nysub",INT } };
void
pl_plaxes_wrapper(int ac,APIARGS *av){
	pl_plaxes(*((float *)av[1].argval),	/* float x0 */
		*((float *)av[2].argval),	/* float y0 */
		(char *)av[3].argval,	/* char *xopt */
		*((float *)av[4].argval),	/* float xtick */
		*((int *)av[5].argval),	/* int nxsub */
		(char *)av[6].argval,	/* char *yopt */
		*((float *)av[7].argval),	/* float ytick */
		*((int *)av[8].argval));	/* int nysub */

}

APIARGS pl_plbin_args[]={
	{ "pl_plbin",VOID },
	{ "nbin",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "center",INT } };
void
pl_plbin_wrapper(int ac,APIARGS *av){
	pl_plbin(*((int *)av[1].argval),	/* int nbin */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		*((int *)av[4].argval));	/* int center */

}

APIARGS pl_plbop_args[]={
	{ "pl_plbop",VOID } };
void
pl_plbop_wrapper(int ac,APIARGS *av){
	pl_plbop();
}

APIARGS pl_plbox_args[]={
	{ "pl_plbox",VOID },
	{ "xopt",CONSTBIT | BYTE  | PTRBIT },
	{ "xtick",FLOAT },
	{ "nxsub",INT },
	{ "yopt",CONSTBIT | BYTE  | PTRBIT },
	{ "ytick",FLOAT },
	{ "nysub",INT } };
void
pl_plbox_wrapper(int ac,APIARGS *av){
	pl_plbox((char *)av[1].argval,	/* char *xopt */
		*((float *)av[2].argval),	/* float xtick */
		*((int *)av[3].argval),	/* int nxsub */
		(char *)av[4].argval,	/* char *yopt */
		*((float *)av[5].argval),	/* float ytick */
		*((int *)av[6].argval));	/* int nysub */

}

APIARGS pl_plbox3_args[]={
	{ "pl_plbox3",VOID },
	{ "xopt",CONSTBIT | BYTE  | PTRBIT },
	{ "xlabel",CONSTBIT | BYTE  | PTRBIT },
	{ "xtick",FLOAT },
	{ "nsubx",INT },
	{ "yopt",CONSTBIT | BYTE  | PTRBIT },
	{ "ylabel",CONSTBIT | BYTE  | PTRBIT },
	{ "ytick",FLOAT },
	{ "nsuby",INT },
	{ "zopt",CONSTBIT | BYTE  | PTRBIT },
	{ "zlabel",CONSTBIT | BYTE  | PTRBIT },
	{ "ztick",FLOAT },
	{ "nsubz",INT } };
void
pl_plbox3_wrapper(int ac,APIARGS *av){
	pl_plbox3((char *)av[1].argval,	/* char *xopt */
		(char *)av[2].argval,	/* char *xlabel */
		*((float *)av[3].argval),	/* float xtick */
		*((int *)av[4].argval),	/* int nsubx */
		(char *)av[5].argval,	/* char *yopt */
		(char *)av[6].argval,	/* char *ylabel */
		*((float *)av[7].argval),	/* float ytick */
		*((int *)av[8].argval),	/* int nsuby */
		(char *)av[9].argval,	/* char *zopt */
		(char *)av[10].argval,	/* char *zlabel */
		*((float *)av[11].argval),	/* float ztick */
		*((int *)av[12].argval));	/* int nsubz */

}

APIARGS pl_plcol0_args[]={
	{ "pl_plcol0",VOID },
	{ "icol0",INT } };
void
pl_plcol0_wrapper(int ac,APIARGS *av){
	pl_plcol0(*((int *)av[1].argval));	/* int icol0 */

}

APIARGS pl_plcol1_args[]={
	{ "pl_plcol1",VOID },
	{ "col1",FLOAT } };
void
pl_plcol1_wrapper(int ac,APIARGS *av){
	pl_plcol1(*((float *)av[1].argval));	/* float col1 */

}

APIARGS pl_plcpstrm_args[]={
	{ "pl_plcpstrm",VOID },
	{ "iplsr",INT },
	{ "flags",INT } };
void
pl_plcpstrm_wrapper(int ac,APIARGS *av){
	pl_plcpstrm(*((int *)av[1].argval),	/* int iplsr */
		*((int *)av[2].argval));	/* int flags */

}


APIARGS pl_plcross_args[]={
	{ "pl_plcross",VOID },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT }};
void
pl_plcross_wrapper(int ac,APIARGS *av){
	pl_plcross((float *)av[1].argval,	/* float *x */
		(float *)av[2].argval);	/* float *y */

}


APIARGS pl_plend_args[]={
	{ "pl_plend",VOID } };
void
pl_plend_wrapper(int ac,APIARGS *av){
	pl_plend();
}

APIARGS pl_plend1_args[]={
	{ "pl_plend1",VOID } };
void
pl_plend1_wrapper(int ac,APIARGS *av){
	pl_plend1();
}

APIARGS pl_plenv_args[]={
	{ "pl_plenv",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT },
	{ "just",INT },
	{ "axis",INT } };
void
pl_plenv_wrapper(int ac,APIARGS *av){
	pl_plenv(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float xmax */
		*((float *)av[3].argval),	/* float ymin */
		*((float *)av[4].argval),	/* float ymax */
		*((int *)av[5].argval),	/* int just */
		*((int *)av[6].argval));	/* int axis */

}

APIARGS pl_pleop_args[]={
	{ "pl_pleop",VOID } };
void
pl_pleop_wrapper(int ac,APIARGS *av){
	pl_pleop();
}

APIARGS pl_plerrx_args[]={
	{ "pl_plerrx",VOID },
	{ "n",INT },
	{ "xmin",FLOAT  | PTRBIT },
	{ "xmax",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT } };
void
pl_plerrx_wrapper(int ac,APIARGS *av){
	pl_plerrx(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *xmin */
		(float *)av[3].argval,	/* float *xmax */
		(float *)av[4].argval);	/* float *y */

}

APIARGS pl_plerry_args[]={
	{ "pl_plerry",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "ymin",FLOAT  | PTRBIT },
	{ "ymax",FLOAT  | PTRBIT } };
void
pl_plerry_wrapper(int ac,APIARGS *av){
	pl_plerry(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *ymin */
		(float *)av[4].argval);	/* float *ymax */

}

APIARGS pl_plfamadv_args[]={
	{ "pl_plfamadv",VOID } };
void
pl_plfamadv_wrapper(int ac,APIARGS *av){
	pl_plfamadv();
}

APIARGS pl_plfill_args[]={
	{ "pl_plfill",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT } };
void
pl_plfill_wrapper(int ac,APIARGS *av){
	pl_plfill(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval);	/* float *y */

}

APIARGS pl_plflush_args[]={
	{ "pl_plflush",VOID } };
void
pl_plflush_wrapper(int ac,APIARGS *av){
	pl_plflush();
}

APIARGS pl_plfont_args[]={
	{ "pl_plfont",VOID },
	{ "ifont",INT } };
void
pl_plfont_wrapper(int ac,APIARGS *av){
	pl_plfont(*((int *)av[1].argval));	/* int ifont */

}

APIARGS pl_plfontld_args[]={
	{ "pl_plfontld",VOID },
	{ "fnt",INT } };
void
pl_plfontld_wrapper(int ac,APIARGS *av){
	pl_plfontld(*((int *)av[1].argval));	/* int fnt */

}

APIARGS pl_plgchr_args[]={
	{ "pl_plgchr",VOID },
	{ "p_def",FLOAT  | PTRBIT },
	{ "p_ht",FLOAT  | PTRBIT } };
void
pl_plgchr_wrapper(int ac,APIARGS *av){
	pl_plgchr((float *)av[1].argval,	/* float *p_def */
		(float *)av[2].argval);	/* float *p_ht */

}

APIARGS pl_plgcol0_args[]={
	{ "pl_plgcol0",VOID },
	{ "icol0",INT },
	{ "r",INT  | PTRBIT },
	{ "g",INT  | PTRBIT },
	{ "b",INT  | PTRBIT } };
void
pl_plgcol0_wrapper(int ac,APIARGS *av){
	pl_plgcol0(*((int *)av[1].argval),	/* int icol0 */
		(int *)av[2].argval,	/* int *r */
		(int *)av[3].argval,	/* int *g */
		(int *)av[4].argval);	/* int *b */

}

APIARGS pl_plgcolbg_args[]={
	{ "pl_plgcolbg",VOID },
	{ "r",INT  | PTRBIT },
	{ "g",INT  | PTRBIT },
	{ "b",INT  | PTRBIT } };
void
pl_plgcolbg_wrapper(int ac,APIARGS *av){
	pl_plgcolbg((int *)av[1].argval,	/* int *r */
		(int *)av[2].argval,	/* int *g */
		(int *)av[3].argval);	/* int *b */

}

APIARGS pl_plgdev_args[]={
	{ "pl_plgdev",VOID },
	{ "p_dev",BYTE  | PTRBIT } };
void
pl_plgdev_wrapper(int ac,APIARGS *av){
	pl_plgdev((char *)av[1].argval);	/* char *p_dev */

}

APIARGS pl_plgdidev_args[]={
	{ "pl_plgdidev",VOID },
	{ "p_mar",FLOAT  | PTRBIT },
	{ "p_aspect",FLOAT  | PTRBIT },
	{ "p_jx",FLOAT  | PTRBIT },
	{ "p_jy",FLOAT  | PTRBIT } };
void
pl_plgdidev_wrapper(int ac,APIARGS *av){
	pl_plgdidev((float *)av[1].argval,	/* float *p_mar */
		(float *)av[2].argval,	/* float *p_aspect */
		(float *)av[3].argval,	/* float *p_jx */
		(float *)av[4].argval);	/* float *p_jy */

}

APIARGS pl_plgdiori_args[]={
	{ "pl_plgdiori",VOID },
	{ "p_rot",FLOAT  | PTRBIT } };
void
pl_plgdiori_wrapper(int ac,APIARGS *av){
	pl_plgdiori((float *)av[1].argval);	/* float *p_rot */

}

APIARGS pl_plgdiplt_args[]={
	{ "pl_plgdiplt",VOID },
	{ "p_xmin",FLOAT  | PTRBIT },
	{ "p_ymin",FLOAT  | PTRBIT },
	{ "p_xmax",FLOAT  | PTRBIT },
	{ "p_ymax",FLOAT  | PTRBIT } };
void
pl_plgdiplt_wrapper(int ac,APIARGS *av){
	pl_plgdiplt((float *)av[1].argval,	/* float *p_xmin */
		(float *)av[2].argval,	/* float *p_ymin */
		(float *)av[3].argval,	/* float *p_xmax */
		(float *)av[4].argval);	/* float *p_ymax */

}

APIARGS pl_plgfam_args[]={
	{ "pl_plgfam",VOID },
	{ "p_fam",INT  | PTRBIT },
	{ "p_num",INT  | PTRBIT },
	{ "p_bmax",INT  | PTRBIT } };
void
pl_plgfam_wrapper(int ac,APIARGS *av){
	pl_plgfam((int *)av[1].argval,	/* int *p_fam */
		(int *)av[2].argval,	/* int *p_num */
		(int *)av[3].argval);	/* int *p_bmax */

}

APIARGS pl_plgfnam_args[]={
	{ "pl_plgfnam",VOID },
	{ "fnam",BYTE  | PTRBIT } };
void
pl_plgfnam_wrapper(int ac,APIARGS *av){
	pl_plgfnam((char *)av[1].argval);	/* char *fnam */

}

APIARGS pl_plglevel_args[]={
	{ "pl_plglevel",VOID },
	{ "p_level",INT  | PTRBIT } };
void
pl_plglevel_wrapper(int ac,APIARGS *av){
	pl_plglevel((int *)av[1].argval);	/* int *p_level */

}

APIARGS pl_plgpage_args[]={
	{ "pl_plgpage",VOID },
	{ "p_xp",FLOAT  | PTRBIT },
	{ "p_yp",FLOAT  | PTRBIT },
	{ "p_xleng",INT  | PTRBIT },
	{ "p_yleng",INT  | PTRBIT },
	{ "p_xoff",INT  | PTRBIT },
	{ "p_yoff",INT  | PTRBIT } };
void
pl_plgpage_wrapper(int ac,APIARGS *av){
	pl_plgpage((float *)av[1].argval,	/* float *p_xp */
		(float *)av[2].argval,	/* float *p_yp */
		(int *)av[3].argval,	/* int *p_xleng */
		(int *)av[4].argval,	/* int *p_yleng */
		(int *)av[5].argval,	/* int *p_xoff */
		(int *)av[6].argval);	/* int *p_yoff */

}

APIARGS pl_plgra_args[]={
	{ "pl_plgra",VOID } };
void
pl_plgra_wrapper(int ac,APIARGS *av){
	pl_plgra();
}

APIARGS pl_plgspa_args[]={
	{ "pl_plgspa",VOID },
	{ "xmin",FLOAT  | PTRBIT },
	{ "xmax",FLOAT  | PTRBIT },
	{ "ymin",FLOAT  | PTRBIT },
	{ "ymax",FLOAT  | PTRBIT } };
void
pl_plgspa_wrapper(int ac,APIARGS *av){
	pl_plgspa((float *)av[1].argval,	/* float *xmin */
		(float *)av[2].argval,	/* float *xmax */
		(float *)av[3].argval,	/* float *ymin */
		(float *)av[4].argval);	/* float *ymax */

}

APIARGS pl_plgstrm_args[]={
	{ "pl_plgstrm",VOID },
	{ "p_strm",INT  | PTRBIT } };
void
pl_plgstrm_wrapper(int ac,APIARGS *av){
	pl_plgstrm((int *)av[1].argval);	/* int *p_strm */

}

APIARGS pl_plgver_args[]={
	{ "pl_plgver",VOID },
	{ "p_ver",BYTE  | PTRBIT } };
void
pl_plgver_wrapper(int ac,APIARGS *av){
	pl_plgver((char *)av[1].argval);	/* char *p_ver */

}


APIARGS pl_plgwid_args[]={
	{ "pl_plgwid",VOID },
	{ "p_wid",INT  | PTRBIT } };
void
pl_plgwid_wrapper(int ac,APIARGS *av){
	pl_plgwid((int *)av[1].argval);	/* int *p_wid */

}

APIARGS pl_plgxax_args[]={
	{ "pl_plgxax",VOID },
	{ "p_digmax",INT  | PTRBIT },
	{ "p_digits",INT  | PTRBIT } };
void
pl_plgxax_wrapper(int ac,APIARGS *av){
	pl_plgxax((int *)av[1].argval,	/* int *p_digmax */
		(int *)av[2].argval);	/* int *p_digits */

}

APIARGS pl_plgyax_args[]={
	{ "pl_plgyax",VOID },
	{ "p_digmax",INT  | PTRBIT },
	{ "p_digits",INT  | PTRBIT } };
void
pl_plgyax_wrapper(int ac,APIARGS *av){
	pl_plgyax((int *)av[1].argval,	/* int *p_digmax */
		(int *)av[2].argval);	/* int *p_digits */

}

APIARGS pl_plgzax_args[]={
	{ "pl_plgzax",VOID },
	{ "p_digmax",INT  | PTRBIT },
	{ "p_digits",INT  | PTRBIT } };
void
pl_plgzax_wrapper(int ac,APIARGS *av){
	pl_plgzax((int *)av[1].argval,	/* int *p_digmax */
		(int *)av[2].argval);	/* int *p_digits */

}

APIARGS pl_plhist_args[]={
	{ "pl_plhist",VOID },
	{ "n",INT },
	{ "data",FLOAT  | PTRBIT },
	{ "datmin",FLOAT },
	{ "datmax",FLOAT },
	{ "nbin",INT },
	{ "oldwin",INT } };
void
pl_plhist_wrapper(int ac,APIARGS *av){
	pl_plhist(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *data */
		*((float *)av[3].argval),	/* float datmin */
		*((float *)av[4].argval),	/* float datmax */
		*((int *)av[5].argval),	/* int nbin */
		*((int *)av[6].argval));	/* int oldwin */

}

APIARGS pl_plhls_args[]={
	{ "pl_plhls",VOID },
	{ "h",FLOAT },
	{ "l",FLOAT },
	{ "s",FLOAT } };
void
pl_plhls_wrapper(int ac,APIARGS *av){
	pl_plhls(*((float *)av[1].argval),	/* float h */
		*((float *)av[2].argval),	/* float l */
		*((float *)av[3].argval));	/* float s */

}

APIARGS pl_plinit_args[]={
	{ "pl_plinit",VOID } };
void
pl_plinit_wrapper(int ac,APIARGS *av){
	pl_plinit();
}

APIARGS pl_pljoin_args[]={
	{ "pl_pljoin",VOID },
	{ "x1",FLOAT },
	{ "y1",FLOAT },
	{ "x2",FLOAT },
	{ "y2",FLOAT } };
void
pl_pljoin_wrapper(int ac,APIARGS *av){
	pl_pljoin(*((float *)av[1].argval),	/* float x1 */
		*((float *)av[2].argval),	/* float y1 */
		*((float *)av[3].argval),	/* float x2 */
		*((float *)av[4].argval));	/* float y2 */

}

APIARGS pl_pllab_args[]={
	{ "pl_pllab",VOID },
	{ "xlabel",CONSTBIT | BYTE  | PTRBIT },
	{ "ylabel",CONSTBIT | BYTE  | PTRBIT },
	{ "tlabel",CONSTBIT | BYTE  | PTRBIT } };
void
pl_pllab_wrapper(int ac,APIARGS *av){
	pl_pllab((char *)av[1].argval,	/* char *xlabel */
		(char *)av[2].argval,	/* char *ylabel */
		(char *)av[3].argval);	/* char *tlabel */

}

APIARGS pl_plline_args[]={
	{ "pl_plline",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT } };
void
pl_plline_wrapper(int ac,APIARGS *av){
	pl_plline(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval);	/* float *y */

}

APIARGS pl_plline3_args[]={
	{ "pl_plline3",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "z",FLOAT  | PTRBIT } };
void
pl_plline3_wrapper(int ac,APIARGS *av){
	pl_plline3(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		(float *)av[4].argval);	/* float *z */

}

APIARGS pl_pllsty_args[]={
	{ "pl_pllsty",VOID },
	{ "lin",INT } };
void
pl_pllsty_wrapper(int ac,APIARGS *av){
	pl_pllsty(*((int *)av[1].argval));	/* int lin */

}

APIARGS pl_plmkstrm_args[]={
	{ "pl_plmkstrm",VOID },
	{ "p_strm",INT  | PTRBIT } };
void
pl_plmkstrm_wrapper(int ac,APIARGS *av){
	pl_plmkstrm((int *)av[1].argval);	/* int *p_strm */

}

APIARGS pl_plmtex_args[]={
	{ "pl_plmtex",VOID },
	{ "side",CONSTBIT | BYTE  | PTRBIT },
	{ "disp",FLOAT },
	{ "pos",FLOAT },
	{ "just",FLOAT },
	{ "text",CONSTBIT | BYTE  | PTRBIT } };
void
pl_plmtex_wrapper(int ac,APIARGS *av){
	pl_plmtex((char *)av[1].argval,	/* char *side */
		*((float *)av[2].argval),	/* float disp */
		*((float *)av[3].argval),	/* float pos */
		*((float *)av[4].argval),	/* float just */
		(char *)av[5].argval);	/* char *text */

}

APIARGS pl_plpat_args[]={
	{ "pl_plpat",VOID },
	{ "nlin",INT },
	{ "inc",INT  | PTRBIT },
	{ "del",INT  | PTRBIT } };
void
pl_plpat_wrapper(int ac,APIARGS *av){
	pl_plpat(*((int *)av[1].argval),	/* int nlin */
		(int *)av[2].argval,	/* int *inc */
		(int *)av[3].argval);	/* int *del */

}

APIARGS pl_plpoin_args[]={
	{ "pl_plpoin",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "code",INT } };
void
pl_plpoin_wrapper(int ac,APIARGS *av){
	pl_plpoin(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		*((int *)av[4].argval));	/* int code */

}

APIARGS pl_plpoin3_args[]={
	{ "pl_plpoin3",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "z",FLOAT  | PTRBIT },
	{ "code",INT } };
void
pl_plpoin3_wrapper(int ac,APIARGS *av){
	pl_plpoin3(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		(float *)av[4].argval,	/* float *z */
		*((int *)av[5].argval));	/* int code */

}

APIARGS pl_plpoly3_args[]={
	{ "pl_plpoly3",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "z",FLOAT  | PTRBIT },
	{ "draw",INT  | PTRBIT } };
void
pl_plpoly3_wrapper(int ac,APIARGS *av){
	pl_plpoly3(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		(float *)av[4].argval,	/* float *z */
		(int *)av[5].argval);	/* int *draw */

}

APIARGS pl_plprepl_args[]={
	{ "pl_plprec",VOID },
	{ "setp",INT },
	{ "prec",INT } };
void
pl_plprepl_wrapper(int ac,APIARGS *av){
	pl_plprec(*((int *)av[1].argval),	/* int setp */
		*((int *)av[2].argval));	/* int prec */

}

APIARGS pl_plpsty_args[]={
	{ "pl_plpsty",VOID },
	{ "patt",INT } };
void
pl_plpsty_wrapper(int ac,APIARGS *av){
	pl_plpsty(*((int *)av[1].argval));	/* int patt */

}

APIARGS pl_plptex_args[]={
	{ "pl_plptex",VOID },
	{ "x",FLOAT },
	{ "y",FLOAT },
	{ "dx",FLOAT },
	{ "dy",FLOAT },
	{ "just",FLOAT },
	{ "text",CONSTBIT | BYTE  | PTRBIT } };
void
pl_plptex_wrapper(int ac,APIARGS *av){
	pl_plptex(*((float *)av[1].argval),	/* float x */
		*((float *)av[2].argval),	/* float y */
		*((float *)av[3].argval),	/* float dx */
		*((float *)av[4].argval),	/* float dy */
		*((float *)av[5].argval),	/* float just */
		(char *)av[6].argval);	/* char *text */

}

APIARGS pl_plptexd_args[]={
	{ "pl_plptexd",VOID },
	{ "x",FLOAT },
	{ "y",FLOAT },
	{ "dx",FLOAT },
	{ "dy",FLOAT },
	{ "just",FLOAT },
	{ "text",CONSTBIT | BYTE  | PTRBIT } };
void
pl_plptexd_wrapper(int ac,APIARGS *av){
	pl_plptexd(*((float *)av[1].argval),	/* float x */
		*((float *)av[2].argval),	/* float y */
		*((float *)av[3].argval),	/* float dx */
		*((float *)av[4].argval),	/* float dy */
		*((float *)av[5].argval),	/* float just */
		(char *)av[6].argval);	/* char *text */

}

APIARGS pl_plreplot_args[]={
	{ "pl_plreplot",VOID } };
void
pl_plreplot_wrapper(int ac,APIARGS *av){
	pl_plreplot();
}

APIARGS pl_plrgb_args[]={
	{ "pl_plrgb",VOID },
	{ "r",FLOAT },
	{ "g",FLOAT },
	{ "b",FLOAT } };
void
pl_plrgb_wrapper(int ac,APIARGS *av){
	pl_plrgb(*((float *)av[1].argval),	/* float r */
		*((float *)av[2].argval),	/* float g */
		*((float *)av[3].argval));	/* float b */

}

APIARGS pl_plrgb1_args[]={
	{ "pl_plrgb1",VOID },
	{ "r",INT },
	{ "g",INT },
	{ "b",INT } };
void
pl_plrgb1_wrapper(int ac,APIARGS *av){
	pl_plrgb1(*((int *)av[1].argval),	/* int r */
		*((int *)av[2].argval),	/* int g */
		*((int *)av[3].argval));	/* int b */

}

APIARGS pl_plschr_args[]={
	{ "pl_plschr",VOID },
	{ "def",FLOAT },
	{ "scale",FLOAT } };
void
pl_plschr_wrapper(int ac,APIARGS *av){
	pl_plschr(*((float *)av[1].argval),	/* float def */
		*((float *)av[2].argval));	/* float scale */

}

APIARGS pl_plscmap0_args[]={
	{ "pl_plscmap0",VOID },
	{ "r",INT  | PTRBIT },
	{ "g",INT  | PTRBIT },
	{ "b",INT  | PTRBIT },
	{ "ncol0",INT } };
void
pl_plscmap0_wrapper(int ac,APIARGS *av){
	pl_plscmap0((int *)av[1].argval,	/* int *r */
		(int *)av[2].argval,	/* int *g */
		(int *)av[3].argval,	/* int *b */
		*((int *)av[4].argval));	/* int ncol0 */

}

APIARGS pl_plscmap0n_args[]={
	{ "pl_plscmap0n",VOID },
	{ "ncol0",INT } };
void
pl_plscmap0n_wrapper(int ac,APIARGS *av){
	pl_plscmap0n(*((int *)av[1].argval));	/* int ncol0 */

}

APIARGS pl_plscmap1_args[]={
	{ "pl_plscmap1",VOID },
	{ "r",INT  | PTRBIT },
	{ "g",INT  | PTRBIT },
	{ "b",INT  | PTRBIT },
	{ "ncol1",INT } };
void
pl_plscmap1_wrapper(int ac,APIARGS *av){
	pl_plscmap1((int *)av[1].argval,	/* int *r */
		(int *)av[2].argval,	/* int *g */
		(int *)av[3].argval,	/* int *b */
		*((int *)av[4].argval));	/* int ncol1 */

}

APIARGS pl_plscmap1l_args[]={
	{ "pl_plscmap1l",VOID },
	{ "itype",INT },
	{ "npts",INT },
	{ "intensity",FLOAT  | PTRBIT },
	{ "coord1",FLOAT  | PTRBIT },
	{ "coord2",FLOAT  | PTRBIT },
	{ "coord3",FLOAT  | PTRBIT },
	{ "rev",INT  | PTRBIT } };
void
pl_plscmap1l_wrapper(int ac,APIARGS *av){
	pl_plscmap1l(*((int *)av[1].argval),	/* int itype */
		*((int *)av[2].argval),	/* int npts */
		(float *)av[3].argval,	/* float *intensity */
		(float *)av[4].argval,	/* float *coord1 */
		(float *)av[5].argval,	/* float *coord2 */
		(float *)av[6].argval,	/* float *coord3 */
		(int *)av[7].argval);	/* int *rev */

}

APIARGS pl_plscmap1n_args[]={
	{ "pl_plscmap1n",VOID },
	{ "ncol1",INT } };
void
pl_plscmap1n_wrapper(int ac,APIARGS *av){
	pl_plscmap1n(*((int *)av[1].argval));	/* int ncol1 */

}

APIARGS pl_plscol0_args[]={
	{ "pl_plscol0",VOID },
	{ "icol0",INT },
	{ "r",INT },
	{ "g",INT },
	{ "b",INT } };
void
pl_plscol0_wrapper(int ac,APIARGS *av){
	pl_plscol0(*((int *)av[1].argval),	/* int icol0 */
		*((int *)av[2].argval),	/* int r */
		*((int *)av[3].argval),	/* int g */
		*((int *)av[4].argval));	/* int b */

}

APIARGS pl_plscolbg_args[]={
	{ "pl_plscolbg",VOID },
	{ "r",INT },
	{ "g",INT },
	{ "b",INT } };
void
pl_plscolbg_wrapper(int ac,APIARGS *av){
	pl_plscolbg(*((int *)av[1].argval),	/* int r */
		*((int *)av[2].argval),	/* int g */
		*((int *)av[3].argval));	/* int b */

}

APIARGS pl_plscolor_args[]={
	{ "pl_plscolor",VOID },
	{ "color",INT } };
void
pl_plscolor_wrapper(int ac,APIARGS *av){
	pl_plscolor(*((int *)av[1].argval));	/* int color */

}

APIARGS pl_plsdev_args[]={
	{ "pl_plsdev",VOID },
	{ "devname",CONSTBIT | BYTE  | PTRBIT } };
void
pl_plsdev_wrapper(int ac,APIARGS *av){
	pl_plsdev((char *)av[1].argval);	/* char *devname */

}

APIARGS pl_plsdidev_args[]={
	{ "pl_plsdidev",VOID },
	{ "mar",FLOAT },
	{ "aspect",FLOAT },
	{ "jx",FLOAT },
	{ "jy",FLOAT } };
void
pl_plsdidev_wrapper(int ac,APIARGS *av){
	pl_plsdidev(*((float *)av[1].argval),	/* float mar */
		*((float *)av[2].argval),	/* float aspect */
		*((float *)av[3].argval),	/* float jx */
		*((float *)av[4].argval));	/* float jy */

}

APIARGS pl_plsdimap_args[]={
	{ "pl_plsdimap",VOID },
	{ "dimxmin",INT },
	{ "dimxmax",INT },
	{ "dimymin",INT },
	{ "dimymax",INT },
	{ "dimxpmm",FLOAT },
	{ "dimypmm",FLOAT } };
void
pl_plsdimap_wrapper(int ac,APIARGS *av){
	pl_plsdimap(*((int *)av[1].argval),	/* int dimxmin */
		*((int *)av[2].argval),	/* int dimxmax */
		*((int *)av[3].argval),	/* int dimymin */
		*((int *)av[4].argval),	/* int dimymax */
		*((float *)av[5].argval),	/* float dimxpmm */
		*((float *)av[6].argval));	/* float dimypmm */

}

APIARGS pl_plsdiori_args[]={
	{ "pl_plsdiori",VOID },
	{ "rot",FLOAT } };
void
pl_plsdiori_wrapper(int ac,APIARGS *av){
	pl_plsdiori(*((float *)av[1].argval));	/* float rot */

}

APIARGS pl_plsdiplt_args[]={
	{ "pl_plsdiplt",VOID },
	{ "xmin",FLOAT },
	{ "ymin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymax",FLOAT } };
void
pl_plsdiplt_wrapper(int ac,APIARGS *av){
	pl_plsdiplt(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float ymin */
		*((float *)av[3].argval),	/* float xmax */
		*((float *)av[4].argval));	/* float ymax */

}

APIARGS pl_plsdiplz_args[]={
	{ "pl_plsdiplz",VOID },
	{ "xmin",FLOAT },
	{ "ymin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymax",FLOAT } };
void
pl_plsdiplz_wrapper(int ac,APIARGS *av){
	pl_plsdiplz(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float ymin */
		*((float *)av[3].argval),	/* float xmax */
		*((float *)av[4].argval));	/* float ymax */

}

APIARGS pl_plsespl_args[]={
	{ "pl_plsesc",VOID },
	{ "esc",BYTE } };
void
pl_plsespl_wrapper(int ac,APIARGS *av){
	pl_plsesc(*((char *)av[1].argval));	/* char esc */

}

APIARGS pl_plsfam_args[]={
	{ "pl_plsfam",VOID },
	{ "fam",INT },
	{ "num",INT },
	{ "bmax",INT } };
void
pl_plsfam_wrapper(int ac,APIARGS *av){
	pl_plsfam(*((int *)av[1].argval),	/* int fam */
		*((int *)av[2].argval),	/* int num */
		*((int *)av[3].argval));	/* int bmax */

}

APIARGS pl_plsfnam_args[]={
	{ "pl_plsfnam",VOID },
	{ "fnam",CONSTBIT | BYTE  | PTRBIT } };
void
pl_plsfnam_wrapper(int ac,APIARGS *av){
	pl_plsfnam((char *)av[1].argval);	/* char *fnam */

}

APIARGS pl_plsmaj_args[]={
	{ "pl_plsmaj",VOID },
	{ "def",FLOAT },
	{ "scale",FLOAT } };
void
pl_plsmaj_wrapper(int ac,APIARGS *av){
	pl_plsmaj(*((float *)av[1].argval),	/* float def */
		*((float *)av[2].argval));	/* float scale */

}

APIARGS pl_plsmin_args[]={
	{ "pl_plsmin",VOID },
	{ "def",FLOAT },
	{ "scale",FLOAT } };
void
pl_plsmin_wrapper(int ac,APIARGS *av){
	pl_plsmin(*((float *)av[1].argval),	/* float def */
		*((float *)av[2].argval));	/* float scale */

}

APIARGS pl_plsori_args[]={
	{ "pl_plsori",VOID },
	{ "ori",INT } };
void
pl_plsori_wrapper(int ac,APIARGS *av){
	pl_plsori(*((int *)av[1].argval));	/* int ori */

}

APIARGS pl_plspage_args[]={
	{ "pl_plspage",VOID },
	{ "xp",FLOAT },
	{ "yp",FLOAT },
	{ "xleng",INT },
	{ "yleng",INT },
	{ "xoff",INT },
	{ "yoff",INT } };
void
pl_plspage_wrapper(int ac,APIARGS *av){
	pl_plspage(*((float *)av[1].argval),	/* float xp */
		*((float *)av[2].argval),	/* float yp */
		*((int *)av[3].argval),	/* int xleng */
		*((int *)av[4].argval),	/* int yleng */
		*((int *)av[5].argval),	/* int xoff */
		*((int *)av[6].argval));	/* int yoff */

}

APIARGS pl_plspause_args[]={
	{ "pl_plspause",VOID },
	{ "pause",INT } };
void
pl_plspause_wrapper(int ac,APIARGS *av){
	pl_plspause(*((int *)av[1].argval));	/* int pause */

}

APIARGS pl_plsstrm_args[]={
	{ "pl_plsstrm",VOID },
	{ "strm",INT } };
void
pl_plsstrm_wrapper(int ac,APIARGS *av){
	pl_plsstrm(*((int *)av[1].argval));	/* int strm */

}

APIARGS pl_plssub_args[]={
	{ "pl_plssub",VOID },
	{ "nx",INT },
	{ "ny",INT } };
void
pl_plssub_wrapper(int ac,APIARGS *av){
	pl_plssub(*((int *)av[1].argval),	/* int nx */
		*((int *)av[2].argval));	/* int ny */

}

APIARGS pl_plssym_args[]={
	{ "pl_plssym",VOID },
	{ "def",FLOAT },
	{ "scale",FLOAT } };
void
pl_plssym_wrapper(int ac,APIARGS *av){
	pl_plssym(*((float *)av[1].argval),	/* float def */
		*((float *)av[2].argval));	/* float scale */

}

APIARGS pl_plstar_args[]={
	{ "pl_plstar",VOID },
	{ "nx",INT },
	{ "ny",INT } };
void
pl_plstar_wrapper(int ac,APIARGS *av){
	pl_plstar(*((int *)av[1].argval),	/* int nx */
		*((int *)av[2].argval));	/* int ny */

}

APIARGS pl_plstart_args[]={
	{ "pl_plstart",VOID },
	{ "devname",CONSTBIT | BYTE  | PTRBIT },
	{ "nx",INT },
	{ "ny",INT } };
void
pl_plstart_wrapper(int ac,APIARGS *av){
	pl_plstart((char *)av[1].argval,	/* char *devname */
		*((int *)av[2].argval),	/* int nx */
		*((int *)av[3].argval));	/* int ny */

}

APIARGS pl_plstyl_args[]={
	{ "pl_plstyl",VOID },
	{ "nms",INT },
	{ "mark",INT  | PTRBIT },
	{ "space",INT  | PTRBIT } };
void
pl_plstyl_wrapper(int ac,APIARGS *av){
	pl_plstyl(*((int *)av[1].argval),	/* int nms */
		(int *)av[2].argval,	/* int *mark */
		(int *)av[3].argval);	/* int *space */

}

APIARGS pl_plsvpa_args[]={
	{ "pl_plsvpa",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT } };
void
pl_plsvpa_wrapper(int ac,APIARGS *av){
	pl_plsvpa(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float xmax */
		*((float *)av[3].argval),	/* float ymin */
		*((float *)av[4].argval));	/* float ymax */

}

APIARGS pl_plsxax_args[]={
	{ "pl_plsxax",VOID },
	{ "digmax",INT },
	{ "digits",INT } };
void
pl_plsxax_wrapper(int ac,APIARGS *av){
	pl_plsxax(*((int *)av[1].argval),	/* int digmax */
		*((int *)av[2].argval));	/* int digits */

}

APIARGS pl_plsyax_args[]={
	{ "pl_plsyax",VOID },
	{ "digmax",INT },
	{ "digits",INT } };
void
pl_plsyax_wrapper(int ac,APIARGS *av){
	pl_plsyax(*((int *)av[1].argval),	/* int digmax */
		*((int *)av[2].argval));	/* int digits */

}

APIARGS pl_plsym_args[]={
	{ "pl_plsym",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "code",INT } };
void
pl_plsym_wrapper(int ac,APIARGS *av){
	pl_plsym(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		*((int *)av[4].argval));	/* int code */

}

APIARGS pl_plszax_args[]={
	{ "pl_plszax",VOID },
	{ "digmax",INT },
	{ "digits",INT } };
void
pl_plszax_wrapper(int ac,APIARGS *av){
	pl_plszax(*((int *)av[1].argval),	/* int digmax */
		*((int *)av[2].argval));	/* int digits */

}

APIARGS pl_pltext_args[]={
	{ "pl_pltext",VOID } };
void
pl_pltext_wrapper(int ac,APIARGS *av){
	pl_pltext();
}

APIARGS pl_plvasp_args[]={
	{ "pl_plvasp",VOID },
	{ "aspect",FLOAT } };
void
pl_plvasp_wrapper(int ac,APIARGS *av){
	pl_plvasp(*((float *)av[1].argval));	/* float aspect */

}

APIARGS pl_plvpas_args[]={
	{ "pl_plvpas",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT },
	{ "aspect",FLOAT } };
void
pl_plvpas_wrapper(int ac,APIARGS *av){
	pl_plvpas(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float xmax */
		*((float *)av[3].argval),	/* float ymin */
		*((float *)av[4].argval),	/* float ymax */
		*((float *)av[5].argval));	/* float aspect */

}

APIARGS pl_plvpor_args[]={
	{ "pl_plvpor",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT } };
void
pl_plvpor_wrapper(int ac,APIARGS *av){
	pl_plvpor(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float xmax */
		*((float *)av[3].argval),	/* float ymin */
		*((float *)av[4].argval));	/* float ymax */

}

APIARGS pl_plvsta_args[]={
	{ "pl_plvsta",VOID } };
void
pl_plvsta_wrapper(int ac,APIARGS *av){
	pl_plvsta();
}

APIARGS pl_plw3d_args[]={
	{ "pl_plw3d",VOID },
	{ "basex",FLOAT },
	{ "basey",FLOAT },
	{ "height",FLOAT },
	{ "xmin0",FLOAT },
	{ "xmax0",FLOAT },
	{ "ymin0",FLOAT },
	{ "ymax0",FLOAT },
	{ "zmin0",FLOAT },
	{ "zmax0",FLOAT },
	{ "alt",FLOAT },
	{ "az",FLOAT } };
void
pl_plw3d_wrapper(int ac,APIARGS *av){
	pl_plw3d(*((float *)av[1].argval),	/* float basex */
		*((float *)av[2].argval),	/* float basey */
		*((float *)av[3].argval),	/* float height */
		*((float *)av[4].argval),	/* float xmin0 */
		*((float *)av[5].argval),	/* float xmax0 */
		*((float *)av[6].argval),	/* float ymin0 */
		*((float *)av[7].argval),	/* float ymax0 */
		*((float *)av[8].argval),	/* float zmin0 */
		*((float *)av[9].argval),	/* float zmax0 */
		*((float *)av[10].argval),	/* float alt */
		*((float *)av[11].argval));	/* float az */

}

APIARGS pl_plwid_args[]={
	{ "pl_plwid",VOID },
	{ "width",INT } };
void
pl_plwid_wrapper(int ac,APIARGS *av){
	pl_plwid(*((int *)av[1].argval));	/* int width */

}

APIARGS pl_plwind_args[]={
	{ "pl_plwind",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT } };
void
pl_plwind_wrapper(int ac,APIARGS *av){
	pl_plwind(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float xmax */
		*((float *)av[3].argval),	/* float ymin */
		*((float *)av[4].argval));	/* float ymax */

}

APIARGS plClearOpts_args[]={
	{ "plClearOpts",VOID } };
void
plClearOpts_wrapper(int ac,APIARGS *av){
	plClearOpts();
}

APIARGS plFindCommand_args[]={
	{ "plFindCommand",BYTE  | PTRBIT },
	{ "fn",BYTE  | PTRBIT } };
void
plFindCommand_wrapper(int ac,APIARGS *av){
	char * retval;
	retval = plFindCommand((char *)av[1].argval);	/* char *fn */

	av[0].argval = retval;
}

APIARGS plFindName_args[]={
	{ "plFindName",INT },
	{ "p",BYTE  | PTRBIT } };
void
plFindName_wrapper(int ac,APIARGS *av){
	int  retval;
	retval = plFindName((char *)av[1].argval);	/* char *p */

	av[0].argval = memdup(&retval,sizeof(retval));
}

APIARGS plGetFlt_args[]={
	{ "plGetFlt",FLOAT },
	{ "s",BYTE  | PTRBIT } };
void
plGetFlt_wrapper(int ac,APIARGS *av){
	float  retval;
	retval = plGetFlt((char *)av[1].argval);	/* char *s */

	av[0].argval = memdup(&retval,sizeof(retval));
}

APIARGS plGetInt_args[]={
	{ "plGetInt",INT },
	{ "s",BYTE  | PTRBIT } };
void
plGetInt_wrapper(int ac,APIARGS *av){
	int  retval;
	retval = plGetInt((char *)av[1].argval);	/* char *s */

	av[0].argval = memdup(&retval,sizeof(retval));
}

APIARGS plHLS_RGB_args[]={
	{ "plHLS_RGB",VOID },
	{ "h",FLOAT },
	{ "l",FLOAT },
	{ "s",FLOAT },
	{ "p_r",FLOAT  | PTRBIT },
	{ "p_g",FLOAT  | PTRBIT },
	{ "p_b",FLOAT  | PTRBIT } };
void
plHLS_RGB_wrapper(int ac,APIARGS *av){
	plHLS_RGB(*((float *)av[1].argval),	/* float h */
		*((float *)av[2].argval),	/* float l */
		*((float *)av[3].argval),	/* float s */
		(float *)av[4].argval,	/* float *p_r */
		(float *)av[5].argval,	/* float *p_g */
		(float *)av[6].argval);	/* float *p_b */

}

APIARGS plOptUsage_args[]={
	{ "plOptUsage",VOID } };
void
plOptUsage_wrapper(int ac,APIARGS *av){
	plOptUsage();
}

APIARGS plRGB_HLS_args[]={
	{ "plRGB_HLS",VOID },
	{ "r",FLOAT },
	{ "g",FLOAT },
	{ "b",FLOAT },
	{ "p_h",FLOAT  | PTRBIT },
	{ "p_l",FLOAT  | PTRBIT },
	{ "p_s",FLOAT  | PTRBIT } };
void
plRGB_HLS_wrapper(int ac,APIARGS *av){
	plRGB_HLS(*((float *)av[1].argval),	/* float r */
		*((float *)av[2].argval),	/* float g */
		*((float *)av[3].argval),	/* float b */
		(float *)av[4].argval,	/* float *p_h */
		(float *)av[5].argval,	/* float *p_l */
		(float *)av[6].argval);	/* float *p_s */

}

APIARGS plResetOpts_args[]={
	{ "plResetOpts",VOID } };
void
plResetOpts_wrapper(int ac,APIARGS *av){
	plResetOpts();
}

APIARGS plSetOpt_args[]={
	{ "plSetOpt",INT },
	{ "opt",BYTE  | PTRBIT },
	{ "optarg",BYTE  | PTRBIT } };
void
plSetOpt_wrapper(int ac,APIARGS *av){
	int  retval;
	retval = plSetOpt((char *)av[1].argval,	/* char *opt */
		(char *)av[2].argval);	/* char *optarg */

	av[0].argval = memdup(&retval,sizeof(retval));
}

APIARGS plSetUsage_args[]={
	{ "plSetUsage",VOID },
	{ "program_string",BYTE  | PTRBIT },
	{ "usage_string",BYTE  | PTRBIT } };
void
plSetUsage_wrapper(int ac,APIARGS *av){
	plSetUsage((char *)av[1].argval,	/* char *program_string */
		(char *)av[2].argval);	/* char *usage_string */

}

APIARGS pldid2pc_args[]={
	{ "pldid2pc",VOID },
	{ "xmin",FLOAT  | PTRBIT },
	{ "ymin",FLOAT  | PTRBIT },
	{ "xmax",FLOAT  | PTRBIT },
	{ "ymax",FLOAT  | PTRBIT } };
void
pldid2pc_wrapper(int ac,APIARGS *av){
	pldid2pc((float *)av[1].argval,	/* float *xmin */
		(float *)av[2].argval,	/* float *ymin */
		(float *)av[3].argval,	/* float *xmax */
		(float *)av[4].argval);	/* float *ymax */

}

APIARGS pldip2dc_args[]={
	{ "pldip2dc",VOID },
	{ "xmin",FLOAT  | PTRBIT },
	{ "ymin",FLOAT  | PTRBIT },
	{ "xmax",FLOAT  | PTRBIT },
	{ "ymax",FLOAT  | PTRBIT } };
void
pldip2dc_wrapper(int ac,APIARGS *av){
	pldip2dc((float *)av[1].argval,	/* float *xmin */
		(float *)av[2].argval,	/* float *ymin */
		(float *)av[3].argval,	/* float *xmax */
		(float *)av[4].argval);	/* float *ymax */

}

APIARGS plf2eval_args[]={
	{ "plf2eval",FLOAT },
	{ "ix",INT },
	{ "iy",INT },
	{ "plf2eval_data",VOID  | PTRBIT } };
void
plf2eval_wrapper(int ac,APIARGS *av){
	float  retval;
	retval = plf2eval(*((int *)av[1].argval),	/* int ix */
		*((int *)av[2].argval),	/* int iy */
		(void *)av[3].argval);	/* void *plf2eval_data */

	av[0].argval = memdup(&retval,sizeof(retval));
}

APIARGS plf2eval2_args[]={
	{ "plf2eval2",FLOAT },
	{ "ix",INT },
	{ "iy",INT },
	{ "plf2eval_data",VOID  | PTRBIT } };
void
plf2eval2_wrapper(int ac,APIARGS *av){
	float  retval;
	retval = plf2eval2(*((int *)av[1].argval),	/* int ix */
		*((int *)av[2].argval),	/* int iy */
		(void *)av[3].argval);	/* void *plf2eval_data */

	av[0].argval = memdup(&retval,sizeof(retval));
}

APIARGS plf2evalr_args[]={
	{ "plf2evalr",FLOAT },
	{ "ix",INT },
	{ "iy",INT },
	{ "plf2eval_data",VOID  | PTRBIT } };
void
plf2evalr_wrapper(int ac,APIARGS *av){
	float  retval;
	retval = plf2evalr(*((int *)av[1].argval),	/* int ix */
		*((int *)av[2].argval),	/* int iy */
		(void *)av[3].argval);	/* void *plf2eval_data */

	av[0].argval = memdup(&retval,sizeof(retval));
}

APIARGS plgesc_args[]={
	{ "plgesc",VOID },
	{ "p_esc",BYTE  | PTRBIT } };
void
plgesc_wrapper(int ac,APIARGS *av){
	plgesc((char *)av[1].argval);	/* char *p_esc */

}

APIARGS plsError_args[]={
	{ "plsError",VOID },
	{ "errcode",INT  | PTRBIT },
	{ "errmsg",BYTE  | PTRBIT } };
void
plsError_wrapper(int ac,APIARGS *av){
	plsError((int *)av[1].argval,	/* int *errcode */
		(char *)av[2].argval);	/* char *errmsg */

}

APIARGS plsxwin_args[]={
	{ "plsxwin",VOID },
	{ "window_id",INT } };
void
plsxwin_wrapper(int ac,APIARGS *av){
	plsxwin(*((int *)av[1].argval));	/* int window_id */

}

#endif /* HAVE_PLPLOT */

/* APIDEFS */
const APIDEFS apidefs[] = {
	#ifdef HAVE_PLPLOT
	{ "pl_pladv",pl_pladv_args,2,pl_pladv_wrapper },
	{ "pl_plaxes",pl_plaxes_args,9,pl_plaxes_wrapper },
	{ "pl_plbin",pl_plbin_args,5,pl_plbin_wrapper },
	{ "pl_plbop",pl_plbop_args,1,pl_plbop_wrapper },
	{ "pl_plbox",pl_plbox_args,7,pl_plbox_wrapper },
	{ "pl_plbox3",pl_plbox3_args,13,pl_plbox3_wrapper },
	{ "pl_plcol0",pl_plcol0_args,2,pl_plcol0_wrapper },
	{ "pl_plcol1",pl_plcol1_args,2,pl_plcol1_wrapper },
	{ "pl_plcpstrm",pl_plcpstrm_args,3,pl_plcpstrm_wrapper },
	{ "pl_plcross",pl_plcross_args,3,pl_plcross_wrapper},
	{ "pl_plend",pl_plend_args,1,pl_plend_wrapper },
	{ "pl_plend1",pl_plend1_args,1,pl_plend1_wrapper },
	{ "pl_plenv",pl_plenv_args,7,pl_plenv_wrapper },
	{ "pl_pleop",pl_pleop_args,1,pl_pleop_wrapper },
	{ "pl_plerrx",pl_plerrx_args,5,pl_plerrx_wrapper },
	{ "pl_plerry",pl_plerry_args,5,pl_plerry_wrapper },
	{ "pl_plfamadv",pl_plfamadv_args,1,pl_plfamadv_wrapper },
	{ "pl_plfill",pl_plfill_args,4,pl_plfill_wrapper },
	{ "pl_plflush",pl_plflush_args,1,pl_plflush_wrapper },
	{ "pl_plfont",pl_plfont_args,2,pl_plfont_wrapper },
	{ "pl_plfontld",pl_plfontld_args,2,pl_plfontld_wrapper },
	{ "pl_plgchr",pl_plgchr_args,3,pl_plgchr_wrapper },
	{ "pl_plgcol0",pl_plgcol0_args,5,pl_plgcol0_wrapper },
	{ "pl_plgcolbg",pl_plgcolbg_args,4,pl_plgcolbg_wrapper },
	{ "pl_plgdev",pl_plgdev_args,2,pl_plgdev_wrapper },
	{ "pl_plgdidev",pl_plgdidev_args,5,pl_plgdidev_wrapper },
	{ "pl_plgdiori",pl_plgdiori_args,2,pl_plgdiori_wrapper },
	{ "pl_plgdiplt",pl_plgdiplt_args,5,pl_plgdiplt_wrapper },
	{ "pl_plgfam",pl_plgfam_args,4,pl_plgfam_wrapper },
	{ "pl_plgfnam",pl_plgfnam_args,2,pl_plgfnam_wrapper },
	{ "pl_plglevel",pl_plglevel_args,2,pl_plglevel_wrapper },
	{ "pl_plgpage",pl_plgpage_args,7,pl_plgpage_wrapper },
	{ "pl_plgra",pl_plgra_args,1,pl_plgra_wrapper },
	{ "pl_plgspa",pl_plgspa_args,5,pl_plgspa_wrapper },
	{ "pl_plgstrm",pl_plgstrm_args,2,pl_plgstrm_wrapper },
	{ "pl_plgver",pl_plgver_args,2,pl_plgver_wrapper },
	{ "pl_plgwid",pl_plgwid_args,2,pl_plgwid_wrapper },
	{ "pl_plgxax",pl_plgxax_args,3,pl_plgxax_wrapper },
	{ "pl_plgyax",pl_plgyax_args,3,pl_plgyax_wrapper },
	{ "pl_plgzax",pl_plgzax_args,3,pl_plgzax_wrapper },
	{ "pl_plhist",pl_plhist_args,7,pl_plhist_wrapper },
	{ "pl_plhls",pl_plhls_args,4,pl_plhls_wrapper },
	{ "pl_plinit",pl_plinit_args,1,pl_plinit_wrapper },
	{ "pl_pljoin",pl_pljoin_args,5,pl_pljoin_wrapper },
	{ "pl_pllab",pl_pllab_args,4,pl_pllab_wrapper },
	{ "pl_plline",pl_plline_args,4,pl_plline_wrapper },
	{ "pl_plline3",pl_plline3_args,5,pl_plline3_wrapper },
	{ "pl_pllsty",pl_pllsty_args,2,pl_pllsty_wrapper },
	{ "pl_plmkstrm",pl_plmkstrm_args,2,pl_plmkstrm_wrapper },
	{ "pl_plmtex",pl_plmtex_args,6,pl_plmtex_wrapper },
	{ "pl_plpat",pl_plpat_args,4,pl_plpat_wrapper },
	{ "pl_plpoin",pl_plpoin_args,5,pl_plpoin_wrapper },
	{ "pl_plpoin3",pl_plpoin3_args,6,pl_plpoin3_wrapper },
	{ "pl_plpoly3",pl_plpoly3_args,6,pl_plpoly3_wrapper },
	{ "pl_plprec",pl_plprepl_args,3,pl_plprepl_wrapper },
	{ "pl_plpsty",pl_plpsty_args,2,pl_plpsty_wrapper },
	{ "pl_plptex",pl_plptex_args,7,pl_plptex_wrapper },
	{ "pl_plptexd",pl_plptex_args,7,pl_plptex_wrapper },
	{ "pl_plreplot",pl_plreplot_args,1,pl_plreplot_wrapper },
	{ "pl_plrgb",pl_plrgb_args,4,pl_plrgb_wrapper },
	{ "pl_plrgb1",pl_plrgb1_args,4,pl_plrgb1_wrapper },
	{ "pl_plschr",pl_plschr_args,3,pl_plschr_wrapper },
	{ "pl_plscmap0",pl_plscmap0_args,5,pl_plscmap0_wrapper },
	{ "pl_plscmap0n",pl_plscmap0n_args,2,pl_plscmap0n_wrapper },
	{ "pl_plscmap1",pl_plscmap1_args,5,pl_plscmap1_wrapper },
	{ "pl_plscmap1l",pl_plscmap1l_args,8,pl_plscmap1l_wrapper },
	{ "pl_plscmap1n",pl_plscmap1n_args,2,pl_plscmap1n_wrapper },
	{ "pl_plscol0",pl_plscol0_args,5,pl_plscol0_wrapper },
	{ "pl_plscolbg",pl_plscolbg_args,4,pl_plscolbg_wrapper },
	{ "pl_plscolor",pl_plscolor_args,2,pl_plscolor_wrapper },
	{ "pl_plsdev",pl_plsdev_args,2,pl_plsdev_wrapper },
	{ "pl_plsdidev",pl_plsdidev_args,5,pl_plsdidev_wrapper },
	{ "pl_plsdimap",pl_plsdimap_args,7,pl_plsdimap_wrapper },
	{ "pl_plsdiori",pl_plsdiori_args,2,pl_plsdiori_wrapper },
	{ "pl_plsdiplt",pl_plsdiplt_args,5,pl_plsdiplt_wrapper },
	{ "pl_plsdiplz",pl_plsdiplz_args,5,pl_plsdiplz_wrapper },
	{ "pl_plsesc",pl_plsespl_args,2,pl_plsespl_wrapper },
	{ "pl_plsfam",pl_plsfam_args,4,pl_plsfam_wrapper },
	{ "pl_plsfnam",pl_plsfnam_args,2,pl_plsfnam_wrapper },
	{ "pl_plsmaj",pl_plsmaj_args,3,pl_plsmaj_wrapper },
	{ "pl_plsmin",pl_plsmin_args,3,pl_plsmin_wrapper },
	{ "pl_plsori",pl_plsori_args,2,pl_plsori_wrapper },
	{ "pl_plspage",pl_plspage_args,7,pl_plspage_wrapper },
	{ "pl_plspause",pl_plspause_args,2,pl_plspause_wrapper },
	{ "pl_plsstrm",pl_plsstrm_args,2,pl_plsstrm_wrapper },
	{ "pl_plssub",pl_plssub_args,3,pl_plssub_wrapper },
	{ "pl_plssym",pl_plssym_args,3,pl_plssym_wrapper },
	{ "pl_plstar",pl_plstar_args,3,pl_plstar_wrapper },
	{ "pl_plstart",pl_plstart_args,4,pl_plstart_wrapper },
	{ "pl_plstyl",pl_plstyl_args,4,pl_plstyl_wrapper },
	{ "pl_plsvpa",pl_plsvpa_args,5,pl_plsvpa_wrapper },
	{ "pl_plsxax",pl_plsxax_args,3,pl_plsxax_wrapper },
	{ "pl_plsyax",pl_plsyax_args,3,pl_plsyax_wrapper },
	{ "pl_plsym",pl_plsym_args,5,pl_plsym_wrapper },
	{ "pl_plszax",pl_plszax_args,3,pl_plszax_wrapper },
	{ "pl_pltext",pl_pltext_args,1,pl_pltext_wrapper },
	{ "pl_plvasp",pl_plvasp_args,2,pl_plvasp_wrapper },
	{ "pl_plvpas",pl_plvpas_args,6,pl_plvpas_wrapper },
	{ "pl_plvpor",pl_plvpor_args,5,pl_plvpor_wrapper },
	{ "pl_plvsta",pl_plvsta_args,1,pl_plvsta_wrapper },
	{ "pl_plw3d",pl_plw3d_args,12,pl_plw3d_wrapper },
	{ "pl_plwid",pl_plwid_args,2,pl_plwid_wrapper },
	{ "pl_plwind",pl_plwind_args,5,pl_plwind_wrapper },
	{ "plClearOpts",plClearOpts_args,1,plClearOpts_wrapper },
	{ "plFindCommand",plFindCommand_args,2,plFindCommand_wrapper },
	{ "plFindName",plFindName_args,2,plFindName_wrapper },
	{ "plGetFlt",plGetFlt_args,2,plGetFlt_wrapper },
	{ "plGetInt",plGetInt_args,2,plGetInt_wrapper },
	{ "plHLS_RGB",plHLS_RGB_args,7,plHLS_RGB_wrapper },
	{ "plOptUsage",plOptUsage_args,1,plOptUsage_wrapper },
	{ "plRGB_HLS",plRGB_HLS_args,7,plRGB_HLS_wrapper },
	{ "plResetOpts",plResetOpts_args,1,plResetOpts_wrapper },
	{ "plSetOpt",plSetOpt_args,3,plSetOpt_wrapper },
	{ "plSetUsage",plSetUsage_args,3,plSetUsage_wrapper },
	{ "pldid2pc",pldid2pc_args,5,pldid2pc_wrapper },
	{ "pldip2dc",pldip2dc_args,5,pldip2dc_wrapper },
	{ "plf2eval",plf2eval_args,4,plf2eval_wrapper },
	{ "plf2eval2",plf2eval2_args,4,plf2eval2_wrapper },
	{ "plf2evalr",plf2evalr_args,4,plf2evalr_wrapper },
	{ "plgesc",plgesc_args,2,plgesc_wrapper },
	{ "plsError",plsError_args,3,plsError_wrapper },
	{ "plsxwin",plsxwin_args,2,plsxwin_wrapper }
	#endif /* HAVE_PLPLOT */
};

#ifdef __cplusplus
}
#endif
