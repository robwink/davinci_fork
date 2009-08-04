#include "parser.h"
#ifdef INCLUDE_API
#include "api_extern_defs.h"
#include <limits.h>
#define SCALECONST	25.0

static int vp[100]={0};
static int plRow[100]={0};
static int plCol[100]={0};

void ptext_border(float,int,int,float,float,char *,int);

Var *
ff_popen(vfuncptr func, Var * arg)
{

	int Row=1,Col=1,Xpos=200,Ypos=200,Xpixels=512,Ypixels=512;
	char *Title=NULL;
	char nTitle[200];
	int Portrait=0;

	int *Stream=(int *)calloc(1,sizeof(int));
	int r,g,b;
	int c1,c2;

	int ac;
	Var **av;
	Alist alist[9];
	alist[0] = make_alist("row",		INT,	NULL,   &Row);
	alist[1] = make_alist("col",		INT,	NULL,	&Col);
	alist[2] = make_alist("xpos",		INT, 	NULL,	&Xpos);
	alist[3] = make_alist("ypos",		INT, 	NULL,	&Ypos);
	alist[4] = make_alist("xpixels",		INT, 	NULL,	&Xpixels);
	alist[5] = make_alist("ypixels",		INT, 	NULL,	&Ypixels);
	alist[6] = make_alist("title",		ID_STRING, 	NULL,	&Title);
	alist[7] = make_alist("portrait",		INT, 	NULL,	&Portrait);
	alist[8].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (Title==NULL){
		Title=strdup("Plot Window");
	}

	pl_plmkstrm(Stream);	

	sprintf(nTitle,"%s-WindowID:%d",Title,*Stream);

	pl_plsplw(nTitle);

	pl_plssub(Col,Row);

	plRow[*Stream]=Row;
	plCol[*Stream]=Col;

	pl_plsdev("xtwin");
	pl_plspage(0,0,Xpixels,Ypixels,Xpos,Ypos);

	pl_plscol0(0,255,255,255);
	pl_plscol0(15,0,0,0);

	if (Portrait){
		pl_plsdiori(1.0);
	}

	pl_plinit();

	pl_plcol0(15);

	return (newVal(BSQ,1,1,1,INT,(Stream)));
}


Var *
ff_pprint(vfuncptr func, Var * arg)
{

	char *Title=NULL;
	int Stream=0;
	int Dummy;
	int r,g,b;
	char *Output=NULL;
	int Translate=0;

	char *output[]={"ps","psc","pbm",NULL};

	int ac;
	Var **av;
	Alist alist[4];
	alist[0] = make_alist("id",		INT,	NULL,   &Stream);
	alist[1] = make_alist("title",		ID_STRING, 	NULL,	&Title);
	alist[2] = make_alist("type",		ID_ENUM, 	output,	&Output);
	alist[3].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (Title==NULL){
		Title=strdup("MyPlot.ps");
	}

	if (Output==NULL){
		Output=strdup("ps");
	}

	if (Stream < 1){
		parse_error("%s: Valid window ID: %d\n", func->name,Stream);	
		return(NULL);
	}


	pl_plmkstrm(&Dummy);
	pl_plsdev(Output);
	pl_plsfnam(Title);
/*	pl_plsstrm(Stream); */
/*	pl_plgcolbg(&r,&g,&b); */
/*	pl_plsstrm(Dummy); */
	pl_plscolbg(255,255,255);/*White background for printing*/
	pl_plcpstrm(Stream,0);
	pl_plreplot();
	pl_plend1();
	pl_plsstrm(Stream);
/*	pl_plscolbg(r,g,b); */ /*Set background back to what it was*/ 


	return(NULL);
}

