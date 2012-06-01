#include "Xfred.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#define APPCOLORS       HI,LO,FG,BG
#define SECTION_SPACE 2
#define LT      LEFTTEXT

int FG;
int BG;
int HI;
int LO;

Button Xlow,Xhigh;
Button Ylow,Yhigh;
Button Zlow,Zhigh;
Button Xlabel, Ylabel, Zlabel;
CButton Xauto, Yauto, Zauto;
CButton Xlog, Ylog, Zlog;

Button Surf_Xrot;
Button Surf_Zrot;
Button Surf_ZScale;
Button Surf_Scale;
CButton Surf_Hidden3D;

CButton Cont_Surf;
CButton Cont_Base;

RButton Cont_rb;
Button Cont_start;
Button Cont_incr;
Button Cont_end;
Button Cont_levels;

Button PS_fname;
Button PS_font;
Button PS_pt;
PButton PS_write;
MButton PS_type;

MenuItem *approx;
MenuItem *app_order;

Button MakeTextInput();
int readfd, writefd;
char *get_pipe_input();

main()
{
    Display *d;
    XEvent E;

	int i;
    int val;
    char buf[512], *ptr;
    int count = 0;
    int fds;
    struct timeval t;
	int fd1[2], fd2[2];
	int pid=0;
	int err;

	extern int _Xdebug;

	_Xdebug = True;

	if (pipe(fd1) < 0 || pipe(fd2) < 0) {
		fprintf(stderr, "Unable to open pipes\n");
		exit(1);
	}
	if ((pid = fork()) < 0) {
		fprintf(stderr, "Unable to fork()\n");
		exit(1);
	} else if (pid == 0) {
		close(fd1[1]);
		close(fd2[0]);
		if (fd1[0] != STDIN_FILENO) {
			if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO) {
				fprintf(stderr, "Error: Child unable to dup2 stdin\n");
				exit(1);
			}
			close(fd1[0]);
		}
		if (fd2[1] != STDOUT_FILENO) {
			if (dup2(fd2[1], STDERR_FILENO) != STDERR_FILENO) {
				fprintf(stderr, "Error: Child unable to dup2 stderr\n");
				exit(1);
			}
			close(fd2[1]);
		}
		execlp("gnuplot", "gnuplot", (char *)0);
		fprintf(stdout, "Unable to find gnuplot.  Aborting.\n");
		exit(1);
	} else {
		close(fd1[0]);
		close(fd2[1]);
		readfd = fd2[0];
		writefd = fd1[1];
	}


    if (!initx(NULL, &d, NULL, NULL, NULL)) {
        (void)fprintf(stderr, "Could not initialize X server.\n");
        exit(1);
    }

    HI = XfColor(0,(char *)1);
    LO = XfColor(0,(char *)0);
    FG = XfColor(0,(char *)0);
    BG = XfColor(0,"gray80");

    /**
     ** Create out main window, activate it disabled so it doesn't do anything
     **/

    CreateWindows(d);
	fcntl(fileno(stdin),F_SETFL,fcntl(fileno(stdin),F_GETFL,0) | O_NONBLOCK);

    while (1) {         /* Standard loop to handle all event processing */
        while (XPending(d)) {
            XNextEvent(d, &E);
            XfPushXB(XfEventXB(&E), &E);
            XfButtonEvents(&E);
        }

        /**
         ** Handle file events
         ** Note: this should be generalized into an event type
         **/

		while((i = read(0, buf, 512)) != -1) {
			if (i == 0) {
				quit();
			}
			flush_pipe();
			write(writefd, buf, i);
			flush_pipe();
		}
    }
}

/***************************************************************************/

#define NSECTIONS 6

PButton PBClosed[NSECTIONS];
PButton PBOpened[NSECTIONS];
PButton PBSection[NSECTIONS];
PButton Main;
MButton Cont_aOrder;
MButton Cont_Approx;


XB_CALLBACK(quit_cb)
{
	quit();
}

XB_CALLBACK(SetPSType)
{
    MButton MB = (MButton)XfGetCallbackItem();
	char *str;

	str = XfMBSelectedText(MB);
	SetMenuText(MB, MB->menu, str);
}

