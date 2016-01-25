/*******************************************************************************
*	widget_linebox
*
*	This module provides interactions between gui.c and the linebox widget.
*
*******************************************************************************/

#include <Xm/Xm.h>
#include "gui.h"
#include "LineBox.h"

/* utilities */
static Var *pointsToVar (int num, LBoxPoint_t *in);
static void varToPoints (int *pNum, LBoxPoint_t **ppPoints, Var *var);

/* callback handlers */
void handle_motion (Widget w);
void handle_change (Widget w);

/* resource handlers */
static Var * getPointer   (Widget w);
static Var * getPoints    (Widget w);
static int   setHistogram (Widget w, Var *var);
static int   setPoints    (Widget w, Var *var);
static Var * getHistXMin  (Widget w);
static int   setHistXMin  (Widget w, Var *var);
static Var * getHistXMax  (Widget w);
static int   setHistXMax  (Widget w, Var *var);
static Var * getHistYMin  (Widget w);
static int   setHistYMin  (Widget w, Var *var);
static Var * getHistYMax  (Widget w);
static int   setHistYMax  (Widget w, Var *var);
static Var * getMappedPts (Widget w);
static int   setMappedPts (Widget w, Var *var);

/*******************************************************************************
*	Used by gui_setLineBoxPseudoResources
*******************************************************************************/

/* defines general structure for pseudo-resource handling */
typedef struct pseudo_t {
	char *name;
	Var* (*get)(Widget);
	int  (*set)(Widget, Var *); /* returns 0 if set worked */
} pseudo_t;

/* list of resources that are 'public' i.e. always retrieved by gui.get */
static const char *lineBoxPublicResources[] = {
	LBoxNlineMode,
	LBoxNleftMask,
	LBoxNrightMask,
	LBoxNshowLeftMask,
	LBoxNshowRightMask,
	LBoxNhistXMin,
	LBoxNhistXMax,
	LBoxNhistYMin,
	LBoxNhistYMax
};

/* lists all pseudo-resources and the corresponding read/write handlers */
static const pseudo_t pseudo_resources[] = {
	/* read only resources */
	{"pointer", getPointer, NULL},
	/* write only resources */
	{"histogram", NULL, setHistogram},
	/* read+write resources */
	{"points", getPoints, setPoints},
	{"mappedPoints", getMappedPts, setMappedPts},
	{"histXMin", getHistXMin, setHistXMin},
	{"histXMax", getHistXMax, setHistXMax},
	{"histYMin", getHistYMin, setHistYMin},
	{"histYMax", getHistYMax, setHistYMax},
	{NULL, NULL, NULL}
};

/* lists all pseudo-callbacks */
static CallbackEntry lineBoxCallbacks[] = {
	{ "motion", NULL, NULL, },
	{ "change", NULL, NULL, },
	{ NULL, NULL, NULL }
};

/*******************************************************************************
*	Interfaces to gui.c
*******************************************************************************/

/* passes aliases for this widget to gui_isDefaut, which searches them */
int
gui_isLineBox(const char *name)
{
	const char *aliases[] = { "LineBox", "lineBoxWidgetClass", NULL };
	return gui_isDefault(aliases, name);
}

/* return the class identifier */
WidgetClass
gui_getLineBoxClass(void)
{
	return lineBoxWidgetClass;
}

/* return Narray of public resource names */
Narray *
gui_getLineBoxPublicResources()
{
	Narray *resList;
	int i, num;

	num = sizeof(lineBoxPublicResources) / sizeof(lineBoxPublicResources[0]);
	resList = Narray_create(num);
	for (i = 0; i < num; i++) {
		Narray_add(resList, (char *) lineBoxPublicResources[i], NULL);
	}

	return resList;
}

/* return the list of callbacks */
CallbackList
gui_getLineBoxCallbacks(void)
{
	return lineBoxCallbacks;
}

/* gui_initLineBoxWidget
 *
 * Creates the widget using the default init routine
 * Then adds pseudo callbacks and returns final widget
 */

Widget
gui_initLineBoxWidget (
	const char *dvName,
	WidgetClass class,
	Widget parent,
	Var *dvResources,
	void **optData,
	Narray *publicResources,
	Widget *outerWidget)
{
	/* call default init function */
	Widget lineBox = gui_initDefault (
		dvName, class, parent, dvResources,
		optData, publicResources, outerWidget);
	/* set pseudo callback functions */
	LBoxSetMotionCB (lineBox, handle_motion);
	LBoxSetStretchChangeCB (lineBox, handle_change);
	return lineBox;
}