Var *
ff_pplot(vfuncptr func, Var * arg)
{
	Var *obj = NULL, *Xaxis=NULL;
	Var *pmax=NULL,*pmin=NULL,*pxa_max=NULL,*pxa_min=NULL;
	int WindowID=-1,append=0;
	char *Axis=NULL;
	int setup=0;
	int xFlag=0;

	float *x,*y;
	int i,j,k;
	int Mode[3];

	int Ord[3];
	int XOrd[3];

	int CE[3];

	int dsize,xa_dsize;

	char nTitle[200];

	float min=FLT_MAX,xa_min=FLT_MAX;
	float max=FLT_MIN,xa_max=FLT_MIN;
	float tmp;

	int max_flag=0,min_flag=0,xa_max_flag=0,xa_min_flag=0;

	int color=-1;
	static int ccolor=0;

	int zm=0;

	int obj_index;
	

	const char *options[]={"X","Y","Z","x","y","z",NULL};

	
        int ac;
        Var **av;
        Alist alist[11];
	static int Stream;

        alist[0] = make_alist("obj",		ID_VAL,	NULL,   &obj);
        alist[1] = make_alist("axis",		ID_ENUM,	options,	&Axis);
        alist[2] = make_alist("append",		INT, 	NULL,	&append);
        alist[3] = make_alist("window",		INT, 	NULL,	&WindowID);
        alist[4] = make_alist("Xaxis",		ID_VAL, 	NULL,	&Xaxis);
        alist[5] = make_alist("color",		INT, 	NULL,	&color);
        alist[6] = make_alist("xhigh",		ID_VAL, 	NULL,	&pxa_max);
        alist[7] = make_alist("xlow",		ID_VAL, 	NULL,	&pxa_min);
        alist[8] = make_alist("yhigh",		ID_VAL, 	NULL,	&pmax);
        alist[9] = make_alist("ylow",		ID_VAL, 	NULL,	&pmin);
        alist[10].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

        if (obj == NULL) {
                parse_error("%s: No object specified\n", func->name);
                return(NULL);
        }

	if (Axis==NULL){
		Axis=strdup("X");
	}

	if (pmax!=NULL) {max_flag=1;max=extract_float(pmax,0);}
	if (pmin!=NULL) {min_flag=1;min=extract_float(pmin,0);}
	if (pxa_max!=NULL) {xa_max_flag=1;xa_max=extract_float(pxa_max,0);}
	if (pxa_min!=NULL) {xa_min_flag=1;xa_min=extract_float(pxa_min,0);}



	pl_plgstrm(&Stream);

	/*First, descide about which window to use:*/

	if (WindowID > 0) { /*They want a specific window*/
		if (Stream != WindowID){ /*And it's not the one we're currently pointing at*/
			Stream=WindowID;
			pl_plsstrm(WindowID); /*Assume (for the time being) ID is valid*/
		}
	}

	else { /*No Specified Window which means...*/
		if (Stream==0){/*In this case, a new window*/
			pl_plmkstrm(&Stream);	
			sprintf(nTitle,"Plot Window-WindowID:%d",Stream);
			pl_plsplw(nTitle);
			pl_plssub(1,1);
			pl_plsdev("xtwin");
			pl_plspage(0,0,512,512,1,1);
			pl_plinit();
			setup=1;
		} /*else, will be using the current stream*/
	}


	Ord[0] = GetSamples(V_SIZE(obj), V_ORG(obj));
        Ord[1] = GetLines(V_SIZE(obj), V_ORG(obj));
        Ord[2] = GetBands(V_SIZE(obj), V_ORG(obj));

	switch (*Axis) {

	case 'X':
	case 'x':
		Mode[0]=0;
		Mode[1]=1;
		Mode[2]=2;
		break;

	case 'Y':
	case 'y':
		Mode[0]=1;
		Mode[1]=0;
		Mode[2]=2;
		break;

	case 'Z':
	case 'z':
		Mode[0]=2;
		Mode[1]=0;
		Mode[2]=1;
		break;
	}

	if (Xaxis!=NULL){
		XOrd[0] = GetSamples(V_SIZE(Xaxis), V_ORG(Xaxis));
		XOrd[1] = GetLines(V_SIZE(Xaxis), V_ORG(Xaxis));
		XOrd[2] = GetBands(V_SIZE(Xaxis), V_ORG(Xaxis));
		if (XOrd[Mode[0]]!=Ord[Mode[0]] ){
			parse_error("Given X-Axis doesn't agree with given data set");
			return(NULL);
		}
		else if ((XOrd[1]!=1 && XOrd[2]!=1) && (XOrd[1]!=Ord[Mode[1]] && XOrd[2]!=Ord[Mode[2]])){
			parse_error("Given X-Axis doesn't agree with given data set");
			return(NULL);
		}

		xFlag=1;
	}	

	else {
		if (!xa_min_flag){
			xa_min=0.0;
		}
		if (!xa_max_flag){
			xa_max=(float)Ord[Mode[0]];
		}
	}

	if (append==0){/*Then erase what's already there, and set flag to define window borders */
		if (vp[Stream]){
			pl_pladv(plRow[Stream]*plCol[Stream]);
		}
		else {
			vp[Stream]=1;
		}

		setup=1;
	}

			
	if (setup){/*Set up the borders and put up a box*/
		/*Find Y-Max/Min values for Box*/
		dsize=V_DSIZE(obj);
		if (!max_flag || !min_flag){
			for (i=1;i<dsize;i++){	
				tmp=extract_float(obj,i);
				if ((tmp > max) && (!max_flag)) max=tmp;
				if ((tmp < min) && (!min_flag)) min=tmp;
			}
		}
		if (xFlag){
			xa_dsize=V_DSIZE(Xaxis);
			if (!xa_max_flag || !xa_min_flag){
				for (i=1;i<xa_dsize;i++){
					tmp=extract_float(Xaxis,i);
					if ((tmp > xa_max) && (!xa_max_flag)) xa_max=tmp;
					if ((tmp < xa_min) && (!xa_min_flag)) xa_min=tmp;
				}
			}
		}
		pl_plcol0(15);/*Always draw the box in black*/
		pl_plenv(xa_min,xa_max,min,max,0,0);/*Last two will be settable in the future*/
	}

	x=calloc(Ord[Mode[0]],sizeof(float));
	y=calloc(Ord[Mode[0]],sizeof(float));


	if (color > 0 && color < 16)
		pl_plcol0(color);
	else {
		ccolor++;
		if (ccolor > 15) ccolor=1;
		pl_plcol0(ccolor);
	}


		
	for (i=0;i<Ord[Mode[2]];i++){
	  for (j=0;j<Ord[Mode[1]];j++){
	    for (k=0;k<Ord[Mode[0]];k++){
		CE[Mode[2]]=i;
		CE[Mode[1]]=j;
		CE[Mode[0]]=k;
		obj_index=cpos(CE[0],CE[1],CE[2],obj);
		y[k]=extract_float(obj,obj_index);
		if (xFlag){
			x[k]=extract_float(Xaxis,rpos(obj_index,obj,Xaxis));
		}
		else {
			x[k]=(float)k;
		}
	    }
	    pl_plline(k,x,y);
	  }
	}

	free(x);
	free(y);

	return (newVal(BSQ,1,1,1,INT,(&Stream)));
}