XB_CALLBACK(DoPSWrite)
{
	char buf[512];

    PButton PB = (PButton)XfGetCallbackItem();
	char *fname = GetButtonText(PS_fname);
	char *font = GetButtonText(PS_font);
	char *pt = GetButtonText(PS_pt);
	char *type = PS_type->menu->str;

	if (fname == NULL || strlen(fname) == 0) {
		XBell(PB->display, 50);
		fprintf(stderr, "No filename specified\n");
	}
	sprintf(buf, "set terminal postscript %s color solid", type);
	if (strlen(font)) sprintf(buf+strlen(buf), " \"%s\"", font);
	if (strlen(pt)) sprintf(buf+strlen(buf), " %s ", pt);

	sprintf(buf + strlen(buf), "; set output \"%s\"\n", fname);

	SendToPlot(buf);

	sprintf(buf, "set term x11\n");
	SendToPlot(buf);
}


XB_CALLBACK(SetCont)
{
	char buf[512];
    RButton RB = (RButton)XfGetCallbackItem();
	int i = XfActiveRB(RB);

	if (i == 0) {
		sprintf(buf, "set cntrparam levels incr %s,%s,%s\n", 
						GetButtonText(Cont_start),
						GetButtonText(Cont_incr),
						GetButtonText(Cont_end));
	} else if (i == 1) {
		sprintf(buf, "set cntrparam levels auto %s\n",
						GetButtonText(Cont_levels));
	}
	SendToPlot(buf);
}

XB_CALLBACK(SetIncr)
{
	char buf[512];
    Button B = (Button)XfGetCallbackItem();
	sprintf(buf, "set cntrparam levels incr %s,%s,%s\n", 
					GetButtonText(Cont_start),
					GetButtonText(Cont_incr),
					GetButtonText(Cont_end));
	SendToPlot(buf);
	XfActivateRBList(Cont_rb, 0);
}

XB_CALLBACK(SetLevels)
{
	char buf[512];
	sprintf(buf, "set cntrparam levels %s\n",
					GetButtonText(Cont_levels));
	SendToPlot(buf);
}

XB_CALLBACK(SetCntr)
{
	if (XfActiveCB(Cont_Surf)) {
		if (XfActiveCB(Cont_Base)) {
			SendToPlot("set contour both\n");
		} else {
			SendToPlot("set contour surface\n");
		}
	} else if (XfActiveCB(Cont_Base)) {
		SendToPlot("set contour base\n");
	} else {
		SendToPlot("set nocontour\n");
	}
}

XB_CALLBACK(SetApprox)
{
    MButton MB = (MButton)XfGetCallbackItem();
	char *str;

	str = XfMBSelectedText(MB);
	SetMenuText(MB, MB->menu, str);

	if (!strcmp(str, "Linear")) {
		Cont_aOrder->menu->status = -1;
		XfActivateMB(Cont_aOrder, -1);
		RedrawMB(Cont_aOrder);

		SendToPlot("set cntrparam linear\n");

	} else if (!strcmp(str, "BSpline")) {
		Cont_aOrder->menu->status = 0;
		XfActivateMB(Cont_aOrder, 0);
		RedrawMB(Cont_aOrder);

		SendToPlot("set cntrparam bspline\n");

	} else if (!strcmp(str, "CSpline")) {
		Cont_aOrder->menu->status = -1;
		XfActivateMB(Cont_aOrder, -1);
		RedrawMB(Cont_aOrder);

		SendToPlot("set cntrparam cubicspline\n");

	}
}

XB_CALLBACK(SetAOrder)
{
	char buf[512];
    MButton MB = (MButton)XfGetCallbackItem();
	char *str;

	str = XfMBSelectedText(MB);
	SetMenuText(MB, MB->menu, str);

	sprintf(buf, "set cntrparam order %s\n", str);
	SendToPlot(buf);
}

XB_CALLBACK(SetHidden3D)
{
    CButton B = (CButton)XfGetCallbackItem();

	if (XfActiveCB(B)) {
		SendToPlot("set hidden3d\n");
	} else {
		SendToPlot("set nohidden3d\n");
	}
}

XB_CALLBACK(SetView)
{
    Button B = (Button)XfGetCallbackItem();
	char buf[512];

	sprintf(buf, "set view %s,%s,%s,%s\n",
					GetButtonText(Surf_Xrot),
					GetButtonText(Surf_Zrot),
					GetButtonText(Surf_Scale),
					GetButtonText(Surf_ZScale));
	SendToPlot(buf);
}
XB_CALLBACK(SetLog)
{
    CButton B = (CButton)XfGetCallbackItem();
	char buf[512];

	sprintf(buf, "set %slogscale ", XfActiveCB(B) ? "" : "no");

	if (B == Xlog) strcat(buf, "x\n");
	else if (B == Ylog) strcat(buf, "y\n");
	else if (B == Zlog) strcat(buf, "z\n");

	SendToPlot(buf);
}

