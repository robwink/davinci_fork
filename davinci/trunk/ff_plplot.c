#include "parser.h"
#include "api_extern_defs.h"

void zoom(void);

Var *
ff_popen(vfuncptr func, Var * arg)
{

	int Row=1,Col=1,Xpos=200,Ypos=200,Xpixels=512,Ypixels=512;
	char *Title=NULL;

	int *Stream=(int *)calloc(1,sizeof(int));

        int ac;
        Var **av;
        Alist alist[8];
        alist[0] = make_alist("row",		INT,	NULL,   &Row);
        alist[1] = make_alist("col",		INT,	NULL,	&Col);
        alist[2] = make_alist("xpos",		INT, 	NULL,	&Xpos);
        alist[3] = make_alist("ypos",		INT, 	NULL,	&Ypos);
        alist[4] = make_alist("xpixels",		INT, 	NULL,	&Xpixels);
        alist[5] = make_alist("ypixels",		INT, 	NULL,	&Ypixels);
        alist[6] = make_alist("title",		ID_STRING, 	NULL,	&Title);
        alist[7].name = NULL;

        make_args(&ac, &av, func, arg);
        if (parse_args(ac, av, alist)) return(NULL);

	if (Title==NULL){
		Title=strdup("Plot Window");
	}

	pl_plmkstrm(Stream);	

	pl_plsplw(Title);

	pl_plssub(Col,Row);
	pl_plsdev("xtwin");
	pl_plspage(0,0,Xpixels,Ypixels,Xpos,Ypos);

	pl_plinit();
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

        make_args(&ac, &av, func, arg);
        if (parse_args(ac, av, alist)) return(NULL);

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
	pl_plsstrm(Stream);
	pl_plgcolbg(&r,&g,&b);
	pl_plscolbg(255,255,255);/*White background for printing*/
	pl_plsstrm(Dummy);
	pl_plcpstrm(Stream,0);
	pl_plreplot();
	pl_plend1();
	pl_plsstrm(Stream);
	pl_plscolbg(r,g,b);/*Set background back to what it was*/


	return(NULL);
}

Var *
ff_pplot(vfuncptr func, Var * arg)
{
	Var *obj = NULL, *Xaxis=NULL;
	int WindowID=-1,append=0;
	char *Axis=NULL;
	int setup=0;
	int xFlag=0;

	float *x,*y;
	int i,j,k;
	int Mode[3];

	int Ord[3];

	int CE[3];

	int dsize,xa_dsize;

	int xOrd,yOrd,zOrd;
	float min,max,xa_min,xa_max,tmp;
	int color=-1;
	static int ccolor=0;

	int zm=0;

	int obj_index;
	

	char *options[]={"X","Y","Z","x","y","z",NULL};

	
        int ac;
        Var **av;
        Alist alist[7];
	static int Stream;

        alist[0] = make_alist("obj",		ID_VAL,	NULL,   &obj);
        alist[1] = make_alist("axis",		ID_ENUM,	options,	&Axis);
        alist[2] = make_alist("append",		INT, 	NULL,	&append);
        alist[3] = make_alist("window",		INT, 	NULL,	&WindowID);
        alist[4] = make_alist("Xaxis",		ID_VAL, 	NULL,	&Xaxis);
        alist[5] = make_alist("color",		INT, 	NULL,	&color);
        alist[6].name = NULL;

        make_args(&ac, &av, func, arg);
        if (parse_args(ac, av, alist)) return(NULL);
        if (obj == NULL) {
                parse_error("%s: No object specified\n", func->name);
                return(NULL);
        }

	if (Axis==NULL){
		Axis=strdup("X");
	}

	pl_plgstrm(&Stream);

	/*First, descide about which window to use:*/

	if (WindowID > 0) { /*They want a specific window*/
		if (Stream != WindowID){ /*And it's not the one we're currently pointing at*/
			pl_plsstrm(WindowID); /*Assume (for the time being) ID is valid*/
		}
	}

	else { /*No Specified Window which means...*/
		if (Stream==0){/*In this case, a new window*/
			pl_plmkstrm(&Stream);	
			pl_plsplw("Plot");
			pl_plssub(1,1);
			pl_plsdev("xtwin");
			pl_plspage(0,0,512,512,0,0);
			pl_plinit();
			setup=1;
		} /*else, will be using the current stream*/
	}

	if (append==0){/*Then erase what's already there, and set flag to define window borders */
		pl_plbop();
		setup=1;
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
		xOrd = GetSamples(V_SIZE(Xaxis), V_ORG(Xaxis));
		yOrd = GetLines(V_SIZE(Xaxis), V_ORG(Xaxis));
		zOrd = GetBands(V_SIZE(Xaxis), V_ORG(Xaxis));
		if (xOrd!=Ord[Mode[0]] ){
			parse_error("Given X-Axis doesn't agree with given data set");
			return(NULL);
		}
		else if ((yOrd!=1 && zOrd!=1) || (yOrd!=Ord[Mode[1]] && zOrd!=Ord[Mode[2]])){
			parse_error("Given X-Axis doesn't agree with given data set");
			return(NULL);
		}

		xFlag=1;
	}	

	else {
		xa_min=0.0;
		xa_max=(float)Ord[Mode[0]];
	}

			
	if (setup){/*Set up the borders and put up a box*/
		/*Find Y-Max/Min values for Box*/
		dsize=V_DSIZE(obj);
		min=max=extract_float(obj,0);
		for (i=1;i<dsize;i++){	
			tmp=extract_float(obj,i);
			if (tmp > max) max=tmp;
			if (tmp < min) min=tmp;
		}
		if (xFlag){
			xa_min=xa_max=extract_float(Xaxis,0);
			xa_dsize=V_DSIZE(Xaxis);
			for (i=1;i<xa_dsize;i++){
				tmp=extract_float(Xaxis,i);
				if (tmp > xa_max) xa_max=tmp;
				if (tmp < xa_min) xa_min=tmp;
			}
		}
		pl_plcol0(1);/*Always draw the box in red*/
		pl_plenv(xa_min,xa_max,min,max,0,0);/*Last two will be settable in the future*/
	}

	x=calloc(Ord[Mode[0]],sizeof(float));
	y=calloc(Ord[Mode[0]],sizeof(float));


	if (color > 0 && color < 17)
		pl_plcol0(color);
	else {
		ccolor++;
		if (ccolor > 14) ccolor=1;
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

	if (zm)
	 	zoom();

	return (newVal(BSQ,1,1,1,INT,(&Stream)));
}

void
zoom(void)
{
	int Dummy;
	int Stream;	


	pl_plgstrm(&Stream);
	pl_plmkstrm(&Dummy);
	pl_plenv(2.5,7.5,-0.4,0.2,0,0);
	pl_plcpstrm(Stream,0);
	pl_plreplot();
	pl_plsstrm(Stream);


}