Var * 
ff_ptext(vfuncptr func, Var * arg)
{               
	extern int plP_wcpcx(float);
	extern int plP_wcpcy(float);
	extern float plP_pcdcx(int);
	extern float plP_pcdcy(int);
	extern int plP_mmpcy(float);

        char *text=NULL;
	Var *X=NULL,*Y=NULL,*E=NULL;
	int c=5;
	float e;
	int Angle=0;
	float xpos,ypos;
	float xmin,ymin,xmax,ymax;
	int Location=0;
	int Color=3;
	int Old_Color;
	float Scale;
	float x,y;
	float just;
	int	world=0;
	float def,ht,scale;
	float pheight,dheight;
	
          
        int ac;
        Var **av;
        Alist alist[9];
        alist[0] = make_alist("c",           INT,    		NULL,   &c);
        alist[1] = make_alist("e",           ID_VAL,    	NULL,   &E);
        alist[2] = make_alist("x",           ID_VAL,    	NULL,   &X);
        alist[3] = make_alist("y",           ID_VAL,    	NULL,   &Y);
        alist[4] = make_alist("angle",       INT,      		NULL,   &Angle);
        alist[5] = make_alist("text",        ID_STRING,		NULL,   &text);
        alist[6] = make_alist("kolor",       INT,		NULL,   &Color);
        alist[7] = make_alist("world",       INT,		NULL,   &world);
        alist[8].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

        if (text==NULL) {
		return(NULL);
	}

	if (X==NULL || Y==NULL){
		return(NULL);
	}

	xpos=extract_float(X,0);
	ypos=extract_float(Y,0);

	pl_plcol0(Color);

	if (E==NULL){
		e=1.0;
	}
	else {
		e=extract_float(E,0);
	}

	if (e!=1.0){
		if (e<1.0){
			Scale=((1.0-e)+(e*4.0));
		}
		else {
			Scale=4+(e*10.0)-10.0;
		}

		pl_plschr(1.0,Scale);
	}
	else 
		pl_plschr(1.0,4);


	if (Angle) {
		x=cos((Angle/180.0)*3.1415926);
		y=sin((Angle/180.0)*3.1415926);
		if (c==7 || c==4 || c==1)
			just=1.0;
		else if (c==9 || c==6 || c==3)
			just=0.0;	
		else
			just=0.5;

	}
	else {
		x=1.0;
		y=0.0;
	}

	if (c==7 || c==4 || c==1)
		just=1.0;
	else if (c==9 || c==6 || c==3)
		just=0.0;	
	else
		just=0.5;

	pl_plgwrld(&xmin,&xmax,&ymin,&ymax);

	if (xpos >=xmax) Location +=4;
	if (xpos <=xmin) Location +=2;
	if (ypos >=ymax) Location +=16;
	if (ypos <=ymin) Location +=8;

	if (Location && world){
		ptext_border(e,c,Angle,xpos,ypos,text,Location);
	}

	else if (world) {
		pl_plptex(xpos,ypos,x,y,just,text);
	}
	else {
		pl_plgchr(&def,&ht);
		scale=0.05*ht;/*Conversion scale*/
		pheight=plP_mmpcy(SCALECONST*scale);/*Approx Physical height mm*/
		dheight=plP_pcdcy(pheight);/*This is the height of characters in device coords*/
		if (c==7 || c==8 || c==9){
			ypos+=(dheight/2.0)*x;
			xpos-=(dheight/2.0)*y;
		}

		else if (c==1 || c==2 || c==3){
			ypos-=(dheight/2.0)*x;
			xpos+=(dheight/2.0)*y;
		}

		pl_plptexd(xpos,ypos,x,y,just,text);
	}

	pl_plschr(1,4);
	pl_plcol0(1);

	return(NULL);
}

