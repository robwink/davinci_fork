/* vicar_widget.c
 *
 * Copyright 2003
 * Mars Space Flight Facility
 * Department of Geological Sciences
 * Arizona State University
 *
 * Jim Stewart <Jim.Stewart@asu.edu>
 *
 * Portions of this code are based on test_iw.c, part of the VICAR
 * software package.
 *
 */

/* FIX: TODO
 *
 * o XvicImageFreeGC() on all allocated GCs during cleanup.
 *
 */

#include "gui.h"

#ifdef HAVE_LIBVICAR

#include "widget_vicar.h"

#include <Xm/Form.h>

/* Some macros. */

#ifndef MIN
#define MIN(x,y) ((x)<(y) ? (x) : (y))
#endif

#ifndef ABS
#define ABS(x) ((x)>=0 ? (x) : (-(x)))
#endif

/* Some constants. */

/* FIX: not sure what this constant should be named.. */
#define DV_VICAR_WIDGET_APP_NAME "vicar"
#define DV_VICAR_IMAGE_PSEUDORESOURCE "image"
#define DV_VICAR_TILE_SIZE 256
#define DV_VICAR_OVERLAY_COLOR "#FFFFFF"

/* VicarGraphics structure used for drawing on the overlay. */

typedef struct _VicarGraphics {
  XvicID		vicarOverlayId;
  int			davinciOverlayId;
  XvicColor		color;
  double		x, y;
  Pixmap		pixmap;
  struct _VicarGraphics *prev;
  struct _VicarGraphics *next;
} VicarGraphics;

/* TimerData is used during overlay drawing. */

typedef struct {
  Widget	widget;
  XEvent	event;
  int		x, y;
} TimerData;

/* inputMode values. */

typedef enum {
  DV_VICAR_INPUT_SELECT = 1,
  DV_VICAR_INPUT_POINT
} VicarInputMode;

/* Widget instance data struct.  This gets stored in WidgetListEntry.data. */

typedef struct {

  Widget		imageWidget;
  Var			*imageData;
  VicarInputMode	inputMode;
  double		pointX, pointY;

  /* Selection overlays are for programatically or interactively set selection
   * area demarcation.  All other overlays are for static programmatically set
   * use only, and the Davinci user must select and maintain the Davinci ID
   * (which is mapped to the VICAR overlay object ID).
   */

  XvicColor		defaultOverlayColor;
  XvicGC		overlayGC;
  XvicGC		rubberBandGC;
  double		selectionX, selectionY;
  double		selectionWidth, selectionHeight;
  VicarGraphics 	*selectionOverlay;
  VicarGraphics 	*selectionOverlayRubberBand;
  XtIntervalId		timeoutId;
  VicarGraphics		*overlaysHead;
  VicarGraphics		*overlaysTail;

} VicarData;

/* Property get/set functions specific to the VICAR widget. */

static Var *		getZoom(Widget);
static void		setZoom(Widget, Var *);
static Var *		getImage(Widget);
static unsigned int	setImage(Widget, Var *);
static Var *		getSelection(Widget);
static void		setSelection(Widget, Var *);
static Var *		getPoint(Widget);
static Var *		getInputMode(Widget);
static void		setInputMode(Widget, Var *);
static void		setOverlay(Widget, Var *);

/* Miscellaneous private function prototypes. */

static void 	handleExpose(Widget, XtPointer, XtPointer);
static XvicGC	createNewGC(Widget, int);
static void 	handleInput(Widget, XtPointer, XtPointer);
#if 0
static void 	drawObjectStart(VicarGraphics *, double, double);
#endif
static void 	drawObject(Widget, VicarData *, XvicGC, double, double, int);
static void 	autoPan(XtPointer, XtIntervalId *);
static Var *	convertToBSQ(Var *);

/* Overlay support functions. */

static void		addVicarGraphics(VicarData *, XvicID, int, XvicColor,
					 double, double, Pixmap);
static void		deleteVicarGraphicsObject(VicarData *, VicarGraphics *,
						  Boolean);
static VicarGraphics *	getVicarGraphicsByDavinciId(VicarData *, int);

/* Public resources. */

static const char *vicarPublicResources[] = {
  "zoom", "selection",
};

/*****************************************************************************
 *
 * CALLBACKS
 *
 *****************************************************************************/

static CallbackEntry vicarCallbacks[] = {
  /* These are pseudo-callbacks, called directly by code below. */
  {
    "select",
    NULL,
    NULL,
  },
  {
    "point",
    NULL,
    NULL,
  },
  /* End of pseudo-callbacks. */
  { NULL, NULL, NULL }
};

/* Function definitions. */

CallbackList
gui_getVicarCallbacks(void)
{
  return vicarCallbacks;
}

int
gui_isVicar(const char *name)
{
  const char *aliases[] = { "vicar", "xvicBasicImageWidgetClass", NULL };
  return gui_isDefault(aliases, name);
}

WidgetClass
gui_getVicarClass(void)
{
  return xvicBasicImageWidgetClass;
}

Widget
gui_initVicar(const char *dvName, WidgetClass class, Widget parent,
	      Var *dvResources, void **instanceData, Narray *publicResources,
	      Widget *outerWidget)
{

  Widget		newWidget;
  VicarData		*vicarData;
  XvicColor		vicarColor;
  XColor		color;
  Arg			xtArgs[DV_MAX_XT_ARGS];
  Cardinal		xtArgCount;
  FreeStackListEntry	freeStack;
  Var			*imageData;
  int			idx;
  int			tileWidth = 0, tileHeight = 0;
  VicarGraphics		*selectionOverlay, *selectionOverlayRubberBand;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_initVicar(dvName = '%s', class = %ld, "
	  "parent = %ld, dvResources = %ld, instanceData = %ld, "
	  "publicResources = %ld\n",
	  dvName, class, parent, instanceData, publicResources);
#endif

  /* Parse resources, if the user supplied any. */

  /* First make sure there's an image. */

  idx = find_struct(dvResources, DV_VICAR_IMAGE_PSEUDORESOURCE, &imageData);
  if (idx == -1) {
    /* Not found; fail now. */
    parse_error("Cannot create VICAR widget without image resource.");
    return NULL;
  }
  /* Got one, will be set later in this function. */

  /* Parse anything else that might be a real Xt resource. */

  xtArgCount = 0;
  freeStack.head = freeStack.tail = NULL; /* FIX: free these */
  if (dvResources != NULL) {
    gui_setResourceValues(NULL, widgetClass, dvResources,
			  xtArgs, &xtArgCount, &freeStack,
			  publicResources);
  }

#if DEBUG
  fprintf(stderr, "DEBUG: XvicCreateImage()\n");
#endif
  newWidget = XvicCreateImage(parent, DV_VICAR_WIDGET_APP_NAME,
			      xtArgs, xtArgCount);

  /* Free anything in the free stack. */
  gui_freeStackFree(&freeStack);

  if (newWidget == NULL) {
    parse_error("Error: unable to create new VICAR widget.");
    return NULL;
  }

  /* Specify some defaults if the user didn't provide them. */
  /* FIX: need a generic way of doing this. */

  /* Default tileWidth and tileHeight so drawing is reasonable fast. */

  XtVaGetValues(newWidget, XvicNtileWidth, &tileWidth,
		XvicNtileHeight, &tileHeight, NULL);

  if (tileWidth < DV_VICAR_TILE_SIZE) {
    XtVaSetValues(newWidget, XvicNtileWidth, DV_VICAR_TILE_SIZE, NULL);
  }
  if (tileHeight < DV_VICAR_TILE_SIZE) {
    XtVaSetValues(newWidget, XvicNtileHeight, DV_VICAR_TILE_SIZE, NULL);
  }

  /* Default overlay color. */

  XParseColor(XtDisplay(newWidget),
	      DefaultColormapOfScreen(XtScreen(newWidget)),
	      DV_VICAR_OVERLAY_COLOR, &color);
  vicarColor = XvicImageGetGrColor(newWidget, &color);

  /* Private Vicar data structure for this particular instance. */
  vicarData = (VicarData *) malloc(sizeof(VicarData));

  /* Widget stuff. */
  vicarData->imageWidget = newWidget;

  /* Image stuff. */
  vicarData->imageData = NULL;

  /* Mode stuff. */
  vicarData->inputMode = DV_VICAR_INPUT_POINT;

  /* Point selection stuff. */
  vicarData->pointX = 0.0;
  vicarData->pointY = 0.0;

  /* Overlay stuff. */
  vicarData->defaultOverlayColor = vicarColor;
  vicarData->overlaysHead = NULL;
  vicarData->overlaysTail = NULL;

  /* Selection data. */
  vicarData->selectionX = 0.0;
  vicarData->selectionY = 0.0;
  vicarData->selectionWidth = 0.0;
  vicarData->selectionHeight = 0.0;
  vicarData->timeoutId = 0;
  vicarData->overlayGC = createNewGC(newWidget, False);
  vicarData->rubberBandGC = createNewGC(newWidget, True);

  /* Selection overlays. */
  selectionOverlay = (VicarGraphics *) malloc(sizeof(VicarGraphics)); /* FIX: errchk */
  vicarData->selectionOverlay = selectionOverlay;
  selectionOverlay->vicarOverlayId = 0;
  selectionOverlay->davinciOverlayId = 0;
  selectionOverlay->color = vicarColor;
  selectionOverlay->x = 0.0;
  selectionOverlay->y = 0.0;
  selectionOverlay->pixmap = 0;
  selectionOverlay->prev = NULL;
  selectionOverlay->next = NULL;
  selectionOverlayRubberBand = (VicarGraphics *) malloc(sizeof(VicarGraphics)); /* FIX: errchk */
  vicarData->selectionOverlayRubberBand = selectionOverlayRubberBand;
  selectionOverlayRubberBand->vicarOverlayId = 0;
  selectionOverlayRubberBand->davinciOverlayId = 0;
  selectionOverlayRubberBand->color = vicarColor;
  selectionOverlayRubberBand->x = 0.0;
  selectionOverlayRubberBand->y = 0.0;
  selectionOverlayRubberBand->pixmap = 0;
  selectionOverlayRubberBand->prev = NULL;
  selectionOverlayRubberBand->next = NULL;

  /* Store instance data in feedback pointer. */
  *instanceData = vicarData;

  /* Set the outer widget feedback var so container classes work right. */
  *outerWidget = XtParent(newWidget);

  /* Setup the internal callbacks. */
  XtAddCallback(newWidget, XvicNexposeCallback, handleExpose, NULL);
  XtAddCallback(newWidget, XvicNinputCallback, handleInput, NULL);

  /* Manage the new widget. */
  XtManageChild(newWidget);

#if DEBUG
  fprintf(stderr, "DEBUG: leaving gui_initVicar(), newWidget = %ld\n", newWidget);
#endif

  return newWidget;

}

