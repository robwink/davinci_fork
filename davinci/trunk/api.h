#include "parser.h"
#include "apidef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Prototypes; sorted in ascending order */
void c_pladv(int page);
void c_plaxes(float x0,float y0,char *xopt,float xtick,int nxsub,char *yopt,float ytick,int nysub);
void c_plbin(int nbin,float *x,float *y,int center);
void c_plbop();
void c_plbox(char *xopt,float xtick,int nxsub,char *yopt,float ytick,int nysub);
void c_plbox3(char *xopt,char *xlabel,float xtick,int nsubx,char *yopt,char *ylabel,float ytick,int nsuby,char *zopt,char *zlabel,float ztick,int nsubz);
void c_plcol0(int icol0);
void c_plcol1(float col1);
void c_plcpstrm(int iplsr,int flags);
void c_plend();
void c_plend1();
void c_plenv(float xmin,float xmax,float ymin,float ymax,int just,int axis);
void c_pleop();
void c_plerrx(int n,float *xmin,float *xmax,float *y);
void c_plerry(int n,float *x,float *ymin,float *ymax);
void c_plfamadv();
void c_plfill(int n,float *x,float *y);
void c_plflush();
void c_plfont(int ifont);
void c_plfontld(int fnt);
void c_plgchr(float *p_def,float *p_ht);
void c_plgcol0(int icol0,int *r,int *g,int *b);
void c_plgcolbg(int *r,int *g,int *b);
void c_plgdev(char *p_dev);
void c_plgdidev(float *p_mar,float *p_aspect,float *p_jx,float *p_jy);
void c_plgdiori(float *p_rot);
void c_plgdiplt(float *p_xmin,float *p_ymin,float *p_xmax,float *p_ymax);
void c_plgfam(int *p_fam,int *p_num,int *p_bmax);
void c_plgfnam(char *fnam);
void c_plglevel(int *p_level);
void c_plgpage(float *p_xp,float *p_yp,int *p_xleng,int *p_yleng,int *p_xoff,int *p_yoff);
void c_plgra();
void c_plgspa(float *xmin,float *xmax,float *ymin,float *ymax);
void c_plgstrm(int *p_strm);
void c_plgver(char *p_ver);
void c_plgxax(int *p_digmax,int *p_digits);
void c_plgyax(int *p_digmax,int *p_digits);
void c_plgzax(int *p_digmax,int *p_digits);
void c_plhist(int n,float *data,float datmin,float datmax,int nbin,int oldwin);
void c_plhls(float h,float l,float s);
void c_plinit();
void c_pljoin(float x1,float y1,float x2,float y2);
void c_pllab(char *xlabel,char *ylabel,char *tlabel);
void c_plline(int n,float *x,float *y);
void c_plline3(int n,float *x,float *y,float *z);
void c_pllsty(int lin);
void c_plmkstrm(int *p_strm);
void c_plmtex(char *side,float disp,float pos,float just,char *text);
void c_plpat(int nlin,int *inc,int *del);
void c_plpoin(int n,float *x,float *y,int code);
void c_plpoin3(int n,float *x,float *y,float *z,int code);
void c_plpoly3(int n,float *x,float *y,float *z,int *draw);
void c_plprec(int setp,int prec);
void c_plpsty(int patt);
void c_plptex(float x,float y,float dx,float dy,float just,char *text);
void c_plreplot();
void c_plrgb(float r,float g,float b);
void c_plrgb1(int r,int g,int b);
void c_plschr(float def,float scale);
void c_plscmap0(int *r,int *g,int *b,int ncol0);
void c_plscmap0n(int ncol0);
void c_plscmap1(int *r,int *g,int *b,int ncol1);
void c_plscmap1l(int itype,int npts,float *intensity,float *coord1,float *coord2,float *coord3,int *rev);
void c_plscmap1n(int ncol1);
void c_plscol0(int icol0,int r,int g,int b);
void c_plscolbg(int r,int g,int b);
void c_plscolor(int color);
void c_plsdev(char *devname);
void c_plsdidev(float mar,float aspect,float jx,float jy);
void c_plsdimap(int dimxmin,int dimxmax,int dimymin,int dimymax,float dimxpmm,float dimypmm);
void c_plsdiori(float rot);
void c_plsdiplt(float xmin,float ymin,float xmax,float ymax);
void c_plsdiplz(float xmin,float ymin,float xmax,float ymax);
void c_plsesc(char esc);
void c_plsfam(int fam,int num,int bmax);
void c_plsfnam(char *fnam);
void c_plsmaj(float def,float scale);
void c_plsmin(float def,float scale);
void c_plsori(int ori);
void c_plspage(float xp,float yp,int xleng,int yleng,int xoff,int yoff);
void c_plspause(int pause);
void c_plsstrm(int strm);
void c_plssub(int nx,int ny);
void c_plssym(float def,float scale);
void c_plstar(int nx,int ny);
void c_plstart(char *devname,int nx,int ny);
void c_plstyl(int nms,int *mark,int *space);
void c_plsvpa(float xmin,float xmax,float ymin,float ymax);
void c_plsxax(int digmax,int digits);
void c_plsyax(int digmax,int digits);
void c_plsym(int n,float *x,float *y,int code);
void c_plszax(int digmax,int digits);
void c_pltext();
void c_plvasp(float aspect);
void c_plvpas(float xmin,float xmax,float ymin,float ymax,float aspect);
void c_plvpor(float xmin,float xmax,float ymin,float ymax);
void c_plvsta();
void c_plw3d(float basex,float basey,float height,float xmin0,float xmax0,float ymin0,float ymax0,float zmin0,float zmax0,float alt,float az);
void c_plwid(int width);
void c_plwind(float xmin,float xmax,float ymin,float ymax);

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
APIARGS c_pladv_args[]={
	{ "c_pladv",VOID },
	{ "page",INT } };
void
c_pladv_wrapper(int ac,APIARGS *av){
	c_pladv(*((int *)av[1].argval));	/* int page */

}

APIARGS c_plaxes_args[]={
	{ "c_plaxes",VOID },
	{ "x0",FLOAT },
	{ "y0",FLOAT },
	{ "xopt",CONSTBIT | BYTE  | PTRBIT },
	{ "xtick",FLOAT },
	{ "nxsub",INT },
	{ "yopt",CONSTBIT | BYTE  | PTRBIT },
	{ "ytick",FLOAT },
	{ "nysub",INT } };