void
ptext_border(float e,int c,int angle,float x,float y,char *text,int Location)
{

	extern int plP_wcpcx(float);
	extern int plP_wcpcy(float);
	extern float plP_pcdcx(int);
	extern float plP_pcdcy(int);
	extern int plP_mmpcy(float);


	float xmin,xmax,ymin,ymax;

	char ori[3];

	float disp,pos,just;
	int Disp_Mode;

	float def,ht,scale;
	float pheight,dheight;
	float tmp1,tmp2;
	float dwxmin,dwxmax,dwymin,dwymax;


	pl_plgchr(&def,&ht);
	scale=0.05*ht;/*Conversion scale*/
	pheight=plP_mmpcy(SCALECONST*scale);/*Approx Physical height mm*/
	dheight=plP_pcdcy(pheight);/*This is the height of characters in device coords*/
	

	pl_plgwrld(&xmin,&xmax,&ymin,&ymax);
	dwxmin=plP_pcdcx(plP_wcpcx(xmin));
	dwxmax=plP_pcdcx(plP_wcpcx(xmax));
	dwymin=plP_pcdcy(plP_wcpcy(ymin));
	dwymax=plP_pcdcy(plP_wcpcy(ymax));

	if (c==7 || c==4 || c==1)
		just=1.0;
	else if (c==9 || c==6 || c==3)
		just=0.0;	
	else
		just=0.5;

	if (c==7 || c==8 || c==9)
		Disp_Mode=0.5;
	else if (c==1 || c==2 || c==3)
		Disp_Mode=-0.5;
	else
		Disp_Mode=0;
	

	/*Determin initial border location (left,right,top, or bottom)
	  Corners (10,12,18, & 20) are ignored...not enough room*/

	switch (Location) {

	case 2:
		strcpy(ori,"l");
		if (angle)
			strcat(ori,"v");
		pos=(y-ymin)/(ymax-ymin);
		tmp1=dwxmin-(plP_pcdcx(plP_wcpcx(x)));
		tmp2=tmp1/dheight;
		disp+=tmp2;
		break;
	case 4:
		strcpy(ori,"r");
		if (angle)
			strcat(ori,"v");
		pos=(y-ymin)/(ymax-ymin);
		tmp1=(plP_pcdcx(plP_wcpcx(x)))-dwxmax;
		tmp2=tmp1/dheight;
		disp+=tmp2;
		break;
	case 8:
		strcpy(ori,"b");
		pos=(x-xmin)/(xmax-xmin);
		tmp1=dwymin-(plP_pcdcy(plP_wcpcy(y)));
		tmp2=tmp1/dheight;
		disp+=tmp2;
		break;
	case 16:
		strcpy(ori,"t");
		pos=(x-xmin)/(xmax-xmin);
		tmp1=(plP_pcdcy(plP_wcpcy(y)))-dwymax;
		tmp2=tmp1/dheight;
		disp+=tmp2;
		break;

	case 10:
		return;
	case 12:
		return;
	case 18:
		return;
	case 20:
		return;

	}

	pl_plmtex(ori,disp,pos,just,text);
}