XB_CALLBACK(SetLabel)
{
    Button B = (Button)XfGetCallbackItem();
	char buf[512];

	if (B == Xlabel) sprintf(buf, "set xlabel \"%s\"\n", GetButtonText(B));
	else if (B == Ylabel) sprintf(buf, "set ylabel \"%s\"\n", GetButtonText(B));
	else if (B == Zlabel) sprintf(buf, "set zlabel \"%s\"\n", GetButtonText(B));

	SendToPlot(buf);
}

XB_CALLBACK(SetRange)
{
    Button B = (Button)XfGetCallbackItem();
	char buf[512];

	if (B == Xlow) sprintf(buf, "set xrange [%s:]\n", GetButtonText(B));
	else if (B == Xhigh) sprintf(buf, "set xrange [:%s]\n", GetButtonText(B));
	else if (B == Ylow) sprintf(buf, "set yrange [%s:]\n", GetButtonText(B));
	else if (B == Yhigh) sprintf(buf, "set yrange [:%s]\n", GetButtonText(B));
	else if (B == Zlow) sprintf(buf, "set zrange [%s:]\n", GetButtonText(B));
	else if (B == Zhigh) sprintf(buf, "set zrange [:%s]\n", GetButtonText(B));

	SendToPlot(buf);
}

XB_CALLBACK(XAuto)
{
    CButton B = (CButton)XfGetCallbackItem();
	SetButtonState(Xlow, XfActiveCB(B));
	SetButtonState(Xhigh, XfActiveCB(B));

	get_xrange();
	if (XfActiveCB(B)) {
		SendToPlot("set autoscale x\n");
	} else {
		SendToPlot("set noautoscale x\n");
	}
}
XB_CALLBACK(YAuto)
{
    CButton B = (CButton)XfGetCallbackItem();
	SetButtonState(Ylow, XfActiveCB(B));
	SetButtonState(Yhigh, XfActiveCB(B));
	if (XfActiveCB(B)) {
		SendToPlot("set autoscale y\n");
	} else {
		get_yrange();
		SendToPlot("set noautoscale y\n");
	}
}
XB_CALLBACK(ZAuto)
{
    CButton B = (CButton)XfGetCallbackItem();
	SetButtonState(Zlow, XfActiveCB(B));
	SetButtonState(Zhigh, XfActiveCB(B));
	if (XfActiveCB(B)) {
		SendToPlot("set autoscale z\n");
	} else {
		get_zrange();
		SendToPlot("set noautoscale z\n");
	}
}

XB_CALLBACK(openSection) 
{
    PButton PB;
    Display *disp;
    int i,j;
    Window r, win;
    int x,y;
    unsigned int w,h,bw,d;

    PB = (PButton) XfGetCallbackItem();
    disp = PB->display;
    win = PB->window;

    for (i = 0 ; i < NSECTIONS ; i++) {
        if (PB == PBClosed[i]) {
            XGetGeometry(disp,win, &r, &x, &y, &w, &h, &bw, &d);
            XfMovePB(PBOpened[i], x-bw, y-bw);

            XfActivatePB(PBOpened[i], 0);
            XfDeactivatePB(PB);

            PBSection[i] = PBOpened[i];

            for (j = i+1 ; j < NSECTIONS ; j++) {
                XGetGeometry(disp,PBSection[j-1]->window,
                             &r,&x,&y,&w,&h,&bw,&d);
                XfMovePB(PBSection[j], x-bw, y+h+bw*2+SECTION_SPACE);
            }
            XGetGeometry(disp,PBSection[NSECTIONS-1]->window,
                         &r,&x,&y,&w,&h,&bw,&d);
            XfResizePB(Main, w+(bw+10)*2, y+h+bw+10); 
            return;
        }
    }
}

static Time last=0;