void
c_plaxes_wrapper(int ac,APIARGS *av){
	c_plaxes(*((float *)av[1].argval),	/* float x0 */
		*((float *)av[2].argval),	/* float y0 */
		(char *)av[3].argval,	/* char *xopt */
		*((float *)av[4].argval),	/* float xtick */
		*((int *)av[5].argval),	/* int nxsub */
		(char *)av[6].argval,	/* char *yopt */
		*((float *)av[7].argval),	/* float ytick */
		*((int *)av[8].argval));	/* int nysub */

}

APIARGS c_plbin_args[]={
	{ "c_plbin",VOID },
	{ "nbin",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "center",INT } };
void
c_plbin_wrapper(int ac,APIARGS *av){
	c_plbin(*((int *)av[1].argval),	/* int nbin */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		*((int *)av[4].argval));	/* int center */

}

APIARGS c_plbop_args[]={
	{ "c_plbop",VOID } };
void
c_plbop_wrapper(int ac,APIARGS *av){
	c_plbop();
}

APIARGS c_plbox_args[]={
	{ "c_plbox",VOID },
	{ "xopt",CONSTBIT | BYTE  | PTRBIT },
	{ "xtick",FLOAT },
	{ "nxsub",INT },
	{ "yopt",CONSTBIT | BYTE  | PTRBIT },
	{ "ytick",FLOAT },
	{ "nysub",INT } };
void
c_plbox_wrapper(int ac,APIARGS *av){
	c_plbox((char *)av[1].argval,	/* char *xopt */
		*((float *)av[2].argval),	/* float xtick */
		*((int *)av[3].argval),	/* int nxsub */
		(char *)av[4].argval,	/* char *yopt */
		*((float *)av[5].argval),	/* float ytick */
		*((int *)av[6].argval));	/* int nysub */

}