/* gui_getLineBoxPseudoResources
 *
 * Searches the pseudo_resources array for each resource with a get() function
 * Uses the results of the get() call as the value in a new name/value pair
 * Adds the new pair to the argument dvStruct
 *
 * FIX: this routine is a candidate for generalization into gui.c, replacing
 * much duplicated code under each of the get{widget}PseudoResources() Callbacks.
 */

void
gui_getLineBoxPseudoResources(Widget widget, Var *dvStruct)
{
	int i;
	for (i = 0; pseudo_resources[i].name != NULL; i++) {
		if (pseudo_resources[i].get != NULL) {
			add_struct (dvStruct,
				pseudo_resources[i].name, pseudo_resources[i].get(widget));
		}
	}
}

/*	gui_setLineBoxPseudoResources
 *
 *	Searches the pseudo_resources array for each resource in dvStruct
 *	For each entry in dvStruct which has an entry in pseudo_resources
 *		the corresponding set function is called
 *		the entry is removed from dvStruct
 *		the name is added to the list of public resources
 *
 *	This routine assumes deleting a structure element shifts the next index
 *	into the deleted location's spot
 *
 *	FIX: this routine is a candidate for generalization into gui.c, replacing
 *	much duplicated code under each of the set{widget}PseudoResources() Callbacks.
 */

void
gui_setLineBoxPseudoResources (Widget widget, Var *dvStruct, Narray *publicResources)
{
	int		i,j;
	char	*name;
	Var		*value;

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
}

/*******************************************************************************
*	Callback Handling
*******************************************************************************/

void handle_motion (Widget w)
{
	gui_pseudoCallback (w, "motion");
}

void handle_change (Widget w)
{
	gui_pseudoCallback (w, "change");
}

/*******************************************************************************
*	Resource Handling
*******************************************************************************/

/* returns the the DN/DN pointer position in a new Var struct */
static Var *
getPointer (Widget w)
{
	float x,y;
	Var *ret = create_struct (NULL);
	LBoxGetPointerDN (w, &x, &y);
	add_struct (ret, "x", newFloat(x));
	add_struct (ret, "y", newFloat(y));
	return ret;
}

/* sets the histogram to the argument Val object, clears it if GetY(var)==0 */
static int
setHistogram (Widget w, Var *var)
{
	int x, y, z, num;
	LBoxPoint_t *pPoints;
	x = GetX (var);
	y = GetY (var);
	z = GetZ (var);
	if (V_TYPE(var) != ID_VAL || V_FORMAT(var) != FLOAT || x != 2 || z != 1) {
		parse_error ("widget_linebox.c:setHistogram: Object must be 2xNx1 Var of type FLOAT\n");
		return 1;
	}
	varToPoints (&num, &pPoints, var);
	LBoxSetHistogram (w, num, pPoints);
	free (pPoints);
	return 0;
}

/* returns new Var of points */
static Var *
getPoints (Widget w)
{
	int nPoints;
	LBoxPoint_t *pPoints;
	Var *ret;
	/* get copy of widget's points, convert to Var, free copy, return Var */
	LBoxGetPointsNorm (w, &nPoints, &pPoints);
	ret = pointsToVar (nPoints, pPoints);
	free (pPoints);
	return ret;
}

/* sets widget's points to argument Var */
static int
setPoints (Widget w, Var *var)
{
	LBoxPoint_t *pPoints;
	int x, y, z, nPoints;
	x = GetX (var);
	y = GetY (var);
	z = GetZ (var);
	if (V_TYPE(var) != ID_VAL || x != 2 || z != 1) {
		parse_error ("widget_linebox.c:setPoints: Object must be 2xNx1 Var\n");
		return 1;
	}
	if (y == 0) {
		/* setting empty Var is equivalent to resetting points */
		LBoxResetPoints (w);
	} else {
		/* copy Var to widget point format, set points, free copy */
		varToPoints (&nPoints, &pPoints, var);
		LBoxSetPointsNorm (w, nPoints, pPoints);
		free (pPoints);
	}
}

static Var *
getMappedPts (Widget w)
{
	int nPoints;
	LBoxPoint_t *pPoints;
	Var *ret;
	/* get copy of widget's points, convert to Var, free copy, return Var */
	LBoxGetMappedPoints (w, &nPoints, &pPoints);
	ret = pointsToVar (nPoints, pPoints);
	free (pPoints);
	return ret;
}

