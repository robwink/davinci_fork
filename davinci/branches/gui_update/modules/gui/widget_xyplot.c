// TODO: add pseudo-resource reverseButtons to swap left+right button callbacks

/*******************************************************************************
*	widget_xyplot
*
*	This module provides interactions between gui.c and the plot_widgets source.
*	The xyplot widget is capable of drawing 2D curves and histograms.
*	Interaction is limited to write-only setting of curves and histograms
*	  resources.
*
*	Sample davinci script utilizing widget_xyplot:
*
*	load_module('gui')
*	top=gui.create('topLevelShell')
*	b=gui.create('xyplot',top)
*	x1=cat(1//1,2//2,3//3,4//4,5//5,axis=y)
*	y1={points=x1,mark_style="XY_CIRCLE_MARK"}
*	x2=cat(0//0,2//1,4//2,6//3,8//4,axis=y)
*	y2={points=x2,mark_style="XY_X_MARK",mark_size="XY_TINY"}
*	z={test1=y1,test2=y2}
*	gui.set(b,{curves=z})
*	gui.realize(top)
*
*******************************************************************************/

#include <Xm/Xm.h>
#include <X11/keysym.h>
#include "gui.h"
#include "../plot_widgets/plot_widgets/XY.h"

/* TODO: replace these with Xlib defines */
#define SHIFT_MASK 0x01
#define CTRL_MASK  0x04

int setCurves (Widget widget, Var *dvCurves);
Var * getShiftDown (Widget widget);
Var * getCtrlDown (Widget widget);
Var * getXmin (Widget widget);
int setXmin (Widget widget, Var *val);
Var * getXmax (Widget widget);
int setXmax (Widget widget, Var *val);
Var * getYmin (Widget widget);
int setYmin (Widget widget, Var *val);
Var * getYmax (Widget widget);
int setYmax (Widget widget, Var *val);

/*******************************************************************************
*	Used by gui_setXYPlotPseudoResources
*******************************************************************************/

/* defines general structure for pseudo-resource handling */
typedef struct pseudo_t {
	char *name;
	Var* (*get)(Widget);
	int  (*set)(Widget, Var *);
} pseudo_t;

/* lists all pseudo-resources and the corresponding read/write handlers */
pseudo_t pseudo_resources[] = {
	{"curves", NULL, setCurves}, /* curves are write-only */
	{"shiftDown", getShiftDown, NULL}, /* key down state is read only */
	{"ctrlDown", getCtrlDown, NULL}, /* key down state is read only */
	{"xMin", getXmin, setXmin},
	{"xMax", getXmax, setXmax},
	{"yMin", getYmin, setYmin},
	{"yMax", getYmax, setYmax},
	{NULL, NULL, NULL}
};

/*******************************************************************************
*	Callbacks
*******************************************************************************/

static CallbackEntry xyPlotCallbacks[] = {
	{ "btn3Callback",		XmNbtn3Callback,	gui_defaultCallback },
	{ NULL, NULL, NULL }
};

/*******************************************************************************
*	XYPlot Enumerations
*******************************************************************************/

/* maps XY.h enumerations into strings */
typedef struct xy_enum_t {
	int id;
	char *name;
} xy_enum_t;

#define MSTYLE_SKIP 1 /* only skip NO_MARK */

xy_enum_t mark_styles[] = {
	{XY_NO_MARK, "XY_NO_MARK"},
	{XY_SQUARE_MARK, "XY_SQUARE_MARK"},
	{XY_CIRCLE_MARK, "XY_CIRCLE_MARK"},
	{XY_STAR_MARK, "XY_STAR_MARK"},
	{XY_X_MARK, "XY_X_MARK"},
	{XY_TRIANGLE_MARK, "XY_TRIANGLE_MARK"},
	{XY_SOLID_SQUARE_MARK, "XY_SOLID_SQUARE_MARK"},
	{XY_SOLID_CIRCLE_MARK, "XY_SOLID_CIRCLE_MARK"},
	{XY_THICK_SQUARE_MARK, "XY_THICK_SQUARE_MARK"},
	{XY_THICK_CIRCLE_MARK, "XY_THICK_CIRCLE_MARK"},
	{0, NULL}
};