XB_CALLBACK(closeSection) 
{
    PButton PB;
    Display *disp;
    int i,j;
    Window r, win;
    int x,y;
    unsigned int w,h,bw,d;
    Time current;
	XEvent *e;

    PB = (PButton) XfGetCallbackItem();
    disp = PB->display;
    win = PB->window;

	if ((e = XfGetCallbackEvent()) != NULL) {
		current = e->xbutton.time;
		if ((current - last) > 300) {
			last = current;
			return;
		}
	}

    for (i = 0 ; i < NSECTIONS ; i++) {
        if (PB == PBOpened[i]) {
            XGetGeometry(disp,win, &r, &x, &y, &w, &h, &bw, &d);
            XfMovePB(PBClosed[i], x-bw, y-bw);

            XfActivatePB(PBClosed[i], 0);
            XfDeactivatePB(PBOpened[i]);

            PBSection[i] = PBClosed[i];

            for (j = i+1 ; j < NSECTIONS ; j++) {
                XGetGeometry(disp,PBSection[j-1]->window,
                             &r,&x,&y,&w,&h,&bw,&d);
                XfMovePB(PBSection[j], x-bw, y+h+bw*2+SECTION_SPACE);
            }
            XGetGeometry(disp,PBSection[NSECTIONS-1]->window,
                         &r,&x,&y,&w,&h,&bw,&d);
            XfResizePB(Main, w+(bw+10)*2, y+h+bw+10);
            return;
        }
    }
}

XB_CALLBACK(dont_quit) 
{
        last = XfGetCallbackEvent()->xbutton.time;
        closeSection((XButton)PBSection[NSECTIONS-1], XfGetCallbackEvent());
}

CreateWindows(d)
     Display *d;
{
    Pixmap map;
    Window win;
    MenuItem *file;
    PButton *pb,PB;
    int x,y,w,h,hs;
    int i;
    CallBack func;

    Main = XfCreatePB(d,0, 0,0,290,185, APPCOLORS, NOTEXT, NULL);
    XfActivatePB(Main, 0);      /* disabled */
    win = Main->window;
    XStoreName(d, win, "dv Xgnuplot");

    XfSetDefaultFont(NULL, XfFont(NULL, "helvb12"));

    x = 10;
    y = 10;
    w = 270;
    h = 25;
    hs = h+SECTION_SPACE;

    pb = PBClosed;
    func = openSection;
    pb[0] = XfCreatePB(d, win, x,y,    w,h, APPCOLORS, LT("Objects"), NULL);
    pb[1] = XfCreatePB(d, win, x,y+=hs,w,h, APPCOLORS, LT("Axis"),        func);
    pb[2] = XfCreatePB(d, win, x,y+=hs,w,h, APPCOLORS, LT("Surfaces"),func);
    pb[3] = XfCreatePB(d, win, x,y+=hs,w,h, APPCOLORS, LT("Contours"),func);
    pb[4] = XfCreatePB(d, win, x,y+=hs,w,h, APPCOLORS, LT("Postscript"), func);
    pb[5] = XfCreatePB(d, win, x,y+=hs,w,h, APPCOLORS, LT("Quit"),        func);

    for (i = 0 ; i < NSECTIONS ; i++) {
        XfActivatePB(PBClosed[i],0);
        PBSection[i] = PBClosed[i];
    }
        
    /**
     ** These get postioned and sized when filled
     **/
    pb = PBOpened;
    pb[0] = XfCreatePB(d, win, 0, 0, w, 100, APPCOLORS, NOTEXT, closeSection);
    pb[1] = XfCreatePB(d, win, 0, 0, w, 100, APPCOLORS, NOTEXT, closeSection);
    pb[2] = XfCreatePB(d, win, 0, 0, w, 100, APPCOLORS, NOTEXT, closeSection);
    pb[3] = XfCreatePB(d, win, 0, 0, w, 100, APPCOLORS, NOTEXT, closeSection);
    pb[4] = XfCreatePB(d, win, 0, 0, w, 100, APPCOLORS, NOTEXT, closeSection);
    pb[5] = XfCreatePB(d, win, 0, 0, w, h, APPCOLORS, LT("Really quit?"), quit_cb);

    CreateSection_Objects(PBOpened[0]);
    CreateSection_Axis(PBOpened[1]);
    CreateSection_Surfaces(PBOpened[2]);
    CreateSection_Contours(PBOpened[3]);
    CreateSection_Postscript(PBOpened[4]);
    CreateSection_Quit(PBOpened[5]);

}