static int
setMappedPts (Widget w, Var *var)
{
	int i, x,y,z;
	float *copy;
	x = GetX(var);
	y = GetY(var);
	z = GetZ(var);
	if (V_TYPE(var) != ID_VAL || x != 1 || y < 0 || z != 1) {
		parse_error ("widget_linebox.c:setMappedPoints: Object must be 1xNx1 Var");
		return 1;
	}
	copy = (float *) calloc (y, sizeof(float));
	for (i=0; i<y; i++) {
		copy[i] = extract_float (var, i);
	}
	LBoxSetMappedPoints (w, y, copy);
	free (copy);
}

static Var *
getHistXMin (Widget w)
{
	float xMin;
	XtVaGetValues (w, LBoxNhistXMin, &xMin, NULL);
	return newFloat (xMin);
}

static int
setHistXMin (Widget w, Var *var)
{
	if (V_TYPE(var) != ID_VAL
	|| GetX(var) != 1 || GetY(var) != 1 || GetZ(var) != 1) {
		parse_error ("widget_linebox.c:setHistXMin: Object must be 1x1x1 Var\n");
		return 1;
	}
	gui_setFloatNow (w, LBoxNhistXMin, extract_float (var, 0));
	return 0;
}

static Var *
getHistXMax (Widget w)
{
	float xMax;
	XtVaGetValues (w, LBoxNhistXMax, &xMax, NULL);
	return newFloat (xMax);
}

static int
setHistXMax (Widget w, Var *var)
{
	if (V_TYPE(var) != ID_VAL
	|| GetX(var) != 1 || GetY(var) != 1 || GetZ(var) != 1) {
		parse_error ("widget_linebox.c:setHistXMax: Object must be 1x1x1 Var\n");
		return 1;
	}
	gui_setFloatNow (w, LBoxNhistXMax, extract_float (var, 0));
	return 0;
}

static Var *
getHistYMin (Widget w)
{
	float yMin;
	XtVaGetValues (w, LBoxNhistYMin, &yMin, NULL);
	return newFloat (yMin);
}

static int
setHistYMin (Widget w, Var *var)
{
	if (V_TYPE(var) != ID_VAL
	|| GetX(var) != 1 || GetY(var) != 1 || GetZ(var) != 1) {
		parse_error ("widget_linebox.c:setHistYMin: Object must be 1x1x1 Var\n");
		return 1;
	}
	gui_setFloatNow (w, LBoxNhistYMin, extract_float (var, 0));
	return 0;
}

static Var *
getHistYMax (Widget w)
{
	float yMax;
	XtVaGetValues (w, LBoxNhistYMax, &yMax, NULL);
	return newFloat (yMax);
}

static int
setHistYMax (Widget w, Var *var)
{
	if (V_TYPE(var) != ID_VAL
	|| GetX(var) != 1 || GetY(var) != 1 || GetZ(var) != 1) {
		parse_error ("widget_linebox.c:setHistYMax: Object must be 1x1x1 Var\n");
		return 1;
	}
	gui_setFloatNow (w, LBoxNhistYMax, extract_float (var, 0));
	return 0;
}

/*******************************************************************************
*	Utilities
*******************************************************************************/

/* copies LBoxPoint_t array to Davinci Var */
static Var *
pointsToVar (int num, LBoxPoint_t *in)
{
	int i;
	float *data = calloc (num*2, sizeof(float));
	for (i=0; i<num; i++) {
		data[i*2+0] = in[i].x;
		data[i*2+1] = in[i].y;
	}
	return newVal (BSQ, 2, num, 1, FLOAT, data);
}

/* copies Davinci Var to LBoxPoint_t array */
static void
varToPoints (int *pNum, LBoxPoint_t **ppPoints, Var *var)
{
	int i, x,y,z;
	x = GetX (var);
	y = GetY (var);
	z = GetZ (var);
	if (V_TYPE(var) != ID_VAL || x != 2 || z != 1) {
		*ppPoints = NULL;
		*pNum = 0;
		return;
	}
	*pNum = y;
	*ppPoints = (LBoxPoint_t *) calloc (*pNum, sizeof((*ppPoints)[0]));
	for (i=0; i<*pNum; i++) {
		(*ppPoints)[i].x = extract_float (var, i*2+0);
		(*ppPoints)[i].y = extract_float (var, i*2+1);
	}
}