/* Compound resource functions to handle vicar zoom settings. */
/* Combining XvicNxZoomIn, XvicNyZoomIn into single "zoom".   */

Var *
getZoom(Widget widget) {

  Var	*o;
  int	xZoomIn, xZoomOut, zoom;

  /* XvicNimageZoom cannot be read, so we reconstruct the value manually.
   * It is assumed that the user has set the zoom using "zoom" pseudo-resource
   * (setZoom() function below), and that X and Y zooms are the same.  If the
   * user has set the real Xt resources directly, this assumption is invalid,
   * and the user should not rely on this value, as it reflects only the X
   * axis zoom.
   */

  XtVaGetValues(widget, XvicNxZoomIn, &xZoomIn, XvicNxZoomOut, &xZoomOut, NULL);
  if (xZoomIn == 1) {
    if (xZoomOut == 1) {
      zoom = 1;
    }
    else {
      zoom = -(xZoomOut);
    }
  }
  else {
    zoom = xZoomIn;
  }
  o = newInt(zoom);

  return o;

}

void
setZoom(Widget widget, Var *value)
{

  if (V_TYPE(value) != ID_VAL || V_FORMAT(value) > INT) {
    parse_error("VICAR zoom resource must be an integer");
    return;
  }

  XtVaSetValues(widget, XvicNimageZoom, V_INT(value), NULL);

}

/* FIX: disabling this til gui.get(widgetid, resourceName) works.  Copying the
 * entire image on every gui.get() is too inefficient.
 */

Var *
getImage(Widget widget) {

  MyWidgetList	widgetListEntry;
  VicarData	*vicarData;
  Var 		*imageData, *o;

#if DEBUG
  fprintf(stderr, "DEBUG: getImage(widget = %ld)\n", widget);
#endif

  widgetListEntry = gui_getWidgetListEntryFromWidget(widget);

  /* FIX: can probably remove this check. */
  if (widgetListEntry == NULL) {
    parse_error("Internal error: invalid widget in getImage().");
    return newInt(0);
  }

  vicarData = (VicarData *) widgetListEntry->data;
  imageData = vicarData->imageData;

#if DEBUG
  fprintf(stderr, "DEBUG: imageData = %ld\n", imageData);
#endif

  if (imageData == NULL) {
    o = newInt(0);
  }
  else {
    o = V_DUP(imageData);
  }

#if DEBUG
  fprintf(stderr, "DEBUG: o = %ld\n", widget);
#endif

  return o;

}

/* setImage(widget, value)
 *
 * Replaces the current VICAR image with the one in value, which should be a
 * Davinci Var *.
 *
 * Returns 1 on success, 0 on failure.
 *
 */

