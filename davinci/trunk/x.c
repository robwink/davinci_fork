#include "parser.h"

/*
#include <Xm/MenuShell.h>
#include <Xm/PanedW.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
*/

#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/Xrt3d.h>

static void TestMap(Widget xrt_3d, XButtonEvent *event);
static void TestPick(Widget xrt_3d, XButtonEvent *event);

Widget top=NULL;
static Widget		graph = NULL;
static Widget		show_mesh, show_shade, show_contour, show_zone;
XtAppContext	app;
Xrt3dData   *grid = NULL;

static Xrt3dData *
CalculateGrid()
{
    int			i, j;
    int			numx = 20, numy = 20;
    double		x, y;

    grid = Xrt3dMakeGridData(numx, numy, XRT3D_HUGE_VAL, 
                             8.0/(numx-1), 8.0/(numy-1), -3.0, -3.0, TRUE);
    if (!grid) {
        return(NULL);
    }

    for (i = 0; i < numx; i++) {
        x = grid->g.xorig + i*grid->g.xstep;
        for (j = 0; j < numy; j++) {
            y = grid->g.yorig + j*grid->g.ystep;
            grid->g.values[i][j] = 3*x*y - x*x*x - y*y*y;
        }
    }

    return(grid);
}

static void
DrawGraph(Widget w, XtPointer data, XtPointer cbs)
{
    Boolean		mesh, shade;
    Boolean		contour, zone;

    XtVaGetValues(show_mesh, 	XmNset, &mesh, 	NULL);
    XtVaGetValues(show_shade,	XmNset, &shade,	NULL);
    XtVaGetValues(show_contour,	XmNset, &contour,NULL);
    XtVaGetValues(show_zone, 	XmNset, &zone,	NULL);	

    XtVaSetValues(graph, 
                  XtNxrt3dDrawMesh,		mesh,
                  XtNxrt3dDrawShaded,		shade,
                  XtNxrt3dDrawContours,	contour,
                  XtNxrt3dDrawZones,		zone,
                  NULL);
}

static void
CreateWidgets(Widget parent)
{
    Widget		mainwindow, mainwin, form, options, sep, list_w;
    Xrt3dData	*grid;

    static char translations[] = "#augment \n\
         <Btn1Down>:    TestPick() \n\
         <Btn1Motion>:  TestPick() \n\
         <Btn1Up>:      TestPick() \n\
         <Btn3Down>:    TestMap() \n\
         <Btn3Motion>:  TestMap() \n\
         <Btn3Up>:      TestMap()";
    static XtActionsRec actions[] = {
        { "TestPick",   (XtActionProc) TestPick  },
        { "TestMap",    (XtActionProc) TestMap   }
    };

    XtAppAddActions(app, actions, XtNumber(actions));

	mainwin = XtVaCreateManagedWidget("mainwin",
						 xmMainWindowWidgetClass, parent,
						 NULL);

	MakeMenu(mainwin);

	form = XtVaCreateManagedWidget("form",
				 xmFormWidgetClass, mainwin,
				 XmNmarginHeight, 0,
				 XmNmarginWidth, 0,
				 XmNresizePolicy, XmRESIZE_GROW,
				 XmNheight, 300,
				 XmNwidth, 300,
				 NULL);

    // grid = CalculateGrid();


    graph = XtVaCreateManagedWidget("graph",
                                    xtXrt3dWidgetClass,			form,
                                    XmNtopAttachment,			XmATTACH_WIDGET,
                                    XmNtopAttachment,			XmATTACH_FORM,
                                    XmNbottomAttachment,		XmATTACH_FORM,
                                    XmNleftAttachment,			XmATTACH_FORM,
                                    XmNrightAttachment,			XmATTACH_FORM,
                                    XtNxrt3dSurfaceData,		grid,
                                    XtNxrt3dXAxisTitle,			"X Axis",
                                    XtNxrt3dYAxisTitle,			"Y Axis",
                                    XtNxrt3dZAxisTitle,			"Z Axis",
                                    XtNxrt3dDrawMesh,			TRUE,
                                    XtNxrt3dDrawShaded,			TRUE,
                                    XtNxrt3dDrawContours,		TRUE,
                                    XtNxrt3dDrawZones,			TRUE,
                                    XtNxrt3dHeaderBorder,		XRT3D_BORDER_PLAIN,
        							XmNtranslations,            XtParseTranslationTable(translations),
                                    NULL);
}