CreateSection_Objects(parent)
     PButton parent;
{
}
CreateSection_Axis(parent)
     PButton parent;
{
    int w = parent->width;
    LButton lb;
    Window win = parent->window;
    Display *d = parent->display;

    XfSetDefaultFont(NULL, XfFont(NULL, "helvb12"));
    XfActivateLB(XfCreateLB(d, win, 5,5,0,0, DEFCOLORS, LEFTTEXT("Axis")), 0);

    XfSetDefaultFont(NULL, XfFont(NULL, "helvr12"));

    XfActivateLB(XfCreateLB(d, win, 5,38,0,0, DEFCOLORS, LEFTTEXT("X")), 0);
    XfActivateLB(XfCreateLB(d, win, 5,63,0,0, DEFCOLORS, LEFTTEXT("Y")), 0);
    XfActivateLB(XfCreateLB(d, win, 5,88,0,0, DEFCOLORS, LEFTTEXT("Z")), 0);

    XfActivateLB(XfCreateLB(d, win, 30,20,0,0, DEFCOLORS, LEFTTEXT("Low")), 0);
    XfActivateLB(XfCreateLB(d, win, 85,20,0,0, DEFCOLORS, LEFTTEXT("High")), 0);
    XfActivateLB(XfCreateLB(d, win, 130,20,0,0,DEFCOLORS, LEFTTEXT("Auto")), 0);
    XfActivateLB(XfCreateLB(d, win, 160,20,0,0, DEFCOLORS, LEFTTEXT("Log")), 0);
    XfActivateLB(XfCreateLB(d, win, 185,20,0,0, DEFCOLORS, 
                            LEFTTEXT("Axis Label")), 0);

    Xauto = XfCreateCB(d, win, 135, 36, 20,20,DEFCOLORS,NOTEXT,XAuto);
    Yauto = XfCreateCB(d, win, 135, 61, 20,20,DEFCOLORS,NOTEXT,YAuto);
    Zauto = XfCreateCB(d, win, 135, 86, 20,20,DEFCOLORS,NOTEXT,ZAuto);

    Xlog = XfCreateCB(d, win, 160, 36, 20,20,DEFCOLORS,NOTEXT,SetLog);
    Ylog = XfCreateCB(d, win, 160, 61, 20,20,DEFCOLORS,NOTEXT,SetLog);
    Zlog = XfCreateCB(d, win, 160, 86, 20,20,DEFCOLORS,NOTEXT,SetLog);

    XfActivateCB(Xauto,4);
    XfActivateCB(Yauto,4);
    XfActivateCB(Zauto,4);

    XfActivateCB(Xlog,0);
    XfActivateCB(Ylog,0);
    XfActivateCB(Zlog,0);

    w = 50;

    XfSetDefaultFont(NULL, XfFont(NULL, "fixed12"));

    Xlow =  MakeTextInput(d,win,20,    35, w,20, FG,HI, "-", 0, SetRange);
    Xhigh = MakeTextInput(d,win,20+w+5,35, w,20, FG,HI, "-", 0, SetRange);
    Ylow =  MakeTextInput(d,win,20,    60, w,20, FG,HI, "-", 0, SetRange);
    Yhigh = MakeTextInput(d,win,20+w+5,60, w,20, FG,HI, "-", 0, SetRange);
    Zlow =  MakeTextInput(d,win,20,    85, w,20, FG,HI, "-", 0, SetRange);
    Zhigh = MakeTextInput(d,win,20+w+5,85, w,20, FG,HI, "-", 0, SetRange);

	SetButtonState(Xlow, 1);
	SetButtonState(Xhigh, 1);
	SetButtonState(Ylow, 1);
	SetButtonState(Yhigh, 1);
	SetButtonState(Zlow, 1);
	SetButtonState(Zhigh, 1);

    Xlabel = MakeTextInput(d, win,185,35, 75,20, FG, HI, "", 1, SetLabel);
    Ylabel = MakeTextInput(d, win,185,60, 75,20, FG, HI, "", 1, SetLabel);
    Zlabel = MakeTextInput(d, win,185,85, 75,20, FG, HI, "", 1, SetLabel);

    XfResizePB(parent, parent->width, 120);
}