static unsigned int
setImage(Widget widget, Var *value) {

  unsigned char	dataType;
  Arg		xtArgs[20];
  int		n;
  MyWidgetList	widgetListEntry;
  VicarData	*vicarData;
  XvicImageData	vicImageData;
  Var		*newValue, *oldImageData;
  Dimension	viewWidth, viewHeight;
  int		tileWidth, tileHeight, imageWidth, imageHeight, imageDepth;

#if DEBUG
  fprintf(stderr, "DEBUG: setImage(widget = %ld, value = %ld)\n",
	  widget, value);
#endif

  if (V_TYPE(value) != ID_VAL) {
    parse_error("VICAR image resource must be image data.");
    return 0;
  }

  imageDepth = GetZ(value);

  if (imageDepth !=1 && imageDepth !=3) {
    parse_error("VICAR image data must be 1 or 3 band.");
    return 0;
  }

  switch (V_FORMAT(value)) {
  case BYTE:
    dataType = XvicBYTE;
    break;
  case SHORT:
    dataType = XvicHALF;
    break;
  case INT:
    dataType = XvicFULL;
    break;
  case FLOAT:
    dataType = XvicREAL;
    break;
  case DOUBLE:
    dataType = XvicDOUBLE;
    break;
  default:
    parse_error("VICAR image data must be BYTE, SHORT, INT, FLOAT, or DOUBLE.");
    return 0;
  }

  widgetListEntry = gui_getWidgetListEntryFromWidget(widget);
  vicarData = (VicarData *) widgetListEntry->data;

  oldImageData = vicarData->imageData;

  /* Make sure we've got a BSQ image.  */
  if (V_ORG(value) != BSQ) {
#if DEBUG
    fprintf(stderr, "DEBUG: converting image to BSQ\n");
#endif
    newValue = convertToBSQ(value);
    mem_claim(newValue);
#if DEBUG
    fprintf(stderr, "DEBUG: newValue = %ld\n", newValue);
#endif
    /* FIX: free old value? */
    value = newValue;
  }

  /* This memory is already claimed, so use the existing copy. */
#if DEBUG
  fprintf(stderr, "DEBUG: new imageData = %ld\n", value);
#endif
  vicarData->imageData = value;

  /* Get some existing resource values for bounds checking. */

  XtVaGetValues(widget,
		XvicNviewWidth, &viewWidth,
		XvicNviewHeight, &viewHeight,
		NULL);

  imageWidth = GetX(value);
  imageHeight = GetY(value);
  imageDepth = GetZ(value); /* FIX: should not have changed; remove */

#if DEBUG
  fprintf(stderr, "DEBUG: imageWidth = %d\n", imageWidth);
  fprintf(stderr, "DEBUG: imageHeight = %d\n", imageHeight);
  fprintf(stderr, "DEBUG: imageDepth = %d\n", imageDepth);
#endif

  /* Give the new image to the widget. */

  vicImageData.x = 0;
  vicImageData.y = 0;
  vicImageData.width = imageWidth;
  vicImageData.height = imageHeight;
  vicImageData.memory_control = XvicMEMORY_SHARED;
  vicImageData.line_width = imageWidth;
  vicImageData.start_offset = 0;

  if (imageDepth == 1) {
#if DEBUG
    fprintf(stderr, "DEBUG: greyscale image\n");
#endif
    vicImageData.bw_pixels = (unsigned char *) V_DATA(value);
  }
  else {
#if DEBUG
    fprintf(stderr, "DEBUG: color image\n");
    fprintf(stderr, "DEBUG: invalidating bw_pixels\n");
    vicImageData.bw_pixels = NULL;
#endif
    vicImageData.red_pixels = (unsigned char *) V_DATA(value);
    vicImageData.grn_pixels = ((unsigned char *) V_DATA(value)) +
      (vicImageData.width * vicImageData.height);
    vicImageData.blu_pixels = ((unsigned char *) V_DATA(value)) +
      (vicImageData.width * vicImageData.height * 2);
  }

  /* Set resources.
   * FIX: Not sure if this should be done before XvicImageWrite(), after,
   * or not at all.
   */

  n = 0;
  XtSetArg(xtArgs[n], XvicNimageWidth, (XtArgVal) imageWidth); n++;
  XtSetArg(xtArgs[n], XvicNimageHeight, (XtArgVal) imageHeight); n++;
  XtSetArg(xtArgs[n], XvicNdataType, (XtArgVal) dataType); n++;
  XtSetArg(xtArgs[n], XvicNimageMode, (XtArgVal) (imageDepth == 1 ? XvicBW : XvicCOLOR)); n++;
  /* FIX: this probably needs to vary depending on the X server setting.. */
  XtSetArg(xtArgs[n], XvicNvisualType, (XtArgVal) XvicUSE_24BIT); n++;

  /* These need to be explicitly set because the widget sets them to 1 upon
   * creation, when there's no image.
   */
  tileWidth = MIN(DV_VICAR_TILE_SIZE, imageWidth);
  tileHeight = MIN(DV_VICAR_TILE_SIZE, imageHeight);
  XtSetArg(xtArgs[n], XvicNtileWidth, (XtArgVal) tileWidth); n++;
  XtSetArg(xtArgs[n], XvicNtileHeight, (XtArgVal) tileHeight); n++;
  viewWidth = MIN(viewWidth, imageWidth);
  viewHeight = MIN(viewHeight, imageHeight);
  XtSetArg(xtArgs[n], XvicNviewWidth, (XtArgVal) viewWidth); n++;
  XtSetArg(xtArgs[n], XvicNviewHeight, (XtArgVal) viewHeight); n++;

  XtSetValues(widget, xtArgs, n);

#if DEBUG
  fprintf(stderr, "DEBUG: calling XvicImageWrite()\n");
#endif
  XvicImageWrite(widget, &vicImageData, True);

  /* Free the old image, if any. */
  if (oldImageData != NULL) {
#if DEBUG
    fprintf(stderr, "DEBUG: freeing old imageData (%ld)\n", vicarData->imageData);
#endif
    free_var(oldImageData);
  }

#if DEBUG
  fprintf(stderr, "DEBUG: leaving setImage()\n");
#endif
  return 1;

}

static Var *
getSelection(Widget widget) {

  MyWidgetList	widgetPtr = gui_getWidgetListEntryFromWidget(widget);
  VicarData	*vicarData = (VicarData *) widgetPtr->data;
  Var		*o = new_struct(0);

  add_struct(o, "x", newDouble(vicarData->selectionX));
  add_struct(o, "y", newDouble(vicarData->selectionY));
  add_struct(o, "width", newDouble(vicarData->selectionWidth));
  add_struct(o, "height", newDouble(vicarData->selectionHeight));

  return o;

}

static Var *
getPoint(Widget widget) {

  MyWidgetList	widgetPtr = gui_getWidgetListEntryFromWidget(widget);
  VicarData	*vicarData = (VicarData *) widgetPtr->data;
  Var		*o = new_struct(0);

  add_struct(o, "x", newDouble(vicarData->pointX));
  add_struct(o, "y", newDouble(vicarData->pointY));

  return o;

}

static Var *
getInputMode(Widget widget) {

  MyWidgetList	widgetPtr = gui_getWidgetListEntryFromWidget(widget);
  VicarData	*vicarData = (VicarData *) widgetPtr->data;
  char		*inputModeString;

  switch (vicarData->inputMode) {
  case DV_VICAR_INPUT_POINT:
    inputModeString = "MODE_POINT";
    break;
  case DV_VICAR_INPUT_SELECT:
    inputModeString = "MODE_SELECT";
    break;
#if 0
  case DV_VICAR_INPUT_DRAW:
    inputModeString = "MODE_DRAW";
    break;
#endif
  default:
    parse_error("Internal Error: invalid inputMode in VICAR widget.");
    inputModeString = "MODE_INVALID";
    break;
  }

  return newString(dupString(inputModeString));

}

static void
setInputMode(Widget widget, Var *value)
{

  MyWidgetList		widgetListEntry;
  VicarData		*vicarData;
  char			*inputModeString;
  VicarInputMode	inputMode;

  if (V_TYPE(value) != ID_STRING) {
    parse_error("Error: VICAR inputMode resource must be a string.");
    return;
  }

  widgetListEntry = gui_getWidgetListEntryFromWidget(widget);
  if (widgetListEntry == NULL) {
    /* This isn't ever supposed to happen. */
    parse_error("Internal Error: unknown widget in widget list.");
    return;
  }

  vicarData = (VicarData *) widgetListEntry->data;
  if (vicarData == NULL) {
    /* This isn't ever supposed to happen. */
    parse_error("Internal Error: unknown widget in widget list.");
    return;
  }

  /* Clear any selection or point. */
  if (vicarData->selectionOverlay->vicarOverlayId) {
    XvicImageEraseObject(widget, vicarData->selectionOverlay->vicarOverlayId);
    vicarData->selectionOverlay->vicarOverlayId = 0;
    vicarData->selectionX = 0.0;
    vicarData->selectionY = 0.0;
    vicarData->selectionWidth = 0.0;
    vicarData->selectionHeight = 0.0;
  }
  vicarData->pointX = 0.0;
  vicarData->pointY = 0.0;

  inputMode = vicarData->inputMode;
  inputModeString = V_STRING(value);

  if (!strcmp(inputModeString, "MODE_POINT")) {
    inputMode = DV_VICAR_INPUT_POINT;
  }
  else if (!strcmp(inputModeString, "MODE_SELECT")) {
    inputMode = DV_VICAR_INPUT_SELECT;
  }
#if 0
  else if (!strcmp(inputModeString, "MODE_DRAW")) {
    inputMode = DV_VICAR_INPUT_DRAW;
  }
#endif
  else {
    parse_error("Error: attempt to set invalid VICAR inputMode.");
  }

  vicarData->inputMode = inputMode;

  return;

}