#define MSIZE_DEFAULT XY_MEDIUM

xy_enum_t mark_sizes[] = {
	{XY_TINY, "XY_TINY"},
	{XY_SMALL, "XY_SMALL"},
	{XY_MEDIUM, "XY_MEDIUM"},
	{XY_LARGE, "XY_LARGE"},
	{0, NULL}
};

#define LSTYLE_DEFAULT XY_PLAIN_LINE

xy_enum_t line_styles[] = {
	{XY_NO_LINE, "XY_NO_LINE"},
	{XY_PLAIN_LINE, "XY_PLAIN_LINE"},
	{XY_FINE_DASH, "XY_FINE_DASH"},
	{XY_MED_FINE_DASH, "XY_MED_FINE_DASH"},
	{XY_DASH, "XY_DASH"},
	{XY_LONG_DASH, "XY_LONG_DASH"},
	{XY_X_LONG_DASH, "XY_X_LONG_DASH"},
	{XY_1_DOT_DASH, "XY_1_DOT_DASH"},
	{XY_2_DOT_DASH, "XY_2_DOT_DASH"},
	{XY_3_DOT_DASH, "XY_3_DOT_DASH"},
	{XY_4_DOT_DASH, "XY_4_DOT_DASH"},
	{XY_THICK_LINE, "XY_THICK_LINE"},
	{XY_X_THICK_LINE, "XY_X_THICK_LINE"},
	{0, NULL}
};

/*******************************************************************************
*	Interfaces to gui.c
*******************************************************************************/

/* passes aliases for this wiget to gui_isDefault, which searches them */
int
gui_isXYPlot(const char *name)
{
	const char *aliases[] = { "xyplot", "xyWidgetClass", NULL };
	return gui_isDefault(aliases, name);
}

/* return the class identifier */
WidgetClass
gui_getXYPlotClass(void)
{
	return xyWidgetClass;
}

void
gui_getXYPlotPseudoResources(const Widget widget, Var *dvStruct)
{
	int i;
	dbgprintf ("gui_getPseudoPseudoResources(%ld, %ld)\n", widget, dvStruct);
	for (i=0; pseudo_resources[i].name != NULL; i++) {
		if (pseudo_resources[i].get) {
			add_struct (dvStruct,
				pseudo_resources[i].name, 
				pseudo_resources[i].get(widget));
		}
	}
}

/* use initDefault to create the widget, then replace the translation table
   to swap the 1st and 3rd buttons; we leave the actions alone */
/* TODO: add support for translation resource and let the script do this?
   Dangerous to mess with that table, and annoying seeing it in resources
   when the table is large, but it would be much more general */
Widget
gui_initXYPlot(const char *dvName, WidgetClass widgetClass, Widget parent,
	Var *dvResources, void **optData, Narray *publicResources,
	Widget *outerWidget)
{
	Widget newWidget;
	/* this will effectively swap the 1st and 3rd buttons */
	static char davinciDefaultTranslations[] = 
		"<Btn3Motion>: Motion()\n\
		 <Btn3Down>: Motion()\n\
		 <Btn3Up>: BtnUp()\n\
		 <Btn2Down>: Btn2Press()\n\
		 <Btn1Down>: Btn3Press()\n";
	XtTranslations davinciTranslations;

	dbgprintf ("gui_initXYPlot(dvName = \"%s\", class = %ld, "
		"parent = %ld, optData = %ld, outerWidget = %ld)\n",
		dvName, widgetClass, parent, optData, outerWidget);

	newWidget = gui_initDefault (dvName, widgetClass, parent, dvResources,
		optData, publicResources, outerWidget);

	davinciTranslations = XtParseTranslationTable (davinciDefaultTranslations);
	XtVaSetValues (newWidget, XmNtranslations, davinciTranslations, NULL);

	return newWidget;
}

/* return the class callbacks */
CallbackList
gui_getXYPlotCallbacks(void)
{
	return xyPlotCallbacks;
}