CreateSection_Surfaces(parent)
     PButton parent;
{
    int w = parent->width;
    LButton lb;
    Window win = parent->window;
    Display *d = parent->display;

    XfSetDefaultFont(NULL, XfFont(NULL, "helvb12"));
    XfActivateLB(XfCreateLB(d,win, 5,5,0,0, DEFCOLORS, LEFTTEXT("Surfaces")),0);

    XfSetDefaultFont(NULL, XfFont(NULL, "helvr12"));

    XfActivateLB(XfCreateLB(d,win,5,38,0,0, DEFCOLORS, LT("View")), 0);
    XfActivateLB(XfCreateLB(d,win,50,20,0,0, DEFCOLORS, LT("X rot")), 0);
    XfActivateLB(XfCreateLB(d,win,105,20,0,0, DEFCOLORS, LT("Z rot")), 0);
    XfActivateLB(XfCreateLB(d,win,160,20,0,0, DEFCOLORS, LT("Scale")), 0);
    XfActivateLB(XfCreateLB(d,win,210,20,0,0, DEFCOLORS, LT("Z Scale")), 0);

    XfSetDefaultFont(NULL, XfFont(NULL, "fixed12"));

    Surf_Xrot =  MakeTextInput(d,win,40,35, 50,20, FG,HI, "60", 0, SetView);
    Surf_Zrot =  MakeTextInput(d,win,95,35, 50,20, FG,HI, "30", 0, SetView);
    Surf_Scale =  MakeTextInput(d,win,150,35, 50,20, FG,HI, "1.0", 0, SetView);
    Surf_ZScale =  MakeTextInput(d,win,205,35, 50,20, FG,HI, "1.0", 0, SetView);

    XfSetDefaultFont(NULL, XfFont(NULL, "helvr12"));

    Surf_Hidden3D = XfCreateCB(d, win, 10, 60, 170,20,DEFCOLORS,
                                  LEFTTEXT("Hidden Line Removal"),SetHidden3D);
    XfActivatePB(Surf_Hidden3D,0);
        
    XfResizePB(parent, parent->width, 90);
}
CreateSection_Contours(parent)
     PButton parent;
{
    int w = parent->width;
    LButton lb;
    Window win = parent->window;
    Display *d = parent->display;
    CButton cb;
        RButton rb;

    XfSetDefaultFont(NULL, XfFont(NULL, "helvb12"));
    XfActivateLB(XfCreateLB(d,win, 5,5,0,0, DEFCOLORS, LEFTTEXT("Contours")),0);

    XfSetDefaultFont(NULL, XfFont(NULL, "helvr12"));

	Cont_Surf = 
    cb = XfCreateCB(d,win, 10,20, 70,20,DEFCOLORS, LEFTTEXT("Surface"),SetCntr);
    XfActivateCB(cb, 0);

	Cont_Base = 
    cb = XfCreateCB(d,win, 10,45,60,20,DEFCOLORS,LEFTTEXT("Base"),SetCntr);
    XfActivateCB(cb, 0);

    lb = XfCreateLB(d,win,115,24,0,0, DEFCOLORS, LT("Approximation"));
    XfActivateLB(lb, 0);

    lb = XfCreateLB(d,win,115,49,0,0, DEFCOLORS, LT("Approximation Order"));
    XfActivateLB(lb, 0);

    approx = XfAddMenuItem(NULL, CENTERTEXT("Linear"), 0,0,0,(void *)0);
    XfAddMenuItem(approx, CENTERTEXT("Linear"), 0,0,0,(void *)1);
    XfAddMenuItem(approx, CENTERTEXT("BSpline"), 0,0,0,(void *)2);
    XfAddMenuItem(approx, CENTERTEXT("CSpline"), 0,0,0,(void *)3);

    app_order = XfAddMenuItem(NULL, CENTERTEXT("10"), 0,0,0,(void *)0);
    XfAddMenuItem(app_order, CENTERTEXT("2"), 0,0,0,(void *)2);
    XfAddMenuItem(app_order, CENTERTEXT("3"), 0,0,0,(void *)3);
    XfAddMenuItem(app_order, CENTERTEXT("4"), 0,0,0,(void *)4);
    XfAddMenuItem(app_order, CENTERTEXT("5"), 0,0,0,(void *)5);
    XfAddMenuItem(app_order, CENTERTEXT("6"), 0,0,0,(void *)6);
    XfAddMenuItem(app_order, CENTERTEXT("7"), 0,0,0,(void *)7);
    XfAddMenuItem(app_order, CENTERTEXT("8"), 0,0,0,(void *)8);
    XfAddMenuItem(app_order, CENTERTEXT("9"), 0,0,0,(void *)9);
    XfAddMenuItem(app_order, CENTERTEXT("10"), 0,0,0,(void *)10);

	Cont_Approx = XfCreateMB(d,win, w-70,20, 60,23, APPCOLORS,approx,SetApprox);
    XfActivateMB(Cont_Approx, 0);

	Cont_aOrder = XfCreateMB(d,win, w-35,45, 25,23, APPCOLORS, app_order,SetAOrder);
    XfActivateMB(Cont_aOrder, 0);


    Cont_rb = 
	rb = 
	XfCreateRB(d,win, 10,90,90,20, DEFCOLORS,LT("Incremental"), SetCont, NULL);
	XfCreateRB(d, win, 10,120,90,20, DEFCOLORS, LT("Auto Levels"), SetCont, rb);
    XfActivateRBList(rb, 1);

    XfActivateLB(XfCreateLB(d,win, 120,75,0,0, DEFCOLORS, LEFTTEXT("start")),0);
    XfActivateLB(XfCreateLB(d,win, 170,75,0,0, DEFCOLORS, LEFTTEXT("incr")),0);
    XfActivateLB(XfCreateLB(d,win, 220,75,0,0, DEFCOLORS, LEFTTEXT("end")),0);

    XfSetDefaultFont(NULL, XfFont(NULL, "fixed12"));

    Cont_start = MakeTextInput(d,win,110,90, 45,20, FG,HI, "-", 0, SetIncr);
    Cont_incr = MakeTextInput(d,win,160,90, 45,20, FG,HI, "-", 0, SetIncr);
    Cont_end = MakeTextInput(d,win,210,90, 45,20, FG,HI, "-", 0, SetIncr);

    Cont_levels = MakeTextInput(d,win,110,120, 45,20, FG,HI, "5", 0, SetLevels);

    XfSetDefaultFont(NULL, XfFont(NULL, "helvr12"));

    XfResizePB(parent, parent->width, 150);
}
CreateSection_Postscript(parent)
     PButton parent;
{
    int w = parent->width;
    LButton lb;
    Window win = parent->window;
    Display *d = parent->display;
    CButton cb;
        RButton rb;
        MenuItem *mode;

    XfSetDefaultFont(NULL, XfFont(NULL, "helvb12"));
    XfActivateLB(XfCreateLB(d,win, 5,5,0,0, DEFCOLORS, LEFTTEXT("Postscript")),0);

    XfSetDefaultFont(NULL, XfFont(NULL, "helvr12"));

    XfActivateLB(XfCreateLB(d,win, 10,30,0,0, DEFCOLORS, LEFTTEXT("File")),0);

	PS_write = XfCreatePB(d, win, 210, 25, 50, 22, 
				APPCOLORS, CENTERTEXT("Write"), DoPSWrite),
    XfActivatePB(PS_write, 0);
        
    XfActivateLB(XfCreateLB(d,win, 10,60,0,0, DEFCOLORS, LEFTTEXT("Font")),0);
    XfActivateLB(XfCreateLB(d,win, 120,60,0,0, DEFCOLORS, LEFTTEXT("pt")),0);

        mode = XfAddMenuItem(NULL, CENTERTEXT("landscape"), 0,0,0,(void*)0);
               XfAddMenuItem(mode, CENTERTEXT("landscape"), 0,0,0,(void*)10);
               XfAddMenuItem(mode, CENTERTEXT("portrait"), 0,0,0,(void*)11);
               XfAddMenuItem(mode, CENTERTEXT("eps"), 0,0,0,(void*)12);

	PS_type = XfCreateMB(d,win, 180,55, 80,25, DEFCOLORS, mode,SetPSType);
    XfActivateMB(PS_type, 0);

    XfSetDefaultFont(NULL, XfFont(NULL, "fixed12"));

    PS_fname = MakeTextInput(d,win,40,25, 160,20, FG,HI, "", 1, NULL);
    PS_font = MakeTextInput(d,win,40,55, 70,20, FG,HI, "Helvetica", 1, NULL);
    PS_pt = MakeTextInput(d,win,135,55, 25,20, FG,HI, "14", 1, NULL);

    XfResizePB(parent, parent->width, 90);
}