static void
setSelection(Widget widget, Var *value)
{

  VicarData	*vicarData;
  Var		*vX, *vY, *vWidth, *vHeight;
  double	x, y, width, height;
  int		numElements;
  VicarGraphics	*graphics;
  XvicID	selectionId;

  if (V_TYPE(value) != ID_STRUCT) {
    parse_error("VICAR image selection must be a struct.");
    return;
  }

  numElements = get_struct_count(value);

  if (numElements) {

    if (numElements != 4) {
      parse_error("VICAR image selection must contain x, y, width, and height values or be empty.");
      return;
    }

    if (find_struct(value, "x", &vX) < 0) {
      parse_error("VICAR image selection must contain x value.");
      return;
    }
    x = extract_double(vX, 0);

    if (find_struct(value, "y", &vY) < 0) {
      parse_error("VICAR image selection must contain y value.");
      return;
    }
    y = extract_double(vY, 0);

    if (find_struct(value, "width", &vWidth) < 0) {
      parse_error("VICAR image selection must contain width value.");
      return;
    }
    width = extract_double(vWidth, 0);

    if (find_struct(value, "height", &vHeight) < 0) {
      parse_error("VICAR image selection must contain height value.");
      return;
    }
    height = extract_double(vHeight, 0);

  }

  /* FIX: make sure new selection coordinates are within image bounds! */

  /* Cancel any existing auto-pan task. */

  vicarData = (VicarData *) gui_getWidgetListEntryFromWidget(widget)->data;

  if (vicarData->timeoutId) {
    XtRemoveTimeOut(vicarData->timeoutId);
    vicarData->timeoutId = 0;
  }

  /* Delete existing rubberband object (if any; unlikely but possible). */

  if (vicarData->selectionOverlayRubberBand->vicarOverlayId) {
    XvicImageEraseObject(widget,
			 vicarData->selectionOverlayRubberBand->vicarOverlayId);
    vicarData->selectionOverlayRubberBand->vicarOverlayId = 0;
  }

  /* Delete existing selection object. */

  if (vicarData->selectionOverlay->vicarOverlayId) {
    XvicImageEraseObject(widget,
			 vicarData->selectionOverlay->vicarOverlayId);
    vicarData->selectionOverlay->vicarOverlayId = 0;
  }

  if (numElements) {
    /* Draw the new object. */
    vicarData->selectionX = x;
    vicarData->selectionY = y;
    vicarData->selectionWidth = width;
    vicarData->selectionHeight = height;
    graphics = vicarData->selectionOverlay;
    selectionId = XvicImageDrawRectangle(widget, 0, vicarData->overlayGC,
					 graphics->color, x, y, width, height);
    graphics->vicarOverlayId = selectionId;
  }
  else {
    /* Empty selection struct; clear selection. */
    vicarData->selectionX = 0;
    vicarData->selectionY = 0;
    vicarData->selectionWidth = 0;
    vicarData->selectionHeight = 0;
  }

  return;

}

void
setOverlay(Widget widget, Var *value) {

  MyWidgetList	widgetPtr;
  Var		*vId, *vX, *vY, *vColor, *vBitmap;
#if 0
  Var		*vShape;
#endif
  int		davinciOverlayId;
  double	x, y;
  char		*dvData;
  unsigned char	*bitmapData;
  int		width, height;
  char		*colorString;
  XColor	color;
  Status	colorStatus;
  XvicColor	vicarColor;
  Pixmap	pixmap;
  VicarData	*vicarData;
  int		row, col, bytesPerLine;
  XvicID	vicarOverlayId;
  VicarGraphics	*graphics;
  Boolean	delete = True, redraw = False, move = False, new = False;

  /* Parse required arguments. */

  /* Extract object Id. */
  if (V_TYPE(value) != ID_STRUCT) {
    parse_error("VICAR overlay: must be a struct.");
    return;
  }

  widgetPtr = gui_getWidgetListEntryFromWidget(widget);
  vicarData = (VicarData *) widgetPtr->data;

  if (get_struct_count(value) == 0) {
#if DEBUG
    fprintf(stderr, "DEBUG: deleting all overlays\n");
    fprintf(stderr, "DEBUG: head = %ld\n", vicarData->overlaysHead);
#endif
    /* Delete all overlays. */
    while (vicarData->overlaysHead != NULL) {
#if DEBUG
      fprintf(stderr, "DEBUG: deleting overlay %ld\n", vicarData->overlaysHead);
#endif
      deleteVicarGraphicsObject(vicarData, vicarData->overlaysHead, True);
    }
    return;
  }

  if (find_struct(value, "id", &vId) < 0 ||
      V_TYPE(vId) != ID_VAL ||
      V_FORMAT(vId) > INT) {
    parse_error("VICAR overlay: must be a struct containing id (integer).");
    return;
  }

  davinciOverlayId = V_INT(vId);

  /* See if this is an existing object. */
  graphics = getVicarGraphicsByDavinciId(vicarData, davinciOverlayId);
  if (graphics == NULL) {
#if DEBUG
    fprintf(stderr, "DEBUG: new object\n");
#endif
    redraw = True;
    new = True;
    delete = False;
  }
#if DEBUG
  else {
    fprintf(stderr, "DEBUG: reusing object\n");
  }
#endif

  /* Parse optional arguments. */

  /* Extract X location. */
  /* FIX: make sure it's within image bounds. */
  if (find_struct(value, "x", &vX) >= 0) {
    if (V_TYPE(vX) != ID_VAL || V_FORMAT(vX) < FLOAT) {
      parse_error("VICAR overlay: x must be a double.");
      return;
    }
    else {
#if DEBUG
      fprintf(stderr, "DEBUG: new X\n");
#endif
      x = extract_double(vX, 0);
      move = True;
      delete = False;
    }
  }
  else {
    if (new) {
      parse_error("VICAR overlay: x (double) is required for new objects.");
      return;
    }
    else {
#if DEBUG
      fprintf(stderr, "DEBUG: reusing X\n");
#endif
      /* Re-use existing X. */
      x = graphics->x;
    }
  }

  /* Extract Y location. */
  /* FIX: make sure it's within image bounds. */
  if (find_struct(value, "y", &vY) >= 0) {
    if (V_TYPE(vY) != ID_VAL || V_FORMAT(vY) < FLOAT) {
      parse_error("VICAR overlay: y must be a double.");
      return;
    }
    else {
#if DEBUG
      fprintf(stderr, "DEBUG: new Y\n");
#endif
      y = extract_double(vY, 0);
      move = True;
      delete = False;
    }
  }
  else {
    if (new) {
      parse_error("VICAR overlay: y (double) is required for new objects.");
      return;
    }
    else {
#if DEBUG
      fprintf(stderr, "DEBUG: reusing Y\n");
#endif
      /* Re-use existing Y. */
      y = graphics->y;
    }
  }

  /* Extract color. */
  if (find_struct(value, "color", &vColor) >= 0) {
    if (V_TYPE(vColor) != ID_STRING) {
      parse_error("VICAR overlay: color must be a string.");
      return;
    }
    else {
#if DEBUG
      fprintf(stderr, "DEBUG: new color\n");
#endif
      colorString = V_STRING(vColor);
      /* Color changes require redrawing objects. */
      redraw = True;
      delete = False;
      /* Parse the color. */
      colorStatus = XParseColor(XtDisplay(widget),
				DefaultColormapOfScreen(XtScreen(widget)),
				colorString, &color);
      if (colorStatus == BadColor || colorStatus == BadValue) {
	parse_error("VICAR overlay: invalid color specified.");
      }
      vicarColor = XvicImageGetGrColor(widget, &color);
    }
  }
  else {
    if (new) {
#if DEBUG
      fprintf(stderr, "DEBUG: using default color\n");
#endif
      vicarColor = vicarData->defaultOverlayColor;
    }
    else {
#if DEBUG
      fprintf(stderr, "DEBUG: reusing old color\n");
#endif
      vicarColor = graphics->color;
    }
  }

  /* See if it's a shape object or a bitmap object or a change. */
  if (find_struct(value, "bitmap", &vBitmap) >= 0 &&
      find_struct(value, "shape", &vBitmap) >= 0) {
    parse_error("VICAR overlay: cannot set bitmap and shape simultaneously.");
    return;
  }

  if (find_struct(value, "bitmap", &vBitmap) >= 0) {
    /* Parse bitmap. */
    if (V_TYPE(vBitmap) != ID_VAL || V_FORMAT(vBitmap) != BYTE ||
	GetZ(vBitmap) != 1) {
      parse_error("VICAR overlay: bitmap must be 2D byte.");
      return;
    }
    else {

#if DEBUG
      fprintf(stderr, "DEBUG: creating new pixmap\n");
#endif
      redraw = True;
      delete = False;
      dvData = V_DATA(vBitmap);

      width = GetX(vBitmap);
      height = GetY(vBitmap);

      /* Calculate the bytes per line, rounding up. */
      if ((width % 8) == 0) {
	bytesPerLine = width / 8;
      }
      else {
	bytesPerLine = (width / 8) + 1;
      }

      /* Allocate the bitmap data and clear the memory. */
      bitmapData = (unsigned char *) calloc((size_t) (bytesPerLine * height),
					    sizeof(char));
      if (bitmapData == NULL) {
	parse_error("VICAR overlay: unable to allocate memory for overlay.");
	return;
      }

      /* Build the bitmap. */
      for (row = 0; row < height; row++) {
	for (col = 0; col < width; col++) {
	  if (((unsigned char) *(dvData + (row * height) + col))) {
	    *(bitmapData + (bytesPerLine * row) + (col / 8)) |=
	      (1 << (col % 8));
	  }
	}
      }

      /* Create a pixmap from the bitmap. */
      pixmap = XCreatePixmapFromBitmapData(XtDisplay(widget),
					   RootWindowOfScreen(XtScreen(widget)),
					   (char *) bitmapData,
					   width,
					   height,
					   1, /* Foreground pixel value. */
					   0, /* Background pixel value. */
					   1  /* Bit deptch. */
					   );

      /* Free the bitmap data. */
      free(bitmapData);

      if (pixmap == BadAlloc || pixmap == BadDrawable || pixmap == BadMatch) {
	parse_error("VICAR unable to create pixmap.");
	/* FIX: return something indicating error */
	return;
      }

    }
  }
#if 0
  else if (find_struct(value, "shape", &vShape) >= 0) {
    if (V_TYPE(vBitmap) != ID_STRING) {
      parse_error("VICAR overlay: shape must be a string.");
      return;
    }
    else {
    }
  }
#endif
  else {
    if (new) {
      parse_error("VICAR overlay: bitmap (2D byte) is required for new objects.");
      return;
    }
    else {
      /* FIX: handle reuse of shape */
      /* Use the existing pixmap. */
#if DEBUG
      fprintf(stderr, "DEBUG: reusing existing pixmap\n");
#endif
      pixmap = graphics->pixmap;
    }
  }

  /* Take the appropriate action. */

  if (!new && (delete || redraw)) {
#if DEBUG
    fprintf(stderr, "DEBUG: not-new object\n");
#endif
    if (redraw) {
#if DEBUG
      fprintf(stderr, "DEBUG: redraw object; deleting but saving pixmap\n");
#endif
      deleteVicarGraphicsObject(vicarData, graphics, False);
    }
    else {
#if DEBUG
      fprintf(stderr, "DEBUG: delete object; deleting incl pixmap\n");
#endif
      deleteVicarGraphicsObject(vicarData, graphics, True);
      return;
    }
  }

  if (new || redraw) {
    /* Creating a new object or redrawing an existing one. */
#if DEBUG
    fprintf(stderr, "DEBUG: making new object\n");
#endif
    /* Create VICAR overlay object. */
    vicarOverlayId = XvicImageDrawBitmap(widget, 0, vicarData->overlayGC,
					 vicarColor, x, y, pixmap,
					 width, height, 0, 0);
    if (vicarOverlayId == 0) {
      /* This should never happen. */
      parse_error("VICAR overlay: internal error; unable to create overlay.");
      return;
    }

    /* Create a new object node. */
    addVicarGraphics(vicarData, vicarOverlayId, davinciOverlayId,
		     vicarColor, x, y, pixmap);

  }
  else if (move) {
    /* Move an existing object. */
#if DEBUG
    fprintf(stderr, "DEBUG: moving object\n");
#endif
    XvicImageMoveObject(widget, graphics->vicarOverlayId,
			x - graphics->x, y - graphics->y);
    graphics->x = x;
    graphics->y = y;
  }

#if DEBUG
  fprintf(stderr, "DEBUG: done\n");
#endif

  return;

}

