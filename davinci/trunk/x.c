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
#include <Xm/ToggleBG.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/Xrt3d.h>
#include <Xm/PushBG.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>

#include "motif_tools.h"

Widget top=NULL;
static Widget		graph = NULL;
static Widget		show_mesh, show_shade, show_contour, show_zone;
XtAppContext	app;
Xrt3dData   *grid = NULL;

/***************************************************************************
*    stuff from plot.c
***************************************************************************/

static GC       gc;
static Drawable pm;

static int			marker_x = -1, marker_y = -1;

static char		footer_buffer1[100];
static char		footer_buffer2[100];
static char		*footer_ptrs[] = {
    footer_buffer1, footer_buffer2, NULL
};

void CreatePopup2(void);

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

void
PrintCB(Widget w, XtPointer client_data, XtPointer call_data)
#define DEFAULT_PRINTER "ps"
{
    MtShow3dPrintDialog(graph, DEFAULT_PRINTER, "feedback3d.ps");
} /* PrintCB */

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
    int new_window = 0;
    Widget parent;

    Alist alist[3];
    alist[0] = make_alist("obj",    ID_VAL,     NULL,     &obj);
    alist[1] = make_alist("new",    INT,     NULL,     &new_window);

    alist[2].name = NULL;

    make_args(&ac, &av, func, arg);
    if (parse_args(ac, av, alist)) return(NULL);

    if (obj == NULL)  {
        parse_error("No argument specified: %s(...obj=...)", av[0]);
        return(NULL);
    }

    x = V_SIZE(obj)[0];
    y = V_SIZE(obj)[1];

    if (grid && graph && !new_window) {
        /*
        ** Reusing the same graph, delete old data
        */
        Xrt3dDestroyData(grid, TRUE);
    }

    grid = Xrt3dMakeGridData(x, y, XRT3D_HUGE_VAL, 
                             x/10.0, y/10.0, 0.0, 0.0, TRUE);

    for (i = 0 ; i < x ; i++) {
        for (j = 0 ; j < y ; j++) {
            grid->g.values[i][j] = extract_double(obj, x*j+i);
        }
    }

    if (graph == NULL) {
        CreatePopup2();
    } else {
        if (new_window) {
            /*
            ** find and name the parent window, to indicate it is inactive
            */
            while((parent = XtParent(graph)) != NULL &&
                   strcmp(XtName(graph), "shellWindow")) {
                graph=parent;
            }
            XtVaSetValues(graph, XtNtitle, "daVinci/XRT3D (inactive)", NULL);
            CreatePopup2();
        }
    }
    XtVaSetValues(graph, XtNxrt3dSurfaceData, grid, NULL);
    return(NULL);
}

enum actions {
    ACTION_PRINT,
    ACTION_QUIT,

    ACTION_MESH,
    ACTION_SHADED,
    ACTION_CONTOURS,
    ACTION_ZONES,
    ACTION_SOLID,

    ACTION_AB_DEMO,
    ACTION_AB_XRT
};

static void MenuCB(Widget w, void * void_item, XtPointer call_data)
{
    Boolean value;
    Widget parent;
    int item = (int) void_item;

    switch (item) {
    case ACTION_MESH:
        XtVaGetValues(w, XmNset, &value,  NULL);
        XtVaSetValues(graph, XtNxrt3dDrawMesh,	value, NULL);
        break;
    case ACTION_SHADED:
        XtVaGetValues(w, XmNset, &value,  NULL);
        XtVaSetValues(graph, XtNxrt3dDrawShaded, value, NULL);
        break;
    case ACTION_CONTOURS:
        XtVaGetValues(w, XmNset, &value,  NULL);
        XtVaSetValues(graph, XtNxrt3dDrawContours, value, NULL);
        break;
    case ACTION_ZONES:
        XtVaGetValues(w, XmNset, &value,  NULL);
        XtVaSetValues(graph, XtNxrt3dDrawZones,	value, NULL);
        break;
    case ACTION_SOLID:
        XtVaGetValues(w, XmNset, &value,  NULL);
        XtVaSetValues(graph, XtNxrt3dSolidSurface, value, NULL);
        break;
    case ACTION_QUIT:
        while ((parent = XtParent(w)) != NULL && strcmp(XtName(w), "shellWindow")) {
            w = parent;
        }
        XtDestroyWidget(w);
        graph = NULL;
        break;
    }
    
}