CreatePopup()
{
    if (graph == NULL) {
        CreateWidgets(top);
        XtRealizeWidget(top);
    }
}


/**
 ** Create data structure.
 ** Create popup if necessary
 **/

Var *
ff_xrt3d(vfuncptr func, Var * arg)
{
    int ac, i, j, count = 0;
    Var **av;
    Var *obj;
    int x, y;

    Alist alist[2];
    alist[0] = make_alist("obj",    ID_VAL,     NULL,     &obj);
    alist[1].name = NULL;

    make_args(&ac, &av, func, arg);
    if (parse_args(ac, av, alist)) return(NULL);

    if (obj == NULL)  {
        parse_error("No argument specified: %s(...obj=...)", av[0]);
        return(NULL);
    }

    x = V_SIZE(obj)[0];
    y = V_SIZE(obj)[1];

	if (grid) {
		Xrt3dDestroyData(grid, TRUE);
	}

    grid = Xrt3dMakeGridData(x, y, XRT3D_HUGE_VAL, 
                             x/10.0, y/10.0, 0.0, 0.0, TRUE);

    for (i = 0 ; i < x ; i++) {
        for (j = 0 ; j < y ; j++) {
            grid->g.values[i][j] = extract_double(obj, x*j+i);
        }
    }

	CreatePopup();
	
	XtVaSetValues(graph, XtNxrt3dSurfaceData, grid, NULL);
    return(NULL);
}


String menu1[] = { "Main", "Draw", "Help"}; 
String menu1_mnemonics[] = { "M", "D", "H" };

String menu2[] = { "Print", "Quit"};
String menu3[] = { "Mesh", "Shaded", "Contours", "Zones"};

XtCallbackProc 
main_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
	int item_no = (int) client_data;

	switch (item_no) {
		case 0:
			print_cb();
		case 1:
			/* quit */
			break;
	}
}

XtCallbackProc 
draw_cb(Widget widget, XtPointer client_data, XtPointer call_data)
{
	Boolean value;
	int item_no = (int) client_data;

	XtVaGetValues(widget, 	XmNset, &value, 	NULL);
	switch(item_no) {
		case 0:
			XtVaSetValues(graph, XtNxrt3dDrawMesh,		value, NULL);
			break;
		case 1:
			XtVaSetValues(graph, XtNxrt3dDrawShaded,    value, NULL);
			break;
		case 2:
			XtVaSetValues(graph, XtNxrt3dDrawContours,	value, NULL);
			break;
		case 3:
			XtVaSetValues(graph, XtNxrt3dDrawZones,		value, NULL);
			break;
	}
}