/* void
 * addVicarGraphics()
 *
 * Allocates a new overlay graphics object node and adds it to the linked
 * list with the specified details.
 *
 */

static void
addVicarGraphics(VicarData *vicarData, XvicID vicarOverlayId,
		 int davinciOverlayId, XvicColor color,
		 double x, double y, Pixmap pixmap)
{

  VicarGraphics	*graphics;

#if DEBUG
  fprintf(stderr, "DEBUG: addVicarGraphics(vicarData = %ld, XvicID = %d, "
	  "davinciOverlayId = %d, XvicColor = %d, x = %lf, y = %lf, "
	  "pixmap = %d)\n", vicarData, vicarOverlayId, davinciOverlayId,
	  color, x, y, pixmap);
#endif

  /* Build the new node. */

  graphics = (VicarGraphics *) malloc(sizeof(VicarGraphics));
  if (graphics == NULL) {
    parse_error("VICAR overlay: unable to allocate ram for new overlay.");
    return;
  }
#if DEBUG
  fprintf(stderr, "DEBUG: new node = %ld\n", graphics);
#endif

  graphics->vicarOverlayId = vicarOverlayId;
  graphics->davinciOverlayId = davinciOverlayId;
  graphics->color = color;
  graphics->x = x;
  graphics->y = y;
  graphics->pixmap = pixmap;

  /* Put it in the list. */

  if (vicarData->overlaysHead == NULL) {
#if DEBUG
    fprintf(stderr, "DEBUG: first node\n");
#endif
    /* First node. */
    graphics->next = graphics->prev = NULL;
    vicarData->overlaysHead = vicarData->overlaysTail = graphics;
  }
  else {
    graphics->next = NULL;
    graphics->prev = vicarData->overlaysTail;
    graphics->prev->next = graphics;
    vicarData->overlaysTail = graphics;
  }

#if DEBUG
    fprintf(stderr, "DEBUG: vicarData->overlaysHead = %ld\n", vicarData->overlaysHead);
    fprintf(stderr, "DEBUG: vicarData->overlaysTail = %ld\n", vicarData->overlaysTail);
#endif

  return;

}

static void
deleteVicarGraphicsObject(VicarData *vicarData, VicarGraphics *graphics,
			  Boolean deletePixmap)
{

#if DEBUG
  fprintf(stderr, "DEBUG: deleteVicarGraphicsObject(vicarData = %ld, "
	  "graphics = %ld)\n", vicarData, graphics);
#endif

  if (vicarData->overlaysHead == graphics) {
#if DEBUG
    fprintf(stderr, "DEBUG: deleting head\n");
#endif
    /* Deleting first node. */
    vicarData->overlaysHead = graphics->next;
  }
  else {
    graphics->prev->next = graphics->next;
  }

  if (vicarData->overlaysTail == graphics) {
#if DEBUG
    fprintf(stderr, "DEBUG: deleting tail\n");
#endif
    /* Deleting last node. */
    vicarData->overlaysTail = graphics->prev;
  }
  else {
    graphics->next->prev = graphics->prev;
  }

  /* Delete VICAR overlay object. */
#if DEBUG
  fprintf(stderr, "DEBUG: deleting vicar object\n");
#endif
  XvicImageEraseObject(vicarData->imageWidget, graphics->vicarOverlayId);

  /* Free pixmap data (if any). */
  if (deletePixmap == True && graphics->pixmap != 0) {
#if DEBUG
    fprintf(stderr, "DEBUG: freeing pixmap\n");
#endif
    XFreePixmap(XtDisplay(vicarData->imageWidget), graphics->pixmap);
  }

  /* Delete the node. */
#if DEBUG
  fprintf(stderr, "DEBUG: freeing node\n");
#endif
  free(graphics);

#if DEBUG
    fprintf(stderr, "DEBUG: vicarData->overlaysHead = %ld\n", vicarData->overlaysHead);
    fprintf(stderr, "DEBUG: vicarData->overlaysTail = %ld\n", vicarData->overlaysTail);
#endif

  return;

}

/* VicarGraphics *
 * getVicarGraphicsByDavinciId()
 *
 * Returns the VicarGraphics node corresponding to the supplied Davinci ID,
 * or NULL if not found.
 *
 */

static VicarGraphics *
getVicarGraphicsByDavinciId(VicarData *vicarData, int id)
{

  VicarGraphics *cur;

  cur = vicarData->overlaysHead;
  while (cur != NULL) {
    if (cur->davinciOverlayId == id) {
      return cur;
    }
    cur = cur->next;
  }

  return NULL;

}

/* Expose event handler. */

/* FIX: there are probably a bunch of options that still need to be handled here */
/*      make sure all the datatypes work */

/* handleExpose()
 *
 * Processes exposeCallback events.  When an expose event occurs, the widget
 * needs to be given the data to display.  Data should be supplied to the
 * widget via a call to XvicImageWrite() [see Vicar documentation].  It
 * expects "tiles" of data.  Whole tiles should be returned, and the new_data
 * flag should be set to indicate whether or not any of the data in the tile
 * has been modified since the last time it was displayed.  In the current
 * implementation we are not modifying any data, so this flag should always be
 * False.
 *
 */