APIARGS c_plbox3_args[]={
	{ "c_plbox3",VOID },
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
c_plbox3_wrapper(int ac,APIARGS *av){
	c_plbox3((char *)av[1].argval,	/* char *xopt */
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

APIARGS c_plcol0_args[]={
	{ "c_plcol0",VOID },
	{ "icol0",INT } };
void
c_plcol0_wrapper(int ac,APIARGS *av){
	c_plcol0(*((int *)av[1].argval));	/* int icol0 */

}

APIARGS c_plcol1_args[]={
	{ "c_plcol1",VOID },
	{ "col1",FLOAT } };
void
c_plcol1_wrapper(int ac,APIARGS *av){
	c_plcol1(*((float *)av[1].argval));	/* float col1 */

}

APIARGS c_plcpstrm_args[]={
	{ "c_plcpstrm",VOID },
	{ "iplsr",INT },
	{ "flags",INT } };
void
c_plcpstrm_wrapper(int ac,APIARGS *av){
	c_plcpstrm(*((int *)av[1].argval),	/* int iplsr */
		*((int *)av[2].argval));	/* int flags */

}

APIARGS c_plend_args[]={
	{ "c_plend",VOID } };
void
c_plend_wrapper(int ac,APIARGS *av){
	c_plend();
}

APIARGS c_plend1_args[]={
	{ "c_plend1",VOID } };
void
c_plend1_wrapper(int ac,APIARGS *av){
	c_plend1();
}

APIARGS c_plenv_args[]={
	{ "c_plenv",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT },
	{ "just",INT },
	{ "axis",INT } };
void
c_plenv_wrapper(int ac,APIARGS *av){
	c_plenv(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float xmax */
		*((float *)av[3].argval),	/* float ymin */
		*((float *)av[4].argval),	/* float ymax */
		*((int *)av[5].argval),	/* int just */
		*((int *)av[6].argval));	/* int axis */

}

APIARGS c_pleop_args[]={
	{ "c_pleop",VOID } };
void
c_pleop_wrapper(int ac,APIARGS *av){
	c_pleop();
}

APIARGS c_plerrx_args[]={
	{ "c_plerrx",VOID },
	{ "n",INT },
	{ "xmin",FLOAT  | PTRBIT },
	{ "xmax",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT } };
void
c_plerrx_wrapper(int ac,APIARGS *av){
	c_plerrx(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *xmin */
		(float *)av[3].argval,	/* float *xmax */
		(float *)av[4].argval);	/* float *y */

}

APIARGS c_plerry_args[]={
	{ "c_plerry",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "ymin",FLOAT  | PTRBIT },
	{ "ymax",FLOAT  | PTRBIT } };
void
c_plerry_wrapper(int ac,APIARGS *av){
	c_plerry(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *ymin */
		(float *)av[4].argval);	/* float *ymax */

}

APIARGS c_plfamadv_args[]={
	{ "c_plfamadv",VOID } };
void
c_plfamadv_wrapper(int ac,APIARGS *av){
	c_plfamadv();
}

APIARGS c_plfill_args[]={
	{ "c_plfill",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT } };
void
c_plfill_wrapper(int ac,APIARGS *av){
	c_plfill(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval);	/* float *y */

}

APIARGS c_plflush_args[]={
	{ "c_plflush",VOID } };
void
c_plflush_wrapper(int ac,APIARGS *av){
	c_plflush();
}

APIARGS c_plfont_args[]={
	{ "c_plfont",VOID },
	{ "ifont",INT } };
void
c_plfont_wrapper(int ac,APIARGS *av){
	c_plfont(*((int *)av[1].argval));	/* int ifont */

}

APIARGS c_plfontld_args[]={
	{ "c_plfontld",VOID },
	{ "fnt",INT } };
void
c_plfontld_wrapper(int ac,APIARGS *av){
	c_plfontld(*((int *)av[1].argval));	/* int fnt */

}

APIARGS c_plgchr_args[]={
	{ "c_plgchr",VOID },
	{ "p_def",FLOAT  | PTRBIT },
	{ "p_ht",FLOAT  | PTRBIT } };
void
c_plgchr_wrapper(int ac,APIARGS *av){
	c_plgchr((float *)av[1].argval,	/* float *p_def */
		(float *)av[2].argval);	/* float *p_ht */

}

APIARGS c_plgcol0_args[]={
	{ "c_plgcol0",VOID },
	{ "icol0",INT },
	{ "r",INT  | PTRBIT },
	{ "g",INT  | PTRBIT },
	{ "b",INT  | PTRBIT } };
void
c_plgcol0_wrapper(int ac,APIARGS *av){
	c_plgcol0(*((int *)av[1].argval),	/* int icol0 */
		(int *)av[2].argval,	/* int *r */
		(int *)av[3].argval,	/* int *g */
		(int *)av[4].argval);	/* int *b */

}

APIARGS c_plgcolbg_args[]={
	{ "c_plgcolbg",VOID },
	{ "r",INT  | PTRBIT },
	{ "g",INT  | PTRBIT },
	{ "b",INT  | PTRBIT } };
void
c_plgcolbg_wrapper(int ac,APIARGS *av){
	c_plgcolbg((int *)av[1].argval,	/* int *r */
		(int *)av[2].argval,	/* int *g */
		(int *)av[3].argval);	/* int *b */

}

APIARGS c_plgdev_args[]={
	{ "c_plgdev",VOID },
	{ "p_dev",BYTE  | PTRBIT } };
void
c_plgdev_wrapper(int ac,APIARGS *av){
	c_plgdev((char *)av[1].argval);	/* char *p_dev */

}

APIARGS c_plgdidev_args[]={
	{ "c_plgdidev",VOID },
	{ "p_mar",FLOAT  | PTRBIT },
	{ "p_aspect",FLOAT  | PTRBIT },
	{ "p_jx",FLOAT  | PTRBIT },
	{ "p_jy",FLOAT  | PTRBIT } };
void
c_plgdidev_wrapper(int ac,APIARGS *av){
	c_plgdidev((float *)av[1].argval,	/* float *p_mar */
		(float *)av[2].argval,	/* float *p_aspect */
		(float *)av[3].argval,	/* float *p_jx */
		(float *)av[4].argval);	/* float *p_jy */

}

APIARGS c_plgdiori_args[]={
	{ "c_plgdiori",VOID },
	{ "p_rot",FLOAT  | PTRBIT } };
void
c_plgdiori_wrapper(int ac,APIARGS *av){
	c_plgdiori((float *)av[1].argval);	/* float *p_rot */

}

APIARGS c_plgdiplt_args[]={
	{ "c_plgdiplt",VOID },
	{ "p_xmin",FLOAT  | PTRBIT },
	{ "p_ymin",FLOAT  | PTRBIT },
	{ "p_xmax",FLOAT  | PTRBIT },
	{ "p_ymax",FLOAT  | PTRBIT } };
void
c_plgdiplt_wrapper(int ac,APIARGS *av){
	c_plgdiplt((float *)av[1].argval,	/* float *p_xmin */
		(float *)av[2].argval,	/* float *p_ymin */
		(float *)av[3].argval,	/* float *p_xmax */
		(float *)av[4].argval);	/* float *p_ymax */

}

APIARGS c_plgfam_args[]={
	{ "c_plgfam",VOID },
	{ "p_fam",INT  | PTRBIT },
	{ "p_num",INT  | PTRBIT },
	{ "p_bmax",INT  | PTRBIT } };
void
c_plgfam_wrapper(int ac,APIARGS *av){
	c_plgfam((int *)av[1].argval,	/* int *p_fam */
		(int *)av[2].argval,	/* int *p_num */
		(int *)av[3].argval);	/* int *p_bmax */

}

APIARGS c_plgfnam_args[]={
	{ "c_plgfnam",VOID },
	{ "fnam",BYTE  | PTRBIT } };
void
c_plgfnam_wrapper(int ac,APIARGS *av){
	c_plgfnam((char *)av[1].argval);	/* char *fnam */

}

APIARGS c_plglevel_args[]={
	{ "c_plglevel",VOID },
	{ "p_level",INT  | PTRBIT } };
void
c_plglevel_wrapper(int ac,APIARGS *av){
	c_plglevel((int *)av[1].argval);	/* int *p_level */

}

APIARGS c_plgpage_args[]={
	{ "c_plgpage",VOID },
	{ "p_xp",FLOAT  | PTRBIT },
	{ "p_yp",FLOAT  | PTRBIT },
	{ "p_xleng",INT  | PTRBIT },
	{ "p_yleng",INT  | PTRBIT },
	{ "p_xoff",INT  | PTRBIT },
	{ "p_yoff",INT  | PTRBIT } };
void
c_plgpage_wrapper(int ac,APIARGS *av){
	c_plgpage((float *)av[1].argval,	/* float *p_xp */
		(float *)av[2].argval,	/* float *p_yp */
		(int *)av[3].argval,	/* int *p_xleng */
		(int *)av[4].argval,	/* int *p_yleng */
		(int *)av[5].argval,	/* int *p_xoff */
		(int *)av[6].argval);	/* int *p_yoff */

}

APIARGS c_plgra_args[]={
	{ "c_plgra",VOID } };
void
c_plgra_wrapper(int ac,APIARGS *av){
	c_plgra();
}

APIARGS c_plgspa_args[]={
	{ "c_plgspa",VOID },
	{ "xmin",FLOAT  | PTRBIT },
	{ "xmax",FLOAT  | PTRBIT },
	{ "ymin",FLOAT  | PTRBIT },
	{ "ymax",FLOAT  | PTRBIT } };
void
c_plgspa_wrapper(int ac,APIARGS *av){
	c_plgspa((float *)av[1].argval,	/* float *xmin */
		(float *)av[2].argval,	/* float *xmax */
		(float *)av[3].argval,	/* float *ymin */
		(float *)av[4].argval);	/* float *ymax */

}

APIARGS c_plgstrm_args[]={
	{ "c_plgstrm",VOID },
	{ "p_strm",INT  | PTRBIT } };
void
c_plgstrm_wrapper(int ac,APIARGS *av){
	c_plgstrm((int *)av[1].argval);	/* int *p_strm */

}

APIARGS c_plgver_args[]={
	{ "c_plgver",VOID },
	{ "p_ver",BYTE  | PTRBIT } };
void
c_plgver_wrapper(int ac,APIARGS *av){
	c_plgver((char *)av[1].argval);	/* char *p_ver */

}

APIARGS c_plgxax_args[]={
	{ "c_plgxax",VOID },
	{ "p_digmax",INT  | PTRBIT },
	{ "p_digits",INT  | PTRBIT } };
void
c_plgxax_wrapper(int ac,APIARGS *av){
	c_plgxax((int *)av[1].argval,	/* int *p_digmax */
		(int *)av[2].argval);	/* int *p_digits */

}

APIARGS c_plgyax_args[]={
	{ "c_plgyax",VOID },
	{ "p_digmax",INT  | PTRBIT },
	{ "p_digits",INT  | PTRBIT } };
void
c_plgyax_wrapper(int ac,APIARGS *av){
	c_plgyax((int *)av[1].argval,	/* int *p_digmax */
		(int *)av[2].argval);	/* int *p_digits */

}

APIARGS c_plgzax_args[]={
	{ "c_plgzax",VOID },
	{ "p_digmax",INT  | PTRBIT },
	{ "p_digits",INT  | PTRBIT } };
void
c_plgzax_wrapper(int ac,APIARGS *av){
	c_plgzax((int *)av[1].argval,	/* int *p_digmax */
		(int *)av[2].argval);	/* int *p_digits */

}

APIARGS c_plhist_args[]={
	{ "c_plhist",VOID },
	{ "n",INT },
	{ "data",FLOAT  | PTRBIT },
	{ "datmin",FLOAT },
	{ "datmax",FLOAT },
	{ "nbin",INT },
	{ "oldwin",INT } };
void
c_plhist_wrapper(int ac,APIARGS *av){
	c_plhist(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *data */
		*((float *)av[3].argval),	/* float datmin */
		*((float *)av[4].argval),	/* float datmax */
		*((int *)av[5].argval),	/* int nbin */
		*((int *)av[6].argval));	/* int oldwin */

}

APIARGS c_plhls_args[]={
	{ "c_plhls",VOID },
	{ "h",FLOAT },
	{ "l",FLOAT },
	{ "s",FLOAT } };
void
c_plhls_wrapper(int ac,APIARGS *av){
	c_plhls(*((float *)av[1].argval),	/* float h */
		*((float *)av[2].argval),	/* float l */
		*((float *)av[3].argval));	/* float s */

}

APIARGS c_plinit_args[]={
	{ "c_plinit",VOID } };
void
c_plinit_wrapper(int ac,APIARGS *av){
	c_plinit();
}

APIARGS c_pljoin_args[]={
	{ "c_pljoin",VOID },
	{ "x1",FLOAT },
	{ "y1",FLOAT },
	{ "x2",FLOAT },
	{ "y2",FLOAT } };
void
c_pljoin_wrapper(int ac,APIARGS *av){
	c_pljoin(*((float *)av[1].argval),	/* float x1 */
		*((float *)av[2].argval),	/* float y1 */
		*((float *)av[3].argval),	/* float x2 */
		*((float *)av[4].argval));	/* float y2 */

}

APIARGS c_pllab_args[]={
	{ "c_pllab",VOID },
	{ "xlabel",CONSTBIT | BYTE  | PTRBIT },
	{ "ylabel",CONSTBIT | BYTE  | PTRBIT },
	{ "tlabel",CONSTBIT | BYTE  | PTRBIT } };
void
c_pllab_wrapper(int ac,APIARGS *av){
	c_pllab((char *)av[1].argval,	/* char *xlabel */
		(char *)av[2].argval,	/* char *ylabel */
		(char *)av[3].argval);	/* char *tlabel */

}

APIARGS c_plline_args[]={
	{ "c_plline",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT } };
void
c_plline_wrapper(int ac,APIARGS *av){
	c_plline(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval);	/* float *y */

}

APIARGS c_plline3_args[]={
	{ "c_plline3",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "z",FLOAT  | PTRBIT } };
void
c_plline3_wrapper(int ac,APIARGS *av){
	c_plline3(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		(float *)av[4].argval);	/* float *z */

}

APIARGS c_pllsty_args[]={
	{ "c_pllsty",VOID },
	{ "lin",INT } };
void
c_pllsty_wrapper(int ac,APIARGS *av){
	c_pllsty(*((int *)av[1].argval));	/* int lin */

}

APIARGS c_plmkstrm_args[]={
	{ "c_plmkstrm",VOID },
	{ "p_strm",INT  | PTRBIT } };
void
c_plmkstrm_wrapper(int ac,APIARGS *av){
	c_plmkstrm((int *)av[1].argval);	/* int *p_strm */

}

APIARGS c_plmtex_args[]={
	{ "c_plmtex",VOID },
	{ "side",CONSTBIT | BYTE  | PTRBIT },
	{ "disp",FLOAT },
	{ "pos",FLOAT },
	{ "just",FLOAT },
	{ "text",CONSTBIT | BYTE  | PTRBIT } };
void
c_plmtex_wrapper(int ac,APIARGS *av){
	c_plmtex((char *)av[1].argval,	/* char *side */
		*((float *)av[2].argval),	/* float disp */
		*((float *)av[3].argval),	/* float pos */
		*((float *)av[4].argval),	/* float just */
		(char *)av[5].argval);	/* char *text */

}

APIARGS c_plpat_args[]={
	{ "c_plpat",VOID },
	{ "nlin",INT },
	{ "inc",INT  | PTRBIT },
	{ "del",INT  | PTRBIT } };
void
c_plpat_wrapper(int ac,APIARGS *av){
	c_plpat(*((int *)av[1].argval),	/* int nlin */
		(int *)av[2].argval,	/* int *inc */
		(int *)av[3].argval);	/* int *del */

}

APIARGS c_plpoin_args[]={
	{ "c_plpoin",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "code",INT } };
void
c_plpoin_wrapper(int ac,APIARGS *av){
	c_plpoin(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		*((int *)av[4].argval));	/* int code */

}

APIARGS c_plpoin3_args[]={
	{ "c_plpoin3",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "z",FLOAT  | PTRBIT },
	{ "code",INT } };
void
c_plpoin3_wrapper(int ac,APIARGS *av){
	c_plpoin3(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		(float *)av[4].argval,	/* float *z */
		*((int *)av[5].argval));	/* int code */

}

APIARGS c_plpoly3_args[]={
	{ "c_plpoly3",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "z",FLOAT  | PTRBIT },
	{ "draw",INT  | PTRBIT } };
void
c_plpoly3_wrapper(int ac,APIARGS *av){
	c_plpoly3(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		(float *)av[4].argval,	/* float *z */
		(int *)av[5].argval);	/* int *draw */

}

APIARGS c_plprec_args[]={
	{ "c_plprec",VOID },
	{ "setp",INT },
	{ "prec",INT } };
void
c_plprec_wrapper(int ac,APIARGS *av){
	c_plprec(*((int *)av[1].argval),	/* int setp */
		*((int *)av[2].argval));	/* int prec */

}

APIARGS c_plpsty_args[]={
	{ "c_plpsty",VOID },
	{ "patt",INT } };
void
c_plpsty_wrapper(int ac,APIARGS *av){
	c_plpsty(*((int *)av[1].argval));	/* int patt */

}

APIARGS c_plptex_args[]={
	{ "c_plptex",VOID },
	{ "x",FLOAT },
	{ "y",FLOAT },
	{ "dx",FLOAT },
	{ "dy",FLOAT },
	{ "just",FLOAT },
	{ "text",CONSTBIT | BYTE  | PTRBIT } };
void
c_plptex_wrapper(int ac,APIARGS *av){
	c_plptex(*((float *)av[1].argval),	/* float x */
		*((float *)av[2].argval),	/* float y */
		*((float *)av[3].argval),	/* float dx */
		*((float *)av[4].argval),	/* float dy */
		*((float *)av[5].argval),	/* float just */
		(char *)av[6].argval);	/* char *text */

}

APIARGS c_plreplot_args[]={
	{ "c_plreplot",VOID } };
void
c_plreplot_wrapper(int ac,APIARGS *av){
	c_plreplot();
}

APIARGS c_plrgb_args[]={
	{ "c_plrgb",VOID },
	{ "r",FLOAT },
	{ "g",FLOAT },
	{ "b",FLOAT } };
void
c_plrgb_wrapper(int ac,APIARGS *av){
	c_plrgb(*((float *)av[1].argval),	/* float r */
		*((float *)av[2].argval),	/* float g */
		*((float *)av[3].argval));	/* float b */

}

APIARGS c_plrgb1_args[]={
	{ "c_plrgb1",VOID },
	{ "r",INT },
	{ "g",INT },
	{ "b",INT } };
void
c_plrgb1_wrapper(int ac,APIARGS *av){
	c_plrgb1(*((int *)av[1].argval),	/* int r */
		*((int *)av[2].argval),	/* int g */
		*((int *)av[3].argval));	/* int b */

}

APIARGS c_plschr_args[]={
	{ "c_plschr",VOID },
	{ "def",FLOAT },
	{ "scale",FLOAT } };
void
c_plschr_wrapper(int ac,APIARGS *av){
	c_plschr(*((float *)av[1].argval),	/* float def */
		*((float *)av[2].argval));	/* float scale */

}

APIARGS c_plscmap0_args[]={
	{ "c_plscmap0",VOID },
	{ "r",INT  | PTRBIT },
	{ "g",INT  | PTRBIT },
	{ "b",INT  | PTRBIT },
	{ "ncol0",INT } };
void
c_plscmap0_wrapper(int ac,APIARGS *av){
	c_plscmap0((int *)av[1].argval,	/* int *r */
		(int *)av[2].argval,	/* int *g */
		(int *)av[3].argval,	/* int *b */
		*((int *)av[4].argval));	/* int ncol0 */

}

APIARGS c_plscmap0n_args[]={
	{ "c_plscmap0n",VOID },
	{ "ncol0",INT } };
void
c_plscmap0n_wrapper(int ac,APIARGS *av){
	c_plscmap0n(*((int *)av[1].argval));	/* int ncol0 */

}

APIARGS c_plscmap1_args[]={
	{ "c_plscmap1",VOID },
	{ "r",INT  | PTRBIT },
	{ "g",INT  | PTRBIT },
	{ "b",INT  | PTRBIT },
	{ "ncol1",INT } };
void
c_plscmap1_wrapper(int ac,APIARGS *av){
	c_plscmap1((int *)av[1].argval,	/* int *r */
		(int *)av[2].argval,	/* int *g */
		(int *)av[3].argval,	/* int *b */
		*((int *)av[4].argval));	/* int ncol1 */

}

APIARGS c_plscmap1l_args[]={
	{ "c_plscmap1l",VOID },
	{ "itype",INT },
	{ "npts",INT },
	{ "intensity",FLOAT  | PTRBIT },
	{ "coord1",FLOAT  | PTRBIT },
	{ "coord2",FLOAT  | PTRBIT },
	{ "coord3",FLOAT  | PTRBIT },
	{ "rev",INT  | PTRBIT } };
void
c_plscmap1l_wrapper(int ac,APIARGS *av){
	c_plscmap1l(*((int *)av[1].argval),	/* int itype */
		*((int *)av[2].argval),	/* int npts */
		(float *)av[3].argval,	/* float *intensity */
		(float *)av[4].argval,	/* float *coord1 */
		(float *)av[5].argval,	/* float *coord2 */
		(float *)av[6].argval,	/* float *coord3 */
		(int *)av[7].argval);	/* int *rev */

}

APIARGS c_plscmap1n_args[]={
	{ "c_plscmap1n",VOID },
	{ "ncol1",INT } };
void
c_plscmap1n_wrapper(int ac,APIARGS *av){
	c_plscmap1n(*((int *)av[1].argval));	/* int ncol1 */

}

APIARGS c_plscol0_args[]={
	{ "c_plscol0",VOID },
	{ "icol0",INT },
	{ "r",INT },
	{ "g",INT },
	{ "b",INT } };
void
c_plscol0_wrapper(int ac,APIARGS *av){
	c_plscol0(*((int *)av[1].argval),	/* int icol0 */
		*((int *)av[2].argval),	/* int r */
		*((int *)av[3].argval),	/* int g */
		*((int *)av[4].argval));	/* int b */

}

APIARGS c_plscolbg_args[]={
	{ "c_plscolbg",VOID },
	{ "r",INT },
	{ "g",INT },
	{ "b",INT } };
void
c_plscolbg_wrapper(int ac,APIARGS *av){
	c_plscolbg(*((int *)av[1].argval),	/* int r */
		*((int *)av[2].argval),	/* int g */
		*((int *)av[3].argval));	/* int b */

}

APIARGS c_plscolor_args[]={
	{ "c_plscolor",VOID },
	{ "color",INT } };
void
c_plscolor_wrapper(int ac,APIARGS *av){
	c_plscolor(*((int *)av[1].argval));	/* int color */

}

APIARGS c_plsdev_args[]={
	{ "c_plsdev",VOID },
	{ "devname",CONSTBIT | BYTE  | PTRBIT } };
void
c_plsdev_wrapper(int ac,APIARGS *av){
	c_plsdev((char *)av[1].argval);	/* char *devname */

}

APIARGS c_plsdidev_args[]={
	{ "c_plsdidev",VOID },
	{ "mar",FLOAT },
	{ "aspect",FLOAT },
	{ "jx",FLOAT },
	{ "jy",FLOAT } };
void
c_plsdidev_wrapper(int ac,APIARGS *av){
	c_plsdidev(*((float *)av[1].argval),	/* float mar */
		*((float *)av[2].argval),	/* float aspect */
		*((float *)av[3].argval),	/* float jx */
		*((float *)av[4].argval));	/* float jy */

}

APIARGS c_plsdimap_args[]={
	{ "c_plsdimap",VOID },
	{ "dimxmin",INT },
	{ "dimxmax",INT },
	{ "dimymin",INT },
	{ "dimymax",INT },
	{ "dimxpmm",FLOAT },
	{ "dimypmm",FLOAT } };
void
c_plsdimap_wrapper(int ac,APIARGS *av){
	c_plsdimap(*((int *)av[1].argval),	/* int dimxmin */
		*((int *)av[2].argval),	/* int dimxmax */
		*((int *)av[3].argval),	/* int dimymin */
		*((int *)av[4].argval),	/* int dimymax */
		*((float *)av[5].argval),	/* float dimxpmm */
		*((float *)av[6].argval));	/* float dimypmm */

}

APIARGS c_plsdiori_args[]={
	{ "c_plsdiori",VOID },
	{ "rot",FLOAT } };
void
c_plsdiori_wrapper(int ac,APIARGS *av){
	c_plsdiori(*((float *)av[1].argval));	/* float rot */

}

APIARGS c_plsdiplt_args[]={
	{ "c_plsdiplt",VOID },
	{ "xmin",FLOAT },
	{ "ymin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymax",FLOAT } };
void
c_plsdiplt_wrapper(int ac,APIARGS *av){
	c_plsdiplt(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float ymin */
		*((float *)av[3].argval),	/* float xmax */
		*((float *)av[4].argval));	/* float ymax */

}

APIARGS c_plsdiplz_args[]={
	{ "c_plsdiplz",VOID },
	{ "xmin",FLOAT },
	{ "ymin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymax",FLOAT } };
void
c_plsdiplz_wrapper(int ac,APIARGS *av){
	c_plsdiplz(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float ymin */
		*((float *)av[3].argval),	/* float xmax */
		*((float *)av[4].argval));	/* float ymax */

}

APIARGS c_plsesc_args[]={
	{ "c_plsesc",VOID },
	{ "esc",BYTE } };
void
c_plsesc_wrapper(int ac,APIARGS *av){
	c_plsesc(*((char *)av[1].argval));	/* char esc */

}

APIARGS c_plsfam_args[]={
	{ "c_plsfam",VOID },
	{ "fam",INT },
	{ "num",INT },
	{ "bmax",INT } };
void
c_plsfam_wrapper(int ac,APIARGS *av){
	c_plsfam(*((int *)av[1].argval),	/* int fam */
		*((int *)av[2].argval),	/* int num */
		*((int *)av[3].argval));	/* int bmax */

}

APIARGS c_plsfnam_args[]={
	{ "c_plsfnam",VOID },
	{ "fnam",CONSTBIT | BYTE  | PTRBIT } };
void
c_plsfnam_wrapper(int ac,APIARGS *av){
	c_plsfnam((char *)av[1].argval);	/* char *fnam */

}

APIARGS c_plsmaj_args[]={
	{ "c_plsmaj",VOID },
	{ "def",FLOAT },
	{ "scale",FLOAT } };
void
c_plsmaj_wrapper(int ac,APIARGS *av){
	c_plsmaj(*((float *)av[1].argval),	/* float def */
		*((float *)av[2].argval));	/* float scale */

}

APIARGS c_plsmin_args[]={
	{ "c_plsmin",VOID },
	{ "def",FLOAT },
	{ "scale",FLOAT } };
void
c_plsmin_wrapper(int ac,APIARGS *av){
	c_plsmin(*((float *)av[1].argval),	/* float def */
		*((float *)av[2].argval));	/* float scale */

}

APIARGS c_plsori_args[]={
	{ "c_plsori",VOID },
	{ "ori",INT } };
void
c_plsori_wrapper(int ac,APIARGS *av){
	c_plsori(*((int *)av[1].argval));	/* int ori */

}

APIARGS c_plspage_args[]={
	{ "c_plspage",VOID },
	{ "xp",FLOAT },
	{ "yp",FLOAT },
	{ "xleng",INT },
	{ "yleng",INT },
	{ "xoff",INT },
	{ "yoff",INT } };
void
c_plspage_wrapper(int ac,APIARGS *av){
	c_plspage(*((float *)av[1].argval),	/* float xp */
		*((float *)av[2].argval),	/* float yp */
		*((int *)av[3].argval),	/* int xleng */
		*((int *)av[4].argval),	/* int yleng */
		*((int *)av[5].argval),	/* int xoff */
		*((int *)av[6].argval));	/* int yoff */

}

APIARGS c_plspause_args[]={
	{ "c_plspause",VOID },
	{ "pause",INT } };
void
c_plspause_wrapper(int ac,APIARGS *av){
	c_plspause(*((int *)av[1].argval));	/* int pause */

}

APIARGS c_plsstrm_args[]={
	{ "c_plsstrm",VOID },
	{ "strm",INT } };
void
c_plsstrm_wrapper(int ac,APIARGS *av){
	c_plsstrm(*((int *)av[1].argval));	/* int strm */

}

APIARGS c_plssub_args[]={
	{ "c_plssub",VOID },
	{ "nx",INT },
	{ "ny",INT } };
void
c_plssub_wrapper(int ac,APIARGS *av){
	c_plssub(*((int *)av[1].argval),	/* int nx */
		*((int *)av[2].argval));	/* int ny */

}

APIARGS c_plssym_args[]={
	{ "c_plssym",VOID },
	{ "def",FLOAT },
	{ "scale",FLOAT } };
void
c_plssym_wrapper(int ac,APIARGS *av){
	c_plssym(*((float *)av[1].argval),	/* float def */
		*((float *)av[2].argval));	/* float scale */

}

APIARGS c_plstar_args[]={
	{ "c_plstar",VOID },
	{ "nx",INT },
	{ "ny",INT } };
void
c_plstar_wrapper(int ac,APIARGS *av){
	c_plstar(*((int *)av[1].argval),	/* int nx */
		*((int *)av[2].argval));	/* int ny */

}

APIARGS c_plstart_args[]={
	{ "c_plstart",VOID },
	{ "devname",CONSTBIT | BYTE  | PTRBIT },
	{ "nx",INT },
	{ "ny",INT } };
void
c_plstart_wrapper(int ac,APIARGS *av){
	c_plstart((char *)av[1].argval,	/* char *devname */
		*((int *)av[2].argval),	/* int nx */
		*((int *)av[3].argval));	/* int ny */

}

APIARGS c_plstyl_args[]={
	{ "c_plstyl",VOID },
	{ "nms",INT },
	{ "mark",INT  | PTRBIT },
	{ "space",INT  | PTRBIT } };
void
c_plstyl_wrapper(int ac,APIARGS *av){
	c_plstyl(*((int *)av[1].argval),	/* int nms */
		(int *)av[2].argval,	/* int *mark */
		(int *)av[3].argval);	/* int *space */

}

APIARGS c_plsvpa_args[]={
	{ "c_plsvpa",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT } };
void
c_plsvpa_wrapper(int ac,APIARGS *av){
	c_plsvpa(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float xmax */
		*((float *)av[3].argval),	/* float ymin */
		*((float *)av[4].argval));	/* float ymax */

}

APIARGS c_plsxax_args[]={
	{ "c_plsxax",VOID },
	{ "digmax",INT },
	{ "digits",INT } };
void
c_plsxax_wrapper(int ac,APIARGS *av){
	c_plsxax(*((int *)av[1].argval),	/* int digmax */
		*((int *)av[2].argval));	/* int digits */

}

APIARGS c_plsyax_args[]={
	{ "c_plsyax",VOID },
	{ "digmax",INT },
	{ "digits",INT } };
void
c_plsyax_wrapper(int ac,APIARGS *av){
	c_plsyax(*((int *)av[1].argval),	/* int digmax */
		*((int *)av[2].argval));	/* int digits */

}

APIARGS c_plsym_args[]={
	{ "c_plsym",VOID },
	{ "n",INT },
	{ "x",FLOAT  | PTRBIT },
	{ "y",FLOAT  | PTRBIT },
	{ "code",INT } };
void
c_plsym_wrapper(int ac,APIARGS *av){
	c_plsym(*((int *)av[1].argval),	/* int n */
		(float *)av[2].argval,	/* float *x */
		(float *)av[3].argval,	/* float *y */
		*((int *)av[4].argval));	/* int code */

}

APIARGS c_plszax_args[]={
	{ "c_plszax",VOID },
	{ "digmax",INT },
	{ "digits",INT } };
void
c_plszax_wrapper(int ac,APIARGS *av){
	c_plszax(*((int *)av[1].argval),	/* int digmax */
		*((int *)av[2].argval));	/* int digits */

}

APIARGS c_pltext_args[]={
	{ "c_pltext",VOID } };
void
c_pltext_wrapper(int ac,APIARGS *av){
	c_pltext();
}

APIARGS c_plvasp_args[]={
	{ "c_plvasp",VOID },
	{ "aspect",FLOAT } };
void
c_plvasp_wrapper(int ac,APIARGS *av){
	c_plvasp(*((float *)av[1].argval));	/* float aspect */

}

APIARGS c_plvpas_args[]={
	{ "c_plvpas",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT },
	{ "aspect",FLOAT } };
void
c_plvpas_wrapper(int ac,APIARGS *av){
	c_plvpas(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float xmax */
		*((float *)av[3].argval),	/* float ymin */
		*((float *)av[4].argval),	/* float ymax */
		*((float *)av[5].argval));	/* float aspect */

}

APIARGS c_plvpor_args[]={
	{ "c_plvpor",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT } };
void
c_plvpor_wrapper(int ac,APIARGS *av){
	c_plvpor(*((float *)av[1].argval),	/* float xmin */
		*((float *)av[2].argval),	/* float xmax */
		*((float *)av[3].argval),	/* float ymin */
		*((float *)av[4].argval));	/* float ymax */

}

APIARGS c_plvsta_args[]={
	{ "c_plvsta",VOID } };
void
c_plvsta_wrapper(int ac,APIARGS *av){
	c_plvsta();
}

APIARGS c_plw3d_args[]={
	{ "c_plw3d",VOID },
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
c_plw3d_wrapper(int ac,APIARGS *av){
	c_plw3d(*((float *)av[1].argval),	/* float basex */
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

APIARGS c_plwid_args[]={
	{ "c_plwid",VOID },
	{ "width",INT } };
void
c_plwid_wrapper(int ac,APIARGS *av){
	c_plwid(*((int *)av[1].argval));	/* int width */

}

APIARGS c_plwind_args[]={
	{ "c_plwind",VOID },
	{ "xmin",FLOAT },
	{ "xmax",FLOAT },
	{ "ymin",FLOAT },
	{ "ymax",FLOAT } };
void
c_plwind_wrapper(int ac,APIARGS *av){
	c_plwind(*((float *)av[1].argval),	/* float xmin */
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

/* APIDEFS */
const APIDEFS apidefs[] = {
	{ "c_pladv",c_pladv_args,2,c_pladv_wrapper },
	{ "c_plaxes",c_plaxes_args,9,c_plaxes_wrapper },
	{ "c_plbin",c_plbin_args,5,c_plbin_wrapper },
	{ "c_plbop",c_plbop_args,1,c_plbop_wrapper },
	{ "c_plbox",c_plbox_args,7,c_plbox_wrapper },
	{ "c_plbox3",c_plbox3_args,13,c_plbox3_wrapper },
	{ "c_plcol0",c_plcol0_args,2,c_plcol0_wrapper },
	{ "c_plcol1",c_plcol1_args,2,c_plcol1_wrapper },
	{ "c_plcpstrm",c_plcpstrm_args,3,c_plcpstrm_wrapper },
	{ "c_plend",c_plend_args,1,c_plend_wrapper },
	{ "c_plend1",c_plend1_args,1,c_plend1_wrapper },
	{ "c_plenv",c_plenv_args,7,c_plenv_wrapper },
	{ "c_pleop",c_pleop_args,1,c_pleop_wrapper },
	{ "c_plerrx",c_plerrx_args,5,c_plerrx_wrapper },
	{ "c_plerry",c_plerry_args,5,c_plerry_wrapper },
	{ "c_plfamadv",c_plfamadv_args,1,c_plfamadv_wrapper },
	{ "c_plfill",c_plfill_args,4,c_plfill_wrapper },
	{ "c_plflush",c_plflush_args,1,c_plflush_wrapper },
	{ "c_plfont",c_plfont_args,2,c_plfont_wrapper },
	{ "c_plfontld",c_plfontld_args,2,c_plfontld_wrapper },
	{ "c_plgchr",c_plgchr_args,3,c_plgchr_wrapper },
	{ "c_plgcol0",c_plgcol0_args,5,c_plgcol0_wrapper },
	{ "c_plgcolbg",c_plgcolbg_args,4,c_plgcolbg_wrapper },
	{ "c_plgdev",c_plgdev_args,2,c_plgdev_wrapper },
	{ "c_plgdidev",c_plgdidev_args,5,c_plgdidev_wrapper },
	{ "c_plgdiori",c_plgdiori_args,2,c_plgdiori_wrapper },
	{ "c_plgdiplt",c_plgdiplt_args,5,c_plgdiplt_wrapper },
	{ "c_plgfam",c_plgfam_args,4,c_plgfam_wrapper },
	{ "c_plgfnam",c_plgfnam_args,2,c_plgfnam_wrapper },
	{ "c_plglevel",c_plglevel_args,2,c_plglevel_wrapper },
	{ "c_plgpage",c_plgpage_args,7,c_plgpage_wrapper },
	{ "c_plgra",c_plgra_args,1,c_plgra_wrapper },
	{ "c_plgspa",c_plgspa_args,5,c_plgspa_wrapper },
	{ "c_plgstrm",c_plgstrm_args,2,c_plgstrm_wrapper },
	{ "c_plgver",c_plgver_args,2,c_plgver_wrapper },
	{ "c_plgxax",c_plgxax_args,3,c_plgxax_wrapper },
	{ "c_plgyax",c_plgyax_args,3,c_plgyax_wrapper },
	{ "c_plgzax",c_plgzax_args,3,c_plgzax_wrapper },
	{ "c_plhist",c_plhist_args,7,c_plhist_wrapper },
	{ "c_plhls",c_plhls_args,4,c_plhls_wrapper },
	{ "c_plinit",c_plinit_args,1,c_plinit_wrapper },
	{ "c_pljoin",c_pljoin_args,5,c_pljoin_wrapper },
	{ "c_pllab",c_pllab_args,4,c_pllab_wrapper },
	{ "c_plline",c_plline_args,4,c_plline_wrapper },
	{ "c_plline3",c_plline3_args,5,c_plline3_wrapper },
	{ "c_pllsty",c_pllsty_args,2,c_pllsty_wrapper },
	{ "c_plmkstrm",c_plmkstrm_args,2,c_plmkstrm_wrapper },
	{ "c_plmtex",c_plmtex_args,6,c_plmtex_wrapper },
	{ "c_plpat",c_plpat_args,4,c_plpat_wrapper },
	{ "c_plpoin",c_plpoin_args,5,c_plpoin_wrapper },
	{ "c_plpoin3",c_plpoin3_args,6,c_plpoin3_wrapper },
	{ "c_plpoly3",c_plpoly3_args,6,c_plpoly3_wrapper },
	{ "c_plprec",c_plprec_args,3,c_plprec_wrapper },
	{ "c_plpsty",c_plpsty_args,2,c_plpsty_wrapper },
	{ "c_plptex",c_plptex_args,7,c_plptex_wrapper },
	{ "c_plreplot",c_plreplot_args,1,c_plreplot_wrapper },
	{ "c_plrgb",c_plrgb_args,4,c_plrgb_wrapper },
	{ "c_plrgb1",c_plrgb1_args,4,c_plrgb1_wrapper },
	{ "c_plschr",c_plschr_args,3,c_plschr_wrapper },
	{ "c_plscmap0",c_plscmap0_args,5,c_plscmap0_wrapper },
	{ "c_plscmap0n",c_plscmap0n_args,2,c_plscmap0n_wrapper },
	{ "c_plscmap1",c_plscmap1_args,5,c_plscmap1_wrapper },
	{ "c_plscmap1l",c_plscmap1l_args,8,c_plscmap1l_wrapper },
	{ "c_plscmap1n",c_plscmap1n_args,2,c_plscmap1n_wrapper },
	{ "c_plscol0",c_plscol0_args,5,c_plscol0_wrapper },
	{ "c_plscolbg",c_plscolbg_args,4,c_plscolbg_wrapper },
	{ "c_plscolor",c_plscolor_args,2,c_plscolor_wrapper },
	{ "c_plsdev",c_plsdev_args,2,c_plsdev_wrapper },
	{ "c_plsdidev",c_plsdidev_args,5,c_plsdidev_wrapper },
	{ "c_plsdimap",c_plsdimap_args,7,c_plsdimap_wrapper },
	{ "c_plsdiori",c_plsdiori_args,2,c_plsdiori_wrapper },
	{ "c_plsdiplt",c_plsdiplt_args,5,c_plsdiplt_wrapper },
	{ "c_plsdiplz",c_plsdiplz_args,5,c_plsdiplz_wrapper },
	{ "c_plsesc",c_plsesc_args,2,c_plsesc_wrapper },
	{ "c_plsfam",c_plsfam_args,4,c_plsfam_wrapper },
	{ "c_plsfnam",c_plsfnam_args,2,c_plsfnam_wrapper },
	{ "c_plsmaj",c_plsmaj_args,3,c_plsmaj_wrapper },
	{ "c_plsmin",c_plsmin_args,3,c_plsmin_wrapper },
	{ "c_plsori",c_plsori_args,2,c_plsori_wrapper },
	{ "c_plspage",c_plspage_args,7,c_plspage_wrapper },
	{ "c_plspause",c_plspause_args,2,c_plspause_wrapper },
	{ "c_plsstrm",c_plsstrm_args,2,c_plsstrm_wrapper },
	{ "c_plssub",c_plssub_args,3,c_plssub_wrapper },
	{ "c_plssym",c_plssym_args,3,c_plssym_wrapper },
	{ "c_plstar",c_plstar_args,3,c_plstar_wrapper },
	{ "c_plstart",c_plstart_args,4,c_plstart_wrapper },
	{ "c_plstyl",c_plstyl_args,4,c_plstyl_wrapper },
	{ "c_plsvpa",c_plsvpa_args,5,c_plsvpa_wrapper },
	{ "c_plsxax",c_plsxax_args,3,c_plsxax_wrapper },
	{ "c_plsyax",c_plsyax_args,3,c_plsyax_wrapper },
	{ "c_plsym",c_plsym_args,5,c_plsym_wrapper },
	{ "c_plszax",c_plszax_args,3,c_plszax_wrapper },
	{ "c_pltext",c_pltext_args,1,c_pltext_wrapper },
	{ "c_plvasp",c_plvasp_args,2,c_plvasp_wrapper },
	{ "c_plvpas",c_plvpas_args,6,c_plvpas_wrapper },
	{ "c_plvpor",c_plvpor_args,5,c_plvpor_wrapper },
	{ "c_plvsta",c_plvsta_args,1,c_plvsta_wrapper },
	{ "c_plw3d",c_plw3d_args,12,c_plw3d_wrapper },
	{ "c_plwid",c_plwid_args,2,c_plwid_wrapper },
	{ "c_plwind",c_plwind_args,5,c_plwind_wrapper },
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
};

#ifdef __cplusplus
}
#endif
