/*
 * Joystick.h
 *
 * The data structures and prototypes for the Joystick  class of tools.
 *
 * A Joystick consists of the Field and the Thumb. It can be moved around
 * within the field when buttons are pressed and held. The Thumb is defined
 * as a box, and is referenced by it's center. ButtonPress and Motion
 * events warp the center of the box to the pointer.
 *
 * $Header$
 *
 * $Log$
 * Revision 1.1  1999/06/16 03:24:23  gorelick
 * Initial revision
 *
 * Revision 1.1  1999/06/16 01:40:37  gorelick
 * Initial install
 *
 * Revision 0.2  91/09/23  17:55:05  17:55:05  ngorelic (Noel S. Gorelick)
 * *** empty log message ***
 * 
 * Revision 0.1  91/07/24  18:04:09  18:04:09  rray (Randy Ray)
 * *** empty log message ***
 * 
 *
 */

#ifndef _XF_JOYSTICK_H
#define _XF_JOYSTICK_H

/* For ease in typing */
typedef struct _Joystick* Joystick;

/* The basis for a Joystick. That is to say, no definition, no joystick! */
struct _Joystick
{
  Display* display;             /* Display the joystick is on */
  Window window;                /* Window associated with joystick */
  Window parent;                /* window's parent */
  int active;                   /* Joystick active/not active */
  int width, height;            /* Sizing of joystick */
  int x, y;                     /* X and Y position within parent window */
  int border_width;             /* width (in pixels) of border */
  unsigned long border_color;   /* Color of border */
  char name[256];               /* Name used to identify joystick */
  char* ext;                    /* An externally-visible piece of data */
  int* member;                  /* A pseudo-class identifier */
  int thumb_width;              /* Thumb size in X */
  int thumb_height;             /* Thumb size in Y */
  int thumb_max_x;              /* Maximum value along X axis */
  int thumb_max_y;              /* Maximum value along Y axis */
  int thumb_min_x;              /* Maximum value along X axis */
  int thumb_min_y;              /* Maximum value along Y axis */
  int thumb_cur_x;              /* Current pixel value along X axis */
  int thumb_cur_y;              /* Current pixel value along Y axis */
  int new_x;                    /* New X value prior to updating */
  int new_y;                    /* New Y prior to updating */
  struct VisualInfo* field;     /* Defining the background, or "field" */
  struct VisualInfo* thumb;     /* The definition of the thumb */
  struct CallBackList* CallBacks;
  CallBack exposeCallback;
  CallBack updateCallback;
  Joystick nextJoystick;
};

/* The support routines for general no-good */
Joystick XfCreateJoystick();
int XfDestroyJoystick();
int XfActivateJoystickValue();
int XfDeactivateJoystick();
int XfAddJoystickVisual();
int XfAddJoystickCallback();
int XfDelJoystickCallback();
int XfJoystickResponse();
int XfMoveJoystick();
Joystick XfGetJoystick();
Joystick XfEventJoystick();

#define XfActivateJoystick(J, M)          XfActivateJoystickValue(J, 0, 0, M)
#define XfAddJoystickFieldVisual(J, V)    XfAddJoystickVisual(J, V, 0)
#define XfAddJoystickThumbVisual(J, V)    XfAddJoystickVisual(J, V, 1)

/* The easy ones */
#define XfJoystickUpperX(J)     (J)->thumb_cur_x
#define XfJoystickUpperY(J)     (J)->thumb_cur_y
#define XfJoystickLowerX(J)     ((J)->thumb_cur_x + (J)->thumb_width)
#define XfJoystickLowerY(J)     ((J)->thumb_cur_y + (J)->thumb_height)

/* The global list of joysticks */
extern Joystick JoystickList;

#endif /* _XF_JOYSTICK_H */