static void
handleExpose(Widget widget, XtPointer clientData, XtPointer callData)
{

  MyWidgetList			widgetListEntry;
  VicarData			*vicarData;
  Var				*imageData;
  unsigned int			imageWidth, imageHeight;
  XvicImageData			vicImageData;
  XvicImageCallbackStruct	*cb;
  unsigned char			imageMode;

  cb = (XvicImageCallbackStruct *) callData;

#if DEBUG
  fprintf(stderr, "DEBUG: handleExpose(%ld, %ld, %ld)\n",
	  widget, clientData, callData);
#endif

  /* Get the WidgetListEntry for access to the image data. */
  widgetListEntry = gui_getWidgetListEntryFromWidget(widget);

  /* Get the instance data associated with this widget. */
  vicarData = (VicarData *) widgetListEntry->data;

  /* Get the Davinci Var containing the image data in BSQ format. */
  imageData = vicarData->imageData;
  imageWidth = GetX(imageData);
  imageHeight = GetY(imageData);

#if 0
  fprintf(stderr, "DEBUG: imageWidth = %d\n", imageWidth);
  fprintf(stderr, "DEBUG: imageHeight = %d\n", imageHeight);
  fprintf(stderr, "DEBUG: cb->x = %d\n", cb->x);
  fprintf(stderr, "DEBUG: cb->y = %d\n", cb->y);
  fprintf(stderr, "DEBUG: cb->width = %d\n", cb->width);
  fprintf(stderr, "DEBUG: cb->height = %d\n", cb->height);
  fprintf(stderr, "DEBUG: cb->prezoom_width = %d\n", cb->prezoom_width);
#endif

  /* FIX: this doesn't take different data sizes into account. */

#if 1
  XtVaGetValues(widget, XvicNimageMode, &imageMode, NULL);

  if (imageMode == XvicBW) {
    vicImageData.bw_pixels = ((unsigned char *) V_DATA(imageData)) +
      (imageWidth * cb->y) + cb->x;
  }
  else {
    vicImageData.red_pixels = ((unsigned char *) V_DATA(imageData)) +
      (imageWidth * cb->y) + cb->x;
    vicImageData.grn_pixels = ((unsigned char *) V_DATA(imageData)) +
      (imageWidth * (imageHeight + cb->y)) + cb->x;
    vicImageData.blu_pixels = ((unsigned char *) V_DATA(imageData)) +
      (imageWidth * (imageHeight * 2 + cb->y)) + cb->x;
  }

  vicImageData.x = cb->x;
  vicImageData.width = cb->width;
  vicImageData.y = cb->y;
  vicImageData.height = cb->height;
  vicImageData.memory_control = XvicMEMORY_SHARED;
  vicImageData.line_width = imageWidth;
  vicImageData.start_offset = 0;

#else
  vicImageData.x = 0;
  vicImageData.width = imageWidth;
  vicImageData.y = 0;
  vicImageData.height = imageHeight;
  vicImageData.memory_control = XvicMEMORY_SHARED;
  vicImageData.line_width = imageWidth;
  vicImageData.start_offset = 0;
  vicImageData.red_pixels = ((unsigned char *) V_DATA(imageData));
  vicImageData.grn_pixels = ((unsigned char *) V_DATA(imageData)) + (imageWidth * imageHeight);
  vicImageData.blu_pixels = ((unsigned char *) V_DATA(imageData)) + (imageWidth * imageHeight * 2);
#endif

#if DEBUG
  fprintf(stderr, "DEBUG: V_DATA(imageData) = %ld\n", V_DATA(imageData));
  fprintf(stderr, "DEBUG: red_pixels = %ld\n", vicImageData.red_pixels);
  fprintf(stderr, "DEBUG: grn_pixels = %ld\n", vicImageData.grn_pixels);
  fprintf(stderr, "DEBUG: blu_pixels = %ld\n", vicImageData.blu_pixels);
  fprintf(stderr, "DEBUG: writing image data\n");
#endif

  XvicImageWrite(widget, &vicImageData, False);

#if DEBUG
  fprintf(stderr, "DEBUG: written\n");
#endif

  return;

#if 0

  printf("data = %ld\n", data); 
  XtVaGetValues(iw, XvicNimageMode, &imageMode, NULL);
  XtVaGetValues(iw, XvicNdataType, &dataType, NULL);
  
  switch (dataType) {
  case XvicBYTE: pixelSize = sizeof(XvicByte); break;
  case XvicHALF: pixelSize = sizeof(XvicHalf); break;
  case XvicUHALF: pixelSize = sizeof(XvicUHalf); break;
  case XvicFULL: pixelSize = sizeof(XvicFull); break;
  case XvicUFULL: pixelSize = sizeof(XvicUFull); break;
  case XvicREAL: pixelSize = sizeof(XvicReal); break;
  case XvicDOUBLE: pixelSize = sizeof(XvicDouble); break;
  }

  img.line_width = x * pixelSize;

  if (imageMode == XvicCOLOR) {
    printf("color\n");
    printf("line width = %d\n", img.line_width);

    img.red_pixels = data;
    img.grn_pixels = data + x * y;
    img.blu_pixels = data + x * y * 2;

  }
  else {
    printf("BW\n");
    printf("line width = %d\n", img.line_width);
    img.bw_pixels = data;
  }

  /* Populate the select box coordinates, default selection is entire image. */
  /* FIX: would rather have undefined default */
  /* FIX: yeah yeah */
  /* FIX: non-global */

  iwPtr = gui_getWidgetListEntryFromWidget(iw)->data; /* FIX: this was gbl_iw; not sure what's going on.. */
  iwPtr->x = 0;
  iwPtr->y = 0;
  iwPtr->width = x;
  iwPtr->height = y;

  XvicImageWrite(iw, &img, TRUE);

#endif

}

/* handleInput()
 *
 *
 */