Var * 
ff_pline(vfuncptr func, Var * arg)
{               
	int line_style=1,line_thickness=1;
	Var *X=NULL,*Y=NULL;
	float *x,*y;
	int Color=15;
	int length_X,length_Y;
	int dsizeX,dsizeY;
	int i;
	int olt;
          
        int ac;
        Var **av;
        Alist alist[6];
        alist[0] = make_alist("ls",           INT,    		NULL,   &line_style);
        alist[1] = make_alist("lt",           INT,    		NULL,   &line_thickness);
        alist[2] = make_alist("x",           ID_VAL,    	NULL,   &X);
        alist[3] = make_alist("y",           ID_VAL,    	NULL,   &Y);
        alist[4] = make_alist("color",       INT,		NULL,   &Color);
        alist[5].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

        if (X==NULL || Y==NULL){
		return(NULL);
	}

	pl_plcol0(Color);
	pl_plgwid(&olt);/*Get the current line thickness*/

	length_X=GetSamples(V_SIZE(X), V_ORG(X));
	length_Y=GetSamples(V_SIZE(Y), V_ORG(Y));
	if (length_X!=length_Y){
		parse_error("Hey, I can't graph this!\n");
		return(NULL);
	}/*Need to make this more flexible and robust*/
	else  {
		dsizeX=V_DSIZE(X);
		dsizeY=V_DSIZE(Y);
		x=(float *)calloc(dsizeX,sizeof(float));
		y=(float *)calloc(dsizeY,sizeof(float));
		for(i=0;i<dsizeX;i++){
			x[i]=extract_float(X,i);
			y[i]=extract_float(Y,i);
		}
	}
			
	pl_pllsty(line_style);
	pl_plwid(line_thickness);/*Set our desired line thickness*/
	pl_plline(dsizeX,x,y);	
	pl_plwid(olt);/*Now set it back to what it was*/
	return(NULL);
}	