/*	gui_setXYPlotPseudoResources
 *
 *	Searches the pseudo_resources array for each resource in dvStruct
 *	For each entry in dvStruct which has an entry in pseudo_resources
 *		the corresponding set function is called
 *		the entry is removed from dvStruct
 *		the name is added to the list of public resources
 *
 *	This routine relies on structure deletion shifting the next higher index
 *	into the old location's spot
 *
 *	FIX: this routine is a candidate for generalization into gui.c, replacing
 *	much duplicated code under each of the set{widget}PseudoResources() Callbacks.
 */

void
gui_setXYPlotPseudoResources (Widget widget, Var *dvStruct, Narray *publicResources)
{
	int		i,j;
	char	*name;
	Var		*value;

	dbgprintf ("gui_setXYPlotPseudoResources: started\n");

	/* foreach name/value pair in dvStruct */
	i = 0;
	while (i < get_struct_count (dvStruct)) {
		get_struct_element (dvStruct, i, &name, &value);
		j = 0;
		/* handle name/value, delete corresponding dvStruct entry */
		while (pseudo_resources[j].name != NULL) {
			if (!strcmp (name, pseudo_resources[j].name)) {
				if (pseudo_resources[j].set) {
					(*pseudo_resources[j].set) (widget, value);
				}
				/* add to exposed public list, and delete from dvStruct */
				Narray_add(publicResources, name, NULL);
				free_var(Narray_delete(V_STRUCT(dvStruct), name));
				break;
			}
			j ++;
		}
		/* or dont handle this entry, move on to next one */
		if (pseudo_resources[j].name == NULL) {
			i ++;
		}
	}

	dbgprintf ("gui_setXYPlotPseudoResources: finished\n");
}

/*******************************************************************************
*	Helper Functions
*******************************************************************************/

Pixel getPixel (Widget widget, Var *var)
{
	XColor screencolor, exactcolor;
	int result;

	/* verify argument is a string */
	char *colorname = (V_TYPE(var) == ID_STRING ? V_STRING(var) : NULL);
	if (colorname == NULL) {
		parse_error ("Color must be a string");
		return -1;
	}

	/* allocate requested color */
	if (BadColor == XAllocNamedColor (
		XtDisplay(widget),
		DefaultColormapOfScreen(XtScreen(widget)),
		colorname,
		&screencolor, &exactcolor))
	{
		parse_error ("Bad color specified: %s", colorname);
		return -1;
	}

	return screencolor.pixel;
}

/* count enumerations in given set */
int enum_count (xy_enum_t *set)
{
	int i = 0;
	while (set[i].name != NULL) i ++;
	return i;
}

/* return index of enumerated string in the given set */
int enumLookup (xy_enum_t *set, Var *var)
{
	int i;
	char *str = (V_TYPE(var) == ID_STRING ? V_STRING(var) : NULL);
	if (str) {
		for (i=0; set[i].name != NULL; i++) {
			if (!strcmp (set[i].name, str))
				return set[i].id;
		}
	}
	return -1;
}

/*******************************************************************************
*	pseudo resource handling
*******************************************************************************/

/*	setCurveUsage ()
 *
 *	Documents the format of <struct> in:
 *		gui.set (xywidget_id, {curves=<struct>})
 */