static void
handleInput(Widget widget, XtPointer clientData, XtPointer callData)
{

  XvicImageCallbackStruct *cb = (XvicImageCallbackStruct *)callData;
  static TimerData timerData;

  MyWidgetList		widgetListEntry;
  VicarData		*vicarData;
  VicarInputMode	inputMode;

#if DEBUG
  fprintf(stderr, "DEBUG: handleInput(widget = %ld, clientData = %ld,"
	  "callData = %ld)\n",
	  widget, clientData, callData);
#endif

  /* Get the WidgetListEntry for access to the image data. */
  widgetListEntry = gui_getWidgetListEntryFromWidget(widget);

  /* Get the instance data associated with this widget. */
  vicarData = (VicarData *) widgetListEntry->data;

  inputMode = vicarData->inputMode;

#if 0
  fprintf(stderr, "DEBUG: VicarData.x = %lf\n", vicarData->x);
  fprintf(stderr, "DEBUG: VicarData.y = %lf\n", vicarData->y);
  fprintf(stderr, "DEBUG: VicarData.width = %lf\n", vicarData->width);
  fprintf(stderr, "DEBUG: VicarData.height = %lf\n", vicarData->height);
  fprintf(stderr, "DEBUG: VicarData.imageData = %ld\n", vicarData->imageData);
  fprintf(stderr, "DEBUG: VicarData.pixmap = %ld\n", vicarData->pixmap);
  fprintf(stderr, "DEBUG: VicarData.graphics = %ld\n", vicarData->graphics);
  fprintf(stderr, "DEBUG: VicarData.graphics.color = %ld\n", vicarData->graphics.color);
  fprintf(stderr, "DEBUG: VicarData.graphics.gc = %ld\n", vicarData->graphics.gc);
  fprintf(stderr, "DEBUG: VicarData.graphics.rubberBandGC = %ld\n", vicarData->graphics.rubberBandGC);
  fprintf(stderr, "DEBUG: VicarData.graphics.id = %ld\n", vicarData->graphics.id);
  fprintf(stderr, "DEBUG: VicarData.graphics.rubberBandId = %ld\n", vicarData->graphics.rubberBandId);
  fprintf(stderr, "DEBUG: VicarData.graphics.timeoutId = %d\n", vicarData->graphics.timeoutId);
  fprintf(stderr, "DEBUG: VicarData.graphics.x = %lf\n", vicarData->graphics.x);
  fprintf(stderr, "DEBUG: VicarData.graphics.y = %lf\n", vicarData->graphics.y);
  fprintf(stderr, "DEBUG: VicarData.imageWidget = %ld\n", vicarData->imageWidget);
#endif

  if (cb->input_num_params > 0) {

    switch (inputMode) {

    case DV_VICAR_INPUT_POINT:

#if DEBUG
      if (vicarData->timeoutId) {
	fprintf(stderr, "DEBUG: timeout set in point-input mode!\n");
      }
#endif

      if (strcmp(cb->input_params[0], "Draw") == 0) {
	if (strcmp(cb->input_params[1], "end") == 0) {
	  vicarData->pointX = cb->x_fp;
	  vicarData->pointY = cb->y_fp;
#if 0
	  /* Add an event to the stack. */
	  gui_addEvent(widget, "point");
#else
	  /* Trigger a pseudo-callback. */
	  gui_pseudoCallback(widget, "point");
#endif
	}
      }
      /* Ignoring others for now. */
      break;

    case DV_VICAR_INPUT_SELECT:

      if (strcmp(cb->input_params[0], "Draw") == 0) {
	if (vicarData->timeoutId) {
	  /* Clear any existing auto-pan task. */
	  XtRemoveTimeOut(vicarData->timeoutId);
	  vicarData->timeoutId = 0;
	}
	if (cb->input_num_params > 1) {
	  if (strcmp(cb->input_params[1], "start") == 0) {
	    /* Only supporting a single selection object. */
	    if (vicarData->selectionOverlay->vicarOverlayId) {
	      /* Erase any existing selection and clear the selection data. */
	      XvicImageEraseObject(widget, vicarData->selectionOverlay->vicarOverlayId);
	      vicarData->selectionOverlay->vicarOverlayId = 0;
	      vicarData->selectionX = 0;
	      vicarData->selectionY = 0;
	      vicarData->selectionWidth = 0;
	      vicarData->selectionHeight = 0;
	    }
	    /* Save settings for a new selection object. */
	    vicarData->selectionOverlay->x = cb->x_fp;
	    vicarData->selectionOverlay->y = cb->y_fp;
	    vicarData->selectionOverlayRubberBand->x = cb->x_fp;
	    vicarData->selectionOverlayRubberBand->y = cb->y_fp;
	  }
	  else if (strcmp(cb->input_params[1], "drag") == 0) {
	    /* Should always be an existing object here, I think.. */
	    if (vicarData->selectionOverlayRubberBand->vicarOverlayId) {
	      /* Erase existing object if currently drawing. */
	      XvicImageEraseObject(widget,
				   vicarData->selectionOverlayRubberBand->vicarOverlayId);
	      vicarData->selectionOverlayRubberBand->vicarOverlayId = 0;
	    }
	    /* Draw new rubberband object. */
	    drawObject(widget, vicarData, vicarData->rubberBandGC,
		       cb->x_fp, cb->y_fp, True);
	    if (!cb->on_screen) {
	      /* Setup auto-pan. */
	      /* FIX: I'm not sure what this is for. */
	      memcpy((void *)&timerData.event, (void *)cb->event, sizeof(XEvent));
	      timerData.widget = widget;
	      timerData.x = cb->x;
	      timerData.y = cb->y;
	      /* FIX: #define for 500 */
	      vicarData->timeoutId =
		XtAppAddTimeOut(XtWidgetToApplicationContext(widget),
				500, autoPan, &timerData);
	    }
	  }
	  else if (strcmp(cb->input_params[1], "end") == 0) {
	    /* Erase the rubberband object. */
	    if (vicarData->selectionOverlayRubberBand->vicarOverlayId) {
	      XvicImageEraseObject(widget, vicarData->selectionOverlayRubberBand->vicarOverlayId);
	      vicarData->selectionOverlayRubberBand->vicarOverlayId = 0;
	    }
	    /* Draw final (non-rubberband) object. */
	    /* FIX: this is hacky. */
	    drawObject(widget, vicarData, vicarData->overlayGC,
		       cb->x_fp, cb->y_fp, False);
#if 0
	    /* Add an event to the stack. */
	    gui_addEvent(widget, "select");
#else
	    /* Trigger a pseudo-callback. */
#if DEBUG
	    fprintf(stderr, "DEBUG: calling gui_pseudoCallback()\n");
#endif
	    gui_pseudoCallback(widget, "select");
#endif
	  }
	}
      }
      break;

    default:
      parse_error("Internal error: invalid VICAR inputMode.");
      break;

    }
  }

  return;

}

static XvicGC
createNewGC(Widget widget, int rubber)
{

  /* FIX: I barely understand this stuff so it's probably wrong.. */

  XvicGC gc;
  XGCValues values;

  values.line_width = 1;
  values.line_style = LineSolid;

  if (rubber) {
    gc = XvicImageCreateRubberGC(widget, 0, &values);
  }
  else {
    gc = XvicImageCreateGC(widget, 0, &values);
  }

  return gc;

}

#if 0

static XvicID
addOverlayObject(Widget widget, VicarGraphics *graphics, double x, double y)
{

  XvicID	newId;

  return newId;

}

#endif

#if 0

static void
drawObjectStart(VicarGraphics *graphics, double x, double y) {

  /* Only handles rubberband rectangles at the moment. */

#if DEBUG
  fprintf(stderr, "DEBUG: drawObjectStart(graphics = %ld, x = %lf, y = %lf\n",
	  graphics, x, y);
#endif

  graphics->x = x;
  graphics->y = y;

}

#endif

static void
drawObject(Widget widget, VicarData *vicarData, XvicGC gc,
	   double x, double y, int rubberBand)
{

  XvicID	objectId;
  double	height, width;
  double	xx, yy;
  VicarGraphics	*graphics;

#if DEBUG
  fprintf(stderr, "DEBUG: drawObject(graphics = %ld, vicarData = %ld, "
	  "x = %lf, y = %lf, rubberBand = %d\n",
	  graphics, vicarData, x, y, rubberBand);
#endif

  if (rubberBand) {
    graphics = vicarData->selectionOverlayRubberBand;
  }
  else {
    graphics = vicarData->selectionOverlay;
  }

#if DEBUG
  fprintf(stderr, "DEBUG: graphics->x = %lf\n", graphics->x);
  fprintf(stderr, "DEBUG: graphics->y = %lf\n", graphics->y);
#endif

  xx = MIN(graphics->x, x);
  yy = MIN(graphics->y, y);
  height = ABS(y - graphics->y) + 1.0;
  width = ABS(x - graphics->x) + 1.0;

#if DEBUG
  fprintf(stderr, "DEBUG: xx = %lf\n", xx);
  fprintf(stderr, "DEBUG: yy = %lf\n", yy);
  fprintf(stderr, "DEBUG: width = %lf\n", width);
  fprintf(stderr, "DEBUG: height = %lf\n", height);
#endif

  objectId = XvicImageDrawRectangle(widget, 0, gc, graphics->color, xx, yy, width, height);

  graphics->vicarOverlayId = objectId;

  if (!rubberBand) {
    /* FIX: cheap hack to store rectangle coordinates.  Stop doing this. */
    vicarData->selectionX = xx;
    vicarData->selectionY = yy;
    vicarData->selectionWidth = width;
    vicarData->selectionHeight = height;
  }

}

static void
autoPan(XtPointer clientData, XtIntervalId *id) {

  TimerData	*td = (TimerData *)clientData;
  String	params[2] = {"Draw", "drag"};
  int		x_pan, y_pan;
  int		x1, y1, x2, y2;

  XvicImageDisplayBounds(td->widget, &x1, &y1, &x2, &y2);
  XtVaGetValues(td->widget, XvicNxPan, &x_pan, XvicNyPan, &y_pan, NULL);

  if (td->x < x1)
    x_pan -= (x1 - td->x);
  else if (td->x > x2)
    x_pan += (td->x - x2);
  if (td->y < y1)
    y_pan -= (y1 - td->y);
  else if (td->y > y2)
    y_pan += (td->y - y2);

  XtVaSetValues(td->widget, XvicNxPan, x_pan, XvicNyPan, y_pan, NULL);
  XtCallActionProc(td->widget, "Input", (XEvent *)clientData, params, 2);

}