Var *
ff_pbox(vfuncptr func, Var * arg)
{
	extern label_box(char *,float,char *, float);


	int X=0,Y=0,x=1,y=1;
	Var *E=NULL;
	float e=1.0;
	int lt=1,olt;
	int Color=15;
	int m=0;
	char xBoxString[10];
	char yBoxString[10];
	float Scale;
	float xtick=0.0,ytick=0.0;
	
        int ac;
        Var **av;
        Alist alist[9];
        alist[0] = make_alist("e",           ID_VAL,   		NULL,   &E);
        alist[1] = make_alist("lt",          INT,    		NULL,   &lt);
        alist[2] = make_alist("x",           INT,	    	NULL,   &x);
        alist[3] = make_alist("y",           INT,    		NULL,   &y);
        alist[4] = make_alist("X",           INT,    		NULL,   &X);
        alist[5] = make_alist("Y",           INT,    		NULL,   &Y);
        alist[6] = make_alist("m",           INT,    		NULL,   &m);
        alist[7] = make_alist("color",       INT,		NULL,   &Color);
        alist[8].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	strcpy(xBoxString,"bcst");
	strcpy(yBoxString,"bcstv");

	pl_plgwid(&olt);
	pl_plwid(lt);
	pl_plcol0(Color);
	pl_plbox(xBoxString,0,0,yBoxString,0,0);
	pl_plwid(olt);

	if (E!=NULL)
		e=extract_float(E,0);
	if (e!=1.0){
		if (e<1.0){
			Scale=((1.0-e)+(e*4.0));
		}
		else {
			Scale=4+(e*10.0)-10.0;
		}

		pl_plschr(1.0,Scale);
	}
	else 
		pl_plschr(1.0,4);


	if (x)
		strcat(xBoxString,"n");
	if (y)
		strcat(yBoxString,"n");
	if (X){
		strcat(xBoxString,"l");
		xtick=1.0;
	}

	if (Y){
		strcat(yBoxString,"l");
		ytick=1.0;
	}
		
	pl_pllabelbox(xBoxString,xtick,yBoxString,ytick);	

	pl_plschr(1.0,4.0);

	return(NULL);
}


Var *
ff_pzoom(vfuncptr func, Var * arg)
{
	float x_hi=0,x_lo=0,y_hi=0,y_lo=0;
	int wID=-1;
	int Reset=0;
	Var *X=NULL;
	Var *Y=NULL;

	int old_Stream;

	int ac;
	Var **av;
	Alist alist[9];
	alist[0] = make_alist("x_hi",		FLOAT,	NULL,   &x_hi);
	alist[1] = make_alist("x_lo",		FLOAT,	NULL,	&x_lo);
	alist[2] = make_alist("y_hi",		FLOAT, 	NULL,	&y_hi);
	alist[3] = make_alist("y_lo",		FLOAT, 	NULL,	&y_lo);
	alist[4] = make_alist("Xv",		ID_VAL, 	NULL,	&X);
	alist[5] = make_alist("Yv",		ID_VAL, 	NULL,	&Y);
	alist[6] = make_alist("ID",		INT, 	NULL,	&wID);
	alist[7] = make_alist("reset",		INT, 	NULL,	&Reset);
	alist[8].name = NULL;

	if (parse_args(func, arg, alist) == 0) return(NULL);

	if (X!=NULL){
		x_hi=extract_float(X,1);
		x_lo=extract_float(X,0);
	}
	if (Y!=NULL){
		y_hi=extract_float(Y,1);
		y_lo=extract_float(Y,0);
	}

	if (wID < 0){
		pl_plgstrm(&wID);
		old_Stream=wID;
	}
	else {
		pl_plgstrm(&old_Stream);
		pl_plsstrm(wID);
	}

	if (Reset){
		Reset_Zoom(wID);
	}
	else {
		Zoom(0,x_lo,y_lo,wID);
		Zoom(1,x_hi,y_hi,wID);
	}

	pl_plsstrm(old_Stream);

	return(NULL);
}

#endif
