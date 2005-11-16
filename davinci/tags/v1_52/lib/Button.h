/*
 * Button.h
 *
 * The data structure declarations and functional prototypes for the
 * Button class of interface tools.
 *
 * Button as defined here is an object that has 1 or more states (numbered
 * from 0 to n-1 for consideration). For each state as defined, there
 * exists a list of callback-like routines, general display and color
 * information, and visual information. There are some data items that
 * exist in only one instance for each button, such as size and position
 * information.
 *
 * $Header$
 *
 * $Log$
 * Revision 1.1  1999/06/16 03:24:23  gorelick
 * Initial revision
 *
 * Revision 1.1  1999/06/16 01:40:32  gorelick
 * Initial install
 *
 * Revision 0.2  91/09/23  17:55:00  17:55:00  ngorelic (Noel S. Gorelick)
 * *** empty log message ***
 * 
 * Revision 0.1  91/07/24  18:04:05  18:04:05  rray (Randy Ray)
 * *** empty log message ***
 * 
 * Revision 1.1  91/07/18  17:06:28  17:06:28  ngorelic (Noel S. Gorelick)
 * Initial revision
 * 
 *
 */

#ifndef _XF_BUTTON_H
#define _XF_BUTTON_H

/* Information necessary to define a state: callbacks and visual data */
struct ButtonState
{
  struct CallBackList* CallBacks; /* List of call backs for specific state */
  struct VisualInfo* Visuals;   /* Visual elements for button */
};

/* For typing ease */
typedef struct _Button* Button;

/* The underlying structure, defining the definitive button */
struct _Button
{
  Display* display;             /* Display that button is on */
  Window window;                /* Window ID of the button */
  Window parent;                /* Parent's Window ID */
  int active;                   /* Is this button active? */
  int width, height;            /* sizing of the button */
  int x, y;                     /* X and Y position of button wrt parent */
  int border_width;             /* width in pixels of button's border */
  unsigned long border_color;   /* color for window's border */
  char name[256];               /* Name used to identify the button */
  void *ext;                    /* An externally-visible piece of data */
  void *member;                  /* A pseudo-class identifier */
  int state;                    /* Current state of the button */
  int maxstate;                 /* Number of states */
  int noAutoExpose;             /* Don't Handle Exposure Events automatically */
  struct ButtonState** States;  /* Dynamically allocated state data */
  CallBack exposeCallback;      /* Callback action on expose events */
  CallBack updateCallback;      /* Callback action forced, to update */
  Button nextButton;            /* Pointer for linked list */
};

/* The support routines for general manipulation */
#if defined(__STDC__) && defined(__LINT__) 

int XfButtonPush(Button, XEvent *);
Button XfGetButton(char *);
int XfDestroyButton(Button);
int XfAddButtonCallback(Button, int, CallBack, CallBack);
int XfDelButtonCallback(Button, int, CallBack);
int XfAddButtonVisual(Button, int, struct VisualInfo *);
Button XfCreateButton(Display *, Window, int, int, int, int, int,
                        long unsigned int, char *, int);
Button XFCreateButton(Display *, Window, int, int, int, int, int,
                        long unsigned int, long unsigned int, char *, int);
int XfMoveButton(Button);
int XfActivateButtonState(Button, int, long unsigned int);
int XfDeactivateButton(Button);
void defaultButtonCallback(Button, XEvent *);
Button XfEventButton(XEvent *);
void XfNoAutoExposeButton(Button);
void ResizeButton(Button, int, int, int, int);
void SetButtonText(Button, char *);
Button Make2State(Display *, Window, XFontStruct *, int, int, int, int, 
                    int, unsigned int, unsigned int, unsigned int, char *);
Button Make2State3D(Display *, Window, XFontStruct *, int, int, int, int, 
                    int, unsigned int, unsigned int, unsigned int, char *);
char *GetButtonText(Button);
void SetButtonState(Button, int);

#else

int XfButtonPush();
Button XfGetButton();
int XfDestroyButton();
int XfAddButtonCallback();
int XfDelButtonCallback();
int XfAddButtonVisual();
Button XfCreateButton();
Button XFCreateButton();
int XfMoveButton();
int XfActivateButtonState();
int XfDeactivateButton();
void defaultButtonCallback();
Button XfEventButton();
void XfNoAutoExposeButton();
void ResizeButton();
void SetButtonText();
Button Make2State();
Button Make2State3D();
char *GetButtonText();
void SetButtonState();

#endif

#define XfActivateButton(B, mask)       XfActivateButtonState(B, 0, mask)

/* One application that is too easy to waste code on */
#define XfButtonState(B) (B)->state

/* The global list of buttons */
extern Button ButtonList;

#endif /* _XF_BUTTON_H */