CreateSection_Quit(parent)
     PButton parent;
{
    int w = parent->width;
    LButton lb;
    Window win = parent->window;
    Display *d = parent->display;
        PButton PB;

    XfSetDefaultFont(NULL, XfFont(NULL, "helvb12"));
        PB = XfCreatePB(d, win, 0, 0, w/2, 25, APPCOLORS, 
                                        CENTERTEXT("Really Quit?"), quit_cb);
        XfActivatePB(PB, 0);

        PB = XfCreatePB(d, win, w/2, 0, w/2, 25, APPCOLORS, 
                                        CENTERTEXT("Don't Quit."), dont_quit);
        XfActivatePB(PB, 0);
    XfResizePB(parent, parent->width, 25);
}

XB_CALLBACK(get_text)
{
    char buf[512];
    Button B = (Button)XfGetCallbackItem() ;

    if (GetText(B, XfGetCallbackEvent(), buf, 256, 0) == -1) 
        return;

    SetButtonText(B, buf);
    return;
}

Button
MakeTextInput(d,win,x,y,w,h,fg,bg,text,align,func)
     Display *d;
     Window win;
     int x,y,w,h,fg,bg;
     char *text;
     CallBack func;
         int align;
{
    Button B;
        XFontStruct *fs;
        int ht;

    B = XfCreateButton(d, win, x,y,w,h,1,fg,"text",2);
    XfAddButtonCallback(B, 0, get_text, NULL);

	fs = XfFont(0,0);
	ht = (h - fs->ascent)/2;

    XfAddButtonVisual(B, 0, XfCreateVisual(B, align,ht,0,0, fg, bg, 
                                           XfTextVisual, text, fs, align));
    XfAddButtonVisual(B, 1, XfCreateVisual(B, 0,0,0,0, fg, bg, 
                                           XfStippledVisual, "\252\125", 2, 2));

    if (func != NULL) XfAddButtonCallback(B, 0, func, NULL);
    XfActivateButton(B, 
		ExposureMask | ButtonPressMask | 
		KeyPressMask);

	return(B);
}