void setCurveUsage ()
{
	int i;
	parse_error ("Usage: gui.set (id, {curves=dvCurves});");
	parse_error ("Example:");
	parse_error ("\tpts=cat(1//1,2//2,3//3,4//4,5//5,axis=y)");
	parse_error ("\tcurve1={points=pts}");
	parse_error ("\tcurveset={name1=curve1}");
	parse_error ("\tgui.set(xyplot_id,{curves=curveset})");
	parse_error ("Each element in the dvCurves structure represents one curve.");
	parse_error ("The element name is used as the curve's name in the plot.");
	parse_error ("The element value contains the curve data.");
	parse_error ("The curve data structure contains these elements:");
	parse_error ("	points = ID_VAL");
	parse_error ("	line_color = color string (optional)");
	parse_error ("	mark_color = color string (optional)");
	parse_error ("	mark_style = enumerated string (see mark_styles[]) (optional)");
	parse_error ("	mark_size = enumerated string (see mark_sizes[]) (optional)");
	parse_error ("	line_style = enumerated string (see line_styles[]) (optional)");
	parse_error ("Default mark style is a round-robin selection of each style.");
	parse_error ("The points element is a 2xNx1 Val of (x,y) points to plot.");
	parse_error ("Valid colors include \"#FF0000\" (red), \"blue\", etc.");
	parse_error ("mark_styles[]");
	for (i=0; mark_styles[i].name != NULL; i++)
		parse_error ("\t%s", mark_styles[i].name);
	parse_error ("mark_sizes[]");
	for (i=0; mark_sizes[i].name != NULL; i++)
		parse_error ("\t%s", mark_sizes[i].name);
	parse_error ("line_styles[]");
	for (i=0; line_styles[i].name != NULL; i++)
		parse_error ("\t%s", line_styles[i].name);
}

/*	setCurves
 *
 *	widget: widget to set curves into
 *	dvCurves: Davinci structure
 *	returns 0 on success, non-zero on failure
 *
 *	See setCurveUsage() for dvCurves structure format
 */

int setCurves (Widget widget, Var *dvCurves)
{
	int i,j, x,y,z, size;
	XYCurve *curves = NULL;
	char *curve_name;
	Var *curve;
	Var *points, *line_color, *mark_color, *mark_style, *mark_size, *line_style;

	/* validate arguments */
	/* FIX: check the widget class */
	/*
	if (class of widget is not xyplot) {
		setCurveUsage ();
		return;
	}
	*/
	if (V_TYPE(dvCurves) != ID_STRUCT) {
		setCurveUsage ();
		return 1;
	}
	size = get_struct_count(dvCurves);

	/* validate and repack each curve; fail if any one curve is bad */
	i = 0;
	while (i < size) {
		/* Allocate new curve, init new space, set defaults, retrieve element */
		curves = realloc (curves, (i+1) * sizeof(XYCurve));
		memset (curves + i, 0, sizeof(XYCurve));
		curves[i].horizBars = NULL;
		curves[i].vertBars = NULL;
		get_struct_element (dvCurves, i, &curve_name, &curve);
		curves[i].name = XmStringCreateSimple(curve_name);
		if (V_TYPE(curve) != ID_STRUCT) break;

		/* points (required) */
		if (-1 == find_struct (curve, "points", &points)) break;
		if (V_TYPE(points) != ID_VAL) break;
		x = GetX (points);
		y = GetY (points);
		z = GetZ (points);
		if (x != 2 || z != 1) break;
		curves[i].nPoints = y;
		curves[i].points = (XYPoint *) calloc (y, sizeof(XYPoint));
		for (j=0; j<y; j++) {
			curves[i].points[j].x = extract_float(points, 2*j+0);
			curves[i].points[j].y = extract_float(points, 2*j+1);
		}

		/* line color (optional) */
		if (-1 != find_struct (curve, "line_color", &line_color)) {
			j = getPixel (widget, line_color);
			if (-1 == j) break;
		} else {
			j = BlackPixelOfScreen(XtScreen(widget));
		}
		curves[i].linePixel = j;

		/* mark color (optional) */
		if (-1 != find_struct (curve, "mark_color", &mark_color)) {
			j = getPixel (widget, line_color);
			if (-1 == j) break;
		} else {
			j = BlackPixelOfScreen(XtScreen(widget));
		}
		curves[i].markerPixel = j;

		/* mark style (optional)
		   This is selected round robin for curves with unspecified styles */
		if (-1 != find_struct (curve, "mark_style", &mark_style)) {
			j = enumLookup (mark_styles, mark_style);
			if (-1 == j) break;
		} else {
			j = i % (enum_count (mark_styles) - MSTYLE_SKIP) + MSTYLE_SKIP;
		}
		curves[i].markerStyle = j;

		/* mark size (optional) */
		if (-1 != find_struct (curve, "mark_size", &mark_size)) {
			j = enumLookup (mark_sizes, mark_size);
			if (-1 == j) break;
		} else {
			j = MSIZE_DEFAULT;
		}
		curves[i].markerSize = j;

		/* line style (optional) */
		if (-1 != find_struct (curve, "line_style", &line_style)) {
			j = enumLookup (line_styles, line_style);
			if (-1 == j) break;
		} else {
			j = LSTYLE_DEFAULT;
		}
		curves[i].lineStyle = j;
		i ++;
	}

	if (i < size) {
		/* if not all curves were valid, show usage */
		setCurveUsage();
	} else {
		/* actually set the curves */
		XYSetCurves (widget, curves, size, XY_RESCALE, True);
	}

	/* free resources */
	size = i;
	for (i=0; i<size; i++) {
		if (curves[i].name) XmStringFree(curves[i].name);
		if (curves[i].points) free (curves[i].points);
	}
	if (curves) free (curves);
	return 0;
}