void
gui_getVicarPseudoResources(Widget widget, Var *dvStruct)
{

 #if DEBUG
  fprintf(stderr, "DEBUG: gui_getVicarPseudoResources(%ld, %ld)\n",
	  widget, dvStruct);
#endif

  add_struct(dvStruct, "inputMode", getInputMode(widget));
  add_struct(dvStruct, "selection", getSelection(widget));
  add_struct(dvStruct, "point", getPoint(widget));
  add_struct(dvStruct, "zoom", getZoom(widget));
  /* FIX: not returning image for now, since it can be fairly enormous
   * and a copy is necessary.
   */
  add_struct(dvStruct, "image", getImage(widget));

  return;

}

void
gui_setVicarPseudoResources(Widget widget, Var *dvStruct,
			    Narray *publicResources)
{

  /* FIX: will need to add anything set to the gettable-resources
   * list.
   */

  /* NOTE: this deletes anything that it sets from the struct.
   * This doesn't match the current gui.set() model, which returns
   * a list of values-as-they-were-really-set.
   * FIX: do this differently?  Maybe keep a list of valus set?
   */

  int		i, cont;
  char		*name;
  Var		*value, *tmpImage;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_setVicarPseudoResources(widget = %ld, "
	  "dvStruct = %ld, publicResources = %ld)\n",
	  widget, dvStruct, publicResources);
#endif

  /* Iterate over the struct, extracting any pseudo-resource items that
   * we set.  Delete the items from the struct, and start iterating at the
   * beginning again.  This is not really efficient, but I'm not sure how
   * else to do it without mucking about with Davinci's structures more than
   * I'd like to, or keeping track of what has/hasn't been set.
   */

  cont = 1;
  while (cont && get_struct_count(dvStruct)) {
    cont = 0;
    for (i = 0; i < get_struct_count(dvStruct); i++) {
      get_struct_element(dvStruct, i, &name, &value);
      if (!strcmp(name, "inputMode")) {
	setInputMode(widget, value);
	Narray_add(publicResources, name, NULL);
	free_var(Narray_delete(V_STRUCT(dvStruct), "inputMode"));
	cont = 1;
	break;
      }
      if (!strcmp(name, "selection")) {
	setSelection(widget, value);
	Narray_add(publicResources, name, NULL);
	free_var(Narray_delete(V_STRUCT(dvStruct), "selection"));
	cont = 1;
	break;
      }
      if (!strcmp(name, "zoom")) {
	setZoom(widget, value);
	Narray_add(publicResources, name, NULL);
	free_var(Narray_delete(V_STRUCT(dvStruct), "zoom"));
	cont = 1;
	break;
      }
      if (!strcmp(name, "image")) {
	Narray_add(publicResources, name, NULL);
	tmpImage = (Var *) Narray_delete(V_STRUCT(dvStruct), "image");
	/* If setImage() succeeds, the widget owns tmpImage, otherwise
	 * free it.
	 */
	if(setImage(widget, tmpImage) != 1) {
	  free_var(tmpImage);
	}
	cont = 1;
	break;
      }
#if 1
      /* FIX: remove if */
      if (!strcmp(name, "overlay")) {
	(void) setOverlay(widget, value);
	Narray_add(publicResources, name, NULL);
	free_var(Narray_delete(V_STRUCT(dvStruct), "overlay"));
	cont = 1;
	break;
      }
#endif
     /* ...new comparisons go here. */
    }
  }

}

/* gui_getVicarPublicResources()
 *
 * FIX: this could be turned into a generic function in gui.c, with a
 *      helper function in widget_*.c that just returns the static
 *      string list.
 *
 * Builds a Narray based from the public resources list.
 *
 * Returns pointer to Narray.
 *
 */

Narray *
gui_getVicarPublicResources()
{

  Narray	*resList;
  int		i, num;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_getVicarPublicResources()\n");
#endif

  num = sizeof(vicarPublicResources) / sizeof(vicarPublicResources[0]);
  resList = Narray_create(num);
  for (i = 0; i < num; i++) {
    Narray_add(resList, (char *) vicarPublicResources[i], NULL);
  }

  return resList;

}

/* convertToBSQ(Var *from)
 *
 * Returns a Var * of 'from' converted to BSQ format (may just return
 * 'from' if it's already BSQ).  This function shouldn't be called if the
 * data is already in BSQ, since it will just dup the source.
 *
 * This is essentially copied from ff_org() in ff.c.
 *
 */

static Var *
convertToBSQ(Var *from)
{

  Var	*new;
  void	*fromData, *toData;
  int	i, j, dsize, format, nbytes;

#if DEBUG
  fprintf(stderr, "DEBUG: convertToBSQ(from = %ld)\n", from);
#endif

  if (V_ORG(from) == BSQ) {
    /* Just dup it.  This function shouldn't be called in this case anyway, as
     * it may use tons of memory.
     */
    return V_DUP(from);
  }

  dsize = V_DSIZE(from);
  format = V_FORMAT(from);

  new = newVar();
  V_TYPE(new) = ID_VAL;
  memcpy(V_SYM(new), V_SYM(from), sizeof(Sym));
  V_DATA(new) = calloc(dsize, NBYTES(format));
  V_ORG(new) = BSQ;

  for (i = 0; i < 3; i++) {
    V_SIZE(new)[orders[BSQ][i]] = V_SIZE(from)[orders[V_ORG(from)][i]];
  }

  nbytes = NBYTES(format);
  fromData = V_DATA(from);
  toData = V_DATA(new);
  for (i = 0; i < V_DSIZE(new); i++) {
    j = rpos(i, from, new);
    memcpy(((char *) toData) + (j * nbytes), ((char *) fromData) + (i * nbytes), nbytes);
  }

  return new;

}

#if 0

/* Pixmap
 * convertToPixmap(Var *from)
 *
 * Returns
 *
 */

static Pixmap
convertToPixmap(Widget widget, Var *from)
{

  Pixmap	pixmap;
  unsigned int	width, height;
  const short 	depth = 1;

  if (V_TYPE(value) != ID_VAL || GetZ(value) != 1 || V_FORMAT(value) != BYTE) {
    parse_error("VICAR overlay must be 1-band BYTE data.");
    return 0;
  }

  pixmap = XCreatePixmap(XtDisplay(widget), RootWindowOfScreen(XtScreen(widget)),
			 width, height, depth);

}

#endif

void
gui_destroyVicar(Widget widget, void *instanceData)
{

  VicarData	*vicarData;
  VicarGraphics	*graphics;

  vicarData = (VicarData *) instanceData;

#if DEBUG
  fprintf(stderr, "DEBUG: gui_vicarDestroyWidget(widget = %ld, "
	  "instanceData = %ld\n", widget, instanceData);
#endif

  /* Unrealize widget so the following actions don't cause problems. */
#if DEBUG
  fprintf(stderr, "DEBUG: unrealizing widget\n");
#endif
  XtUnrealizeWidget(widget);

  /* Erase any overlays. */
#if DEBUG
  fprintf(stderr, "DEBUG: erasing overlays\n");
#endif
  XvicImageEraseOverlay(widget);

#if 0
  /* Erase any pixmaps that were used in overlays. */
  /* FIX: implement multiple pixmap erasing once setOverlay() actually works. */
  /* FIX: free all overlays.  Free GCs + pixmaps + linked list nodes. */
  if (vicarData->pixmap) {
#if DEBUG
    fprintf(stderr, "DEBUG: freeing pixmap (%d)\n", vicarData->pixmap);
#endif
    XFreePixmap(XtDisplay(widget), vicarData->pixmap);
  }
#endif

  /* Free selection overlays. */

  graphics = vicarData->selectionOverlay;
  free(graphics);
  vicarData->selectionOverlay = NULL; /* FIX: remove; freeing it all anyway. */
  graphics = vicarData->selectionOverlayRubberBand;
  free(graphics);
  vicarData->selectionOverlayRubberBand = NULL; /* FIX: remove; freeing it all anyway. */

  /* Free GCs. */
  XvicImageFreeGC(widget, vicarData->overlayGC);
  XvicImageFreeGC(widget, vicarData->rubberBandGC);

  /* Free the image. */
  if (vicarData->imageData != NULL) {
#if DEBUG
    fprintf(stderr, "DEBUG: freeing imageData (%ld)\n", vicarData->imageData);
#endif
    free_var(vicarData->imageData);
  }

  /* Destroy the widget. */
  XtDestroyWidget(widget);

  /* FIX: missing anything? */

  return;

}

#endif /* HAVE_LIBVICAR */
