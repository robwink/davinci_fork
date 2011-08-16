/*
 * Slider.h
 *
 * The data structures and prototypes for the Slider class of tools.
 *
 * A slider is an object consisting of the field and a thumb. When created,
 * it has values corresponding the minimum and maximum settings. The value
 * associeted with a slider internally is the range [0, 1]. Externally,
 * through XfGetSliderValue(), that value is mapped along the range of max
 * and min.
 *
 * $Header$
 *
 * $Log$
 * Revision 1.1  1999/06/16 03:24:23  gorelick
 * Initial revision
 *
 * Revision 1.1  1999/06/16 01:40:47  gorelick
 * Initial install
 *
 * Revision 0.2  91/09/23  17:55:07  17:55:07  ngorelic (Noel S. Gorelick)
 * *** empty log message ***
 * 
 * Revision 0.1  91/07/24  18:04:11  18:04:11  rray (Randy Ray)
 * *** empty log message ***
 * 
 *
 */

#ifndef _XF_SLIDER_H
#define _XF_SLIDER_H

/* Orientation of sliders */
#define XfSliderUpDown 0
#define XfSliderLeftRight 1

/* For ease in typing */
typedef struct _Slider* Slider;

/* The basis for a Slider. That is to say, no definition, no slider! */
struct _Slider
{
    Display* display;             /* Display the slider is on */
    Window window;                /* Window associated with slider */
    Window parent;                /* window's parent */
    int active;                   /* Slider active/not active */
    int width, height;            /* Sizing of slider */
    int x, y;                     /* X and Y position within parent window */
    int border_width;             /* width (in pixels) of border */
    unsigned long border_color;   /* Color of border */
    char name[256];               /* Name used to identify slider */
    char* ext;                    /* An externally-visible piece of data */
    int* member;                  /* Pseudo-clas identifier */
    int orientation;              /* Whether Left<->Right or Up<->Down */
    float min, max;               /* Minimum and maximum values */
    float delta;                  /* Spacing between min and max */
    float value;                  /* Slider value in range [0, 1] */
    int thumb_width;              /* Thumb size in X */
    int thumb_height;             /* Thumb size in Y */
    int thumb_min_pixel;          /* Minimum value along motion axis */
    int thumb_max_pixel;          /* Maximum along motion axis */
    int thumb_cur_pixel;          /* Current pixel value along motion axis */
    struct VisualInfo* bar;       /* Defining the background, or "bar" */
    struct VisualInfo* thumb;     /* The definition of the thumb */
    struct CallBackList* CallBacks;
    CallBack exposeCallback;
    CallBack updateCallback;
    Slider nextSlider;
};

/* The support routines for general no-good */
Slider XfCreateSlider();
int XfDestroySlider();
int XfActivateSliderValue();
int XfDeactivateSlider();
int XfAddSliderVisual();
int XfAddSliderCallback();
int XfDelSliderCallback();
int XfSliderResponse();
int XfMoveSlider();
int XfSetSliderValue();
Slider XfGetSlider();
Slider XfEventSlider();
void XfResizeThumb();

#define XfActivateSlider(S, M)          XfActivateSliderValue(S, 0.0, M)
#define XfAddSliderBarVisual(S, V)      XfAddSliderVisual(S, V, 0)
#define XfAddSliderThumbVisual(S, V)    XfAddSliderVisual(S, V, 1)

/* The easy one */
#define XfGetSliderValue(S) (float)((S)->min + \
				    ((S)->value * ((S)->max - (S)->min)))

/* The global list of sliders */
extern Slider SliderList;

#endif /* _XF_SLIDER_H */