MakeMenu(Widget mainwin)
{
	int i;
	XmString strings[10];
	Widget widget, menubar, draw_menu, main_menu;

	for (i = 0 ; i < XtNumber(menu1) ; i++) {
		strings[i] =  XmStringCreateLocalized(menu1[i]);
	}

	menubar = XmVaCreateSimpleMenuBar(mainwin, "menubar",
		XmVaCASCADEBUTTON, strings[0], *menu1_mnemonics[0],
		XmVaCASCADEBUTTON, strings[1], *menu1_mnemonics[1],
		XmVaCASCADEBUTTON, strings[2], *menu1_mnemonics[2],
		NULL);

	/**
	 ** Set help menu item
	 **/
	if (widget = XtNameToWidget(menubar, "button_2")) 
		XtVaSetValues(menubar, XmNmenuHelpWidget, widget, NULL);

	for (i = 0 ; i < XtNumber(menu1) ; i++) {
		XmStringFree(strings[i]);
	}

	/**
	 ** Create Main menu
	 **/
	for (i = 0 ; i < XtNumber(menu2) ; i++) {
		strings[i] =  XmStringCreateLocalized(menu2[i]);
	}

	main_menu = XmVaCreateSimplePulldownMenu(menubar, "main_menu", 0, main_cb,
		XmVaPUSHBUTTON, strings[0], 'P', NULL, NULL,
		XmVaDOUBLE_SEPARATOR,
		XmVaPUSHBUTTON, strings[1], 'Q', NULL, NULL,
		NULL);

	for (i = 0 ; i < XtNumber(menu2) ; i++) {
		XmStringFree(strings[i]);
	}


	/**
	 ** Create draw menu
	 **/
	for (i = 0 ; i < XtNumber(menu3) ; i++) {
		strings[i] =  XmStringCreateLocalized(menu3[i]);
	}

	draw_menu = XmVaCreateSimplePulldownMenu(menubar, "draw_menu", 1, draw_cb,
		XmVaTOGGLEBUTTON, strings[0], 'M', NULL, NULL,
		XmVaTOGGLEBUTTON, strings[1], 'S', NULL, NULL,
		XmVaTOGGLEBUTTON, strings[2], 'C', NULL, NULL,
		XmVaTOGGLEBUTTON, strings[3], 'Z', NULL, NULL,
		NULL);

	for (i = 0 ; i < 4 ; i++) {
		char buf[255];
		sprintf(buf, "button_%d", i);
		if (widget = XtNameToWidget(draw_menu, buf)) {
			XmToggleButtonSetState(widget, True, False);
		}
	}
	
	for (i = 0 ; i < XtNumber(menu3) ; i++) {
		XmStringFree(strings[i]);
	}

	XtManageChild(menubar);
}