XfButtonEvents(E)
     XEvent*E;
{
    XfButtonPush(XfEventButton(E), E);
    XfSliderResponse(XfEventSlider(E), E);
}

char *
get_pipe_input()
{
	int fds;
	struct timeval t;
	int size = 0;
	int len = 1024;
	char *ptr = calloc(1,len+1);
	int err=0;

	fds = 1 << readfd;
	t.tv_usec = 100;
	t.tv_sec = 0;


	while(select(readfd+1, &fds, NULL, NULL, &t) > 0) {
		size += read(readfd, ptr + size, len - size);
		*(ptr + size) = '\0';
		if (size == len) {
			len *= 2;
			ptr = realloc(ptr, len);
		}
		t.tv_usec = 100;
		t.tv_sec = 0;
	}
	ptr[size] = '\0';
	return(ptr);
}

send_to_plot(str)
char *str;
{
	write(writefd, str, strlen(str));
}

SendToPlot(str)
char *str;
{
	send_to_plot(str);
	send_to_plot("replot\n");
}

get_xrange()
{
	char *p, *q, *ptr;
	char buf[512];
	float range[2];

	flush_pipe();
	send_to_plot("show xrange\n");
	ptr = get_pipe_input();

	if ((p = strstr(ptr, "[")) != NULL) {
		sscanf(p+1, "%f : %f", &range[0], &range[1]);
	}
	sprintf(buf, "%g", range[0]); SetButtonText(Xlow, buf);
	sprintf(buf, "%g", range[1]); SetButtonText(Xhigh, buf);

	free(ptr);
}

get_yrange()
{
	char *p, *q, *ptr;
	char buf[512];
	float range[2];

	flush_pipe();
	send_to_plot("show yrange\n");
	ptr = get_pipe_input();

	if ((p = strstr(ptr, "[")) != NULL) {
		sscanf(p+1, "%f : %f", &range[0], &range[1]);
	}
	sprintf(buf, "%g", range[0]); SetButtonText(Ylow, buf);
	sprintf(buf, "%g", range[1]); SetButtonText(Yhigh, buf);

	free(ptr);
}

get_zrange()
{
	char *p, *q, *ptr;
	char buf[512];
	float range[2];

	flush_pipe();
	send_to_plot("show zrange\n");
	ptr = get_pipe_input();

	if ((p = strstr(ptr, "[")) != NULL) {
		sscanf(p+1, "%f : %f", &range[0], &range[1]);
	}
	sprintf(buf, "%g", range[0]); SetButtonText(Zlow, buf);
	sprintf(buf, "%g", range[1]); SetButtonText(Zhigh, buf);

	free(ptr);
}

flush_pipe()
{
	int fds;
	struct timeval t;
	int size = 0;
	int len = 1024;
	char ptr[256];
	int err=0;

	fds = 1 << readfd;
	t.tv_usec = 0;
	t.tv_sec = 0;

	size = err;

	while(select(readfd+1, &fds, NULL, NULL, &t) > 0) {
		fds = 1 << readfd;
		err = read(readfd, ptr, 256);
		write(2,ptr,err);
		t.tv_usec = 100;
		t.tv_sec = 0;
	}
}


/*
*/

quit()
{
	fcntl(fileno(stdin),F_SETFL,fcntl(fileno(stdin),F_GETFL,0) & (~O_NONBLOCK));
	exit(1);
}