static MenuItem file_menu[] = {
    { "Print", "Print...", &xmPushButtonGadgetClass, PrintCB, 
      (XtPointer) ACTION_PRINT, 0, NULL},
    { "FileSeparator", NULL, &xmSeparatorGadgetClass, NULL, NULL, 0, NULL},
    { "Quit", "Quit", &xmPushButtonGadgetClass, MenuCB, (XtPointer) ACTION_QUIT,
      0, NULL },
    {NULL},
};



static MenuItem graph_menu[] = {
    { "Mesh", "Mesh", &xmToggleButtonWidgetClass, MenuCB, (XtPointer) ACTION_MESH, 0,
      NULL},
    { "Shaded", "Shaded", &xmToggleButtonWidgetClass, MenuCB, (XtPointer) ACTION_SHADED, 0,
      NULL},
    { "Contours", "Contours", &xmToggleButtonWidgetClass, MenuCB, (XtPointer) ACTION_CONTOURS, 0,
      NULL},
    { "Zones", "Zones", &xmToggleButtonWidgetClass, MenuCB, (XtPointer) ACTION_ZONES, 0,
      NULL},
    { "Solid", "Solid", &xmToggleButtonWidgetClass, MenuCB, (XtPointer) ACTION_SOLID, 0,
      NULL},
    {NULL},
};


static MenuItem help_menu[] = {
    { "Demo", "About spline...", &xmPushButtonGadgetClass, MenuCB, (XtPointer) ACTION_AB_DEMO, 0, NULL},
    { "XRT", "About XRT/3d...", &xmPushButtonGadgetClass, MenuCB, (XtPointer) ACTION_AB_XRT, 0, NULL},
    {NULL},
};

static MenuItem menu_items[] = {
    { "File", "File", NULL, NULL, NULL, 0, file_menu },
    { "Graph", "Graph",  NULL, NULL, NULL, 0, graph_menu },
    { "Help", "Help", NULL, NULL, NULL, 0, help_menu },
    {NULL},
};

static Widget
CreateMain(Widget parent)
{
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

    graph = XtVaCreateManagedWidget("graph",
                                    xtXrt3dWidgetClass,    parent,
                                    XmNtopAttachment,      XmATTACH_WIDGET,
                                    XmNtopAttachment,      XmATTACH_FORM,
                                    XmNbottomAttachment,   XmATTACH_FORM,
                                    XmNleftAttachment,     XmATTACH_FORM,
                                    XmNrightAttachment,    XmATTACH_FORM,
                                    XtNxrt3dSurfaceData,   grid,
                                    XtNxrt3dXAxisTitle,    "X Axis",
                                    XtNxrt3dYAxisTitle,    "Y Axis",
                                    XtNxrt3dZAxisTitle,    "Z Axis",
                                    XtNxrt3dDrawMesh,      TRUE,
                                    XtNxrt3dDrawShaded,    TRUE,
                                    XtNxrt3dDrawContours,  TRUE,
                                    XtNxrt3dDrawZones,     TRUE,
                                    XtNxrt3dHeaderBorder,  XRT3D_BORDER_PLAIN,
                                    XmNtranslations,       XtParseTranslationTable(translations),
                                    NULL);
    return(graph);
}

void
CreatePopup2(void)
{
    Widget base;
    
    base = XtVaAppCreateShell(NULL, "shellWindow",
                              topLevelShellWidgetClass, XtDisplay(top),
                              XtNtitle, "daVinci/XRT3D",
                              NULL);
    
    MtCreateShell(base, NULL, NULL, (Widget (*)())CreateMain, menu_items,
                  NULL, False);

    /*
    ** Set the initial state values for the Graph toggle buttons
    */
    XmToggleButtonSetState(XtNameToWidget(base, "*.Shaded_button"), True, False);
    XmToggleButtonSetState(XtNameToWidget(base, "*.Contours_button"), True, False);
    XmToggleButtonSetState(XtNameToWidget(base, "*.Zones_button"), True, False);
    XmToggleButtonSetState(XtNameToWidget(base, "*.Mesh_button"), True, False);
    
    XtRealizeWidget(base);
}