#if 0
    XmFontList  header_font_list, footer_font_list, legend_font_list;


    graph_win = XtVaCreatePopupShell("Math3D",
                                     xmDialogShellWidgetClass,   parent,
                                     XmNtransient,               FALSE,
                                     XmNallowShellResize,        TRUE,
                                     NULL);
    header_font_list = UseFont(graph_win, "-*-times-bold-i-*-*-*-180-*");
    footer_font_list = UseFont(graph_win, "-*-courier-bold-r-*-*-*-120-*");
    legend_font_list = UseFont(graph_win, "-*-helvetica-medium-r-*-*-*-120-*");
    graph = XtVaCreateManagedWidget("Math3D",
        xtXrt3dWidgetClass,             graph_win,
        XmNtranslations,                XtParseTranslationTable(translations),
#endif

/***************************************************************************
*    stuff from plot.c
***************************************************************************/

static Widget		toplevel;
static Widget		graph_win;
static GC			gc;
static Drawable		pm;
static Range		x_rotate, y_rotate, z_rotate;
static Widget		x_toggle, y_toggle, z_toggle;
static Range		x_scale, y_scale, z_scale;
static Widget		mesh, shade, contour, zone, wire, solid;
static Widget		message, decor, dbuff, xmesh, ymesh;
static Widget		x_xy_plane, x_xz_plane;
static Widget		y_xy_plane, y_yz_plane;
static Widget		z_xz_plane, z_yz_plane;
static Range		perspective;
static int			marker_x = -1, marker_y = -1;

static char		footer_buffer1[100];
static char		footer_buffer2[100];
static char		*footer_ptrs[] = {
	footer_buffer1, footer_buffer2, NULL
};

static void
MoveMarker(int x, int y)
{

	if (! gc)	{
		gc = XCreateGC(XtDisplay(graph), XtWindow(graph), 0, NULL);
		XSetForeground(XtDisplay(graph), gc, 
					   WhitePixel(XtDisplay(graph),
								  DefaultScreen(XtDisplay(graph))));
	}
	if (! pm)	{
		pm = XCreatePixmap(XtDisplay(graph), XtWindow(graph), 3, 3,
						   DefaultDepth(XtDisplay(graph),
										DefaultScreen(XtDisplay(graph))));
	}

	if (marker_x != -1 && marker_y != -1)	{
		XCopyArea(XtDisplay(graph), pm, XtWindow(graph), gc, 0, 0, 3, 3,
				  marker_x - 1, marker_y - 1);
	}
	if (x != -1 && y != -1)	{
		XCopyArea(XtDisplay(graph), XtWindow(graph), pm, gc, x - 1, y - 1,
				  3, 3, 0, 0);
		XFillRectangle(XtDisplay(graph), XtWindow(graph), gc, x-1, y-1, 3, 3);
	}
	marker_x = x;
	marker_y = y;
}

static void
ResetFooter()
{
	char	*footer[3];

	footer[0] = "Use left button to pick a point.";
	footer[1] = "Use right button to map to surface.";
	footer[2] = NULL;

	XtVaSetValues(graph, 
				  XtNxrt3dFooterStrings,	footer,
				  NULL);
}

static void
ClearMarker()
{
	MoveMarker(-1, -1);
	ResetFooter();
}

static void
MovePickMarker(int xindex, int yindex)
{
	Xrt3dPickResult	pick;

	if (xindex != -1) {
		Xrt3dUnpick(graph, xindex, yindex, &pick);
		MoveMarker(pick.pix_x, pick.pix_y);
	}
}

static void
MoveMapMarker(double x, double y, double z)
{
	Xrt3dMapResult	map;

	if (z != XRT3D_HUGE_VAL) {
		Xrt3dUnmap(graph, x, y, z, &map);
		MoveMarker(map.pix_x, map.pix_y);
	}
}

static void
TestPick(Widget xrt_3d, XButtonEvent *event)
{
	Xrt3dPickResult	pick;
	Xrt3dRegion		region;

	sprintf(footer_buffer1, "Pixel: %d, %d", event->x, event->y);

	region = Xrt3dPick(xrt_3d, event->x, event->y, &pick);
	if (region == XRT3D_RGN_IN_GRAPH)	{
		sprintf(footer_buffer2, "Closest Grid Index: %d, %d",
			   pick.xindex, pick.yindex);
	}
	else	{
		switch(region)	{
			case XRT3D_RGN_IN_HEADER:
				sprintf(footer_buffer2, "In header.");
				break;
			case XRT3D_RGN_IN_FOOTER:
				sprintf(footer_buffer2, "In footer.");
				break;
			case XRT3D_RGN_IN_LEGEND:
				sprintf(footer_buffer2, "In legend.");
				break;
			default:
				sprintf(footer_buffer2, "Nowhere.");
				break;
		}
	}

	MovePickMarker(pick.xindex, pick.yindex);
	XtVaSetValues(xrt_3d,
				  XtNxrt3dFooterStrings,	footer_ptrs,
				  NULL);
}

static void
TestMap(Widget xrt_3d, XButtonEvent *event)
{
	Xrt3dMapResult	map;
	Xrt3dRegion		region;

	sprintf(footer_buffer1, "Pixel:   %d, %d", event->x, event->y);
	region = Xrt3dMap(xrt_3d, event->x, event->y, &map);
	if (region == XRT3D_RGN_IN_GRAPH)	{
		if (map.z == XRT3D_HUGE_VAL)	{
			sprintf(footer_buffer2, "Missed Surface");
		}
		else	{
			sprintf(footer_buffer2, "Surface: %g, %g, %g",
				map.x, map.y, map.z);
		}
	}
	else	{
		switch(region)	{
			case XRT3D_RGN_IN_HEADER:
				sprintf(footer_buffer2, "In header.");
				break;
			case XRT3D_RGN_IN_FOOTER:
				sprintf(footer_buffer2, "In footer.");
				break;
			case XRT3D_RGN_IN_LEGEND:
				sprintf(footer_buffer2, "In legend.");
				break;
			default:
				sprintf(footer_buffer2, "Nowhere.");
				break;
		}
	}

	MoveMapMarker(map.x, map.y, map.z);
	XtVaSetValues(xrt_3d,
				  XtNxrt3dFooterStrings,	footer_ptrs,
				  NULL);
}
print_cb(Widget w, XtPointer client_data, XtPointer call_data)
#define DEFAULT_PRINTER "ps"
{
	MtShow3dPrintDialog(graph, DEFAULT_PRINTER, "feedback3d.ps");
} /* print_cb */