/* get shiftDown state */
Var *
getShiftDown (Widget widget)
{
	Window root_return, child_return;
	int root_x, root_y, win_x, win_y, mask, result;
	char *shiftDown;
	result = XQueryPointer (XtDisplay(widget), XtWindow(widget),
		&root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask);
	shiftDown = (mask & SHIFT_MASK ? "True" : "False");
	return newString(strdup(shiftDown));
}

/* get ctrlDown state */
Var *
getCtrlDown (Widget widget)
{
	Window root_return, child_return;
	int root_x, root_y, win_x, win_y, mask, result;
	char *ctrlDown;
	result = XQueryPointer (XtDisplay(widget), XtWindow(widget),
		&root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask);
	ctrlDown = (mask & CTRL_MASK ? "True" : "False");
	return newString(strdup(ctrlDown));
}

/* get/set xMin state */
Var *
getXmin (Widget widget)
{
	double xMin, xMax, yMin, yMax;
	XYGetVisibleRange(widget, &xMin, &yMin, &xMax, &yMax);
	return newFloat(xMin);
}

int
setXmin (Widget widget, Var *val)
{
	double xMin, xMax, yMin, yMax;
	XYGetVisibleRange(widget, &xMin, &yMin, &xMax, &yMax);
	xMin = extract_double (val, 0);
	XYSetVisibleRange(widget, xMin, yMin, xMax, yMax);
}

/* get/set xMax state */
Var *
getXmax (Widget widget)
{
	double xMin, xMax, yMin, yMax;
	XYGetVisibleRange(widget, &xMin, &yMin, &xMax, &yMax);
	return newFloat(xMax);
}

int
setXmax (Widget widget, Var *val)
{
	double xMin, xMax, yMin, yMax;
	XYGetVisibleRange(widget, &xMin, &yMin, &xMax, &yMax);
	xMax = extract_double (val, 0);
	XYSetVisibleRange(widget, xMin, yMin, xMax, yMax);
}

/* get/set yMin state */
Var *
getYmin (Widget widget)
{
	double xMin, xMax, yMin, yMax;
	XYGetVisibleRange(widget, &xMin, &yMin, &xMax, &yMax);
	return newFloat(yMin);
}

int
setYmin (Widget widget, Var *val)
{
	double xMin, xMax, yMin, yMax;
	XYGetVisibleRange(widget, &xMin, &yMin, &xMax, &yMax);
	yMin = extract_double (val, 0);
	XYSetVisibleRange(widget, xMin, yMin, xMax, yMax);
}

/* get/set yMax state */
Var *
getYmax (Widget widget)
{
	double xMin, xMax, yMin, yMax;
	XYGetVisibleRange(widget, &xMin, &yMin, &xMax, &yMax);
	return newFloat(yMax);
}

int
setYmax (Widget widget, Var *val)
{
	double xMin, xMax, yMin, yMax;
	XYGetVisibleRange(widget, &xMin, &yMin, &xMax, &yMax);
	yMax = extract_double (val, 0);
	XYSetVisibleRange(widget, xMin, yMin, xMax, yMax);
}


