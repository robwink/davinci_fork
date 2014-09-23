/*
** Xf* - Resource caching routines
**
** XfStipple - Create (or retrieve) a stippling pixmap 
** XfColor   - Allocate (or retrieve) a pixel value
** XfFont    - Allocate (or retrieve) a font
**
** All of these routines cache their return values, so subsequent calls
** with the same parameters do not reallocate the same resouce.
** 
** Resouces are not currently freed through any means other than
** program termination.
**
*/
#include "Xfred.h"
#include "xf.h"

Pixmap *_XFStip = NULL;
char **_XFStipnames = NULL;
int nXFStip = 0;

int _xf3Dheight=3;


/**
*** This routine tries to allocate a bitmap from the X11_INCLUDE bitmaps 
*** directory.  If the user specifies the full path to a stipple, the
*** path is used instead.
**/

Pixmap
XfStipple(display, window, name)
Display *display;
Drawable window;
char *name;
{
	Pixmap p=0;
	int i;
	char buf[256];
	int x_hot, y_hot;
	int width, height;
	int err;

	for (i = 0 ; i < nXFStip ; i++) {
		if (!strcmp(_XFStipnames[i], name)) {
			return(_XFStip[i]);
		}
	}

	if (display == NULL) display = _xfDisplay;
	if (name == NULL) return((Pixmap)NULL);

	/**
	*** Find and load the bitmap file
	**/
	if (strchr(name, '/')) {
		strcpy(buf, name);
	} else {
		sprintf(buf, "%s/bitmaps/%s", X11_INCLUDE, name);
	}
	err = XReadBitmapFile(display, window, buf, 
						  &width, &height, &p, &x_hot, &y_hot);

	if (err != BitmapSuccess) {
		/* guess at a directory */
		sprintf(buf, "/usr/include/X11/bitmaps/%s", name);
		err = XReadBitmapFile(display, window, buf, 
							  &width, &height, &p, &x_hot, &y_hot);
		if (err != BitmapSuccess) {
			sprintf(buf, "/usr/openwin/include/X11/bitmaps/%s", name);
			err = XReadBitmapFile(display, window, buf, 
								  &width, &height, &p, &x_hot, &y_hot);
		}
	}

	if (err == BitmapSuccess) {
		if (nXFStip == 0) {
			_XFStip = (Pixmap *)calloc(1,sizeof(Pixmap));
			_XFStipnames = (char **)calloc(1,sizeof(char *));
		} else {
			_XFStip = (Pixmap *)realloc(_XFStip, sizeof(Pixmap)*(nXFStip+1));
			_XFStipnames = (char **)realloc(_XFStipnames, sizeof(char *)*(nXFStip+1));
		}

		_XFStip[nXFStip] = p;
		_XFStipnames[nXFStip] = strdup(name);

		nXFStip++;
		return(p);
	}
	return((Pixmap)NULL);
}

char **_XFCNames = NULL;
XColor *_XFColors = NULL;
int nXFColors = 0;

short
XfColor(display, name)
Display *display;
char *name;
{
	int i;
	XColor xc;
    Colormap cmap;

	if (display == NULL) display = _xfDisplay;
	cmap = DefaultColormap(display, DefaultScreen(display));

	if (name == (char *)0) return(BlackPixel(display, DefaultScreen(display)));
	if (name == (char *)1) return(WhitePixel(display, DefaultScreen(display)));

	for (i = 0 ; i < nXFColors ; i++) {
		if (!strcmp(name, _XFCNames[i])) {
			return(_XFColors[i].pixel);
		}
	}

	if (!XParseColor(display, cmap, name, &xc)) return(0);
	if (!XAllocColor(display, cmap, &xc)) return(0);

	if (nXFColors == 0) {
		_XFCNames = (char **) calloc(1,sizeof(char *));
		_XFColors = (XColor *)calloc(1,sizeof(XColor));
	} else {
		_XFCNames = (char **) realloc(_XFCNames, sizeof(char *)*(nXFColors+1));
		_XFColors = (XColor *)realloc(_XFColors, sizeof(XColor)*(nXFColors+1));
	}

	_XFCNames[nXFColors] = strdup(name);
	_XFColors[nXFColors] = xc;

	nXFColors++;

	return(xc.pixel);
}

/**
*** If passed NULL values, this routine uses the default Display and Font
*** values setup in init.
**/
char **_XFFNames = NULL;
XFontStruct **_XFFonts = NULL;
int nXFFonts = 0;

/**
*** This routine allocates the "fixed" font if the requested font is not
*** available.  It will allocate the "fixed" font more than once if more
*** than one requested font is not available.
**/
XFontStruct *
XfFont(display, name)
Display *display;
char *name;
{
	int i;
	XFontStruct *font;

	if (display == NULL) display = _xfDisplay;
	if (name == NULL) {
		if (_xfFontStruct == NULL) {
			XfSetDefaultFont(display, NULL);
		}
		return(_xfFontStruct);
	}

	for (i = 0 ; i < nXFFonts ; i++) {
		if (!strcmp(name, _XFFNames[i])) {
			return(_XFFonts[i]);
		}
	}

	/**
	*** Load requested font, otherwise, try "fixed"
	**/
	if ((font = XLoadQueryFont(display, name)) == NULL) {
		if ((font = XLoadQueryFont(display, "fixed")) == NULL) {
			return(NULL);
		}
	}

	if (nXFFonts == 0) {
		_XFFNames = (char **) calloc(1,sizeof(char *));
		_XFFonts = (XFontStruct **)calloc(1,sizeof(XFontStruct *));
	} else {
		_XFFNames = (char **) realloc(_XFFNames, sizeof(char *)*(nXFColors+1));
		_XFFonts = (XFontStruct **)realloc(_XFFonts,sizeof(XFontStruct)*(nXFFonts+1));
	}

	_XFFNames[nXFFonts] = strdup(name);
	_XFFonts[nXFFonts] = font;

	nXFFonts++;

	return(font);
}

/**
*** This sets up the font returned by XfFont when passed a NULL name.
**/
void
XfSetDefaultFont(display, fs)
Display *display;
XFontStruct *fs;
{
	char *name;

	if (display == NULL) display = _xfDisplay;
	if (fs == NULL) {
		if ((name = getenv("XF_DEFAULT_FONT")) == NULL) 
#ifdef XF_DEFAULT_FONT
		name = XF_DEFAULT_FONT;
#else
		name = "7x13";
#endif
		if ((_xfFontStruct = XLoadQueryFont(display, name)) == NULL) {
			_xfFontStruct = XLoadQueryFont(display, "fixed");
		}
	} else {
		_xfFontStruct = fs;
	}

	return;
}

void
XfSet3DHeight(height)
int height;
{
	if (height == 0) height = 1;
	_xf3Dheight = height;
}

int
Xf3DHeight()
{
	return(_xf3Dheight);
}
