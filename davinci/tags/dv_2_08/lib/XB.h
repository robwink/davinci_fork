#ifndef _XB_H_
#define _XB_H_

#define XF_XB   0
#define XF_PB   1
#define XF_RB   2
#define XF_CB   3
#define XF_MB   4
#define XF_LB   5

/* typedef struct tagCB 		*MButton;	*/	/* MenuButton */
typedef struct tagAnyButton *AButton;	/* AnyButton */
typedef struct tagPB 		*PButton;	/* PushButton */
typedef struct tagRB 		*RButton;	/* RadioButton */
typedef struct tagCB 		*CButton;	/* CheckButton */
typedef struct tagMB 		*MButton;	/* Menu Button */
typedef struct tagLB 		*LButton;	/* Label Button */
typedef union _XButton 		*XButton;

typedef struct tagMenuItem MenuItem;


#include "PB.h"
#include "RB.h"
#include "CB.h"
#include "MB.h"
#include "LB.h"

struct tagAnyButton {
	int             type;
	Display        *display;
	Window          parent;
	Window          window;
	int             width, height;
	short           hi, lo, fg, bg;

	CallBack        function;
	XButton         nextB;

	int             state;
	char           *text;
	int             align;
	XFontStruct    *font;
	Pixmap          pixmap;
	int             pix_w;
	int             pix_h;
};

union _XButton {
	int             type;
	struct tagAnyButton XB;
	struct tagPB PB;
	struct tagRB RB; 
	struct tagCB CB; 
	struct tagMB MB; 
	struct tagLB LB; 
};

extern XButton XBList;

#define DEFCOLORS			-1,-1,-1,-1
#define XV_COLORS			XV_HI, XV_LO, XV_FG, XV_BG

#define XV_HI				XfColor(NULL, "#C6D5E2") 
#define XV_LO				XfColor(NULL, "#8B99B5")
#define XV_FG 				XfColor(NULL, "#000000")
#define XV_BG				XfColor(NULL, "#B2C0DC")

#define CENTER	0
#define LEFT	1
#define RIGHT	-1

#define CENTERTEXT(str)		str, 0, XfFont(NULL,NULL)
#define LEFTTEXT(str)		str, 1, XfFont(NULL,NULL)
#define RIGHTTEXT(str)		str,-1, XfFont(NULL,NULL)
#define NOTEXT				NULL, 0, NULL

#define XfResizeXB(XB, w, h)    XfPosXB(XB, MAXINT, MAXINT, w, h)
#define XfMoveXB(XB, x, y)      XfPosXB(XB, x, y, MAXINT, MAXINT)

#if defined(__STDC__) && defined(__LINT__)

#define XB_CALLBACK(proc)   void proc(XB_ARGS)
#define XB_ARGS     		XButton __xB, XEvent *__xE

int             XfActivateXB(XButton, int);
void            XfDeactivateXB(XButton);
int             XfPushXB(XButton, XEvent * E);
XButton         XfEventXB(XEvent *E);
void            XfPosXB(XButton, int, int, int, int);
void            XfDestroyXB(XButton);
XButton         XfCreateXB(Display *, Window, int, int, int, int,
						   short, short, short, short, 
						   CallBack, int);
void			RedrawXB(XButton);

#else

#define XB_CALLBACK(proc)   void proc(__xB, __xE) XButton __xB; XEvent *__xE;

int             XfActivateXB();
void            XfDeactivateXB();
int             XfPushXB();
XButton         XfEventXB();
void            XfPosXB();
void            XfDestroyXB();
XButton         XfCreateXB();
void			RedrawXB();

#endif		/* __STDC__ */

#define XfGetCallbackItem()		(__xB)
#define XfGetCallbackEvent()	(__xE)

#define XfActivatePB(PB, i)	XfActivateXB((XButton)PB, i)
#define XfDeactivatePB(PB)	XfDeactivateXB((XButton)PB)
#define XfDestroyPB(PB)		XfDestroyXB((XButton)PB)

#define XfActivateRB(RB, i)	XfActivateXB((XButton)RB, i)
#define XfDeactivateRB(RB)	XfDeactivateRB((XButton)RB)

#define XfActivateCB(CB, i)	XfActivateXB((XButton)CB, i)
#define XfDeactivateCB(CB)	XfDeactivateXB((XButton)CB)
#define XfDestroyCB(CB)		XfDestroyXB((XButton)CB)

#define XfActivateMB(MB, i)	XfActivateXB((XButton)MB, i)
#define XfDeactivateMB(MB)	XfDeactivateXB((XButton)MB)
#define XfDestroyMB(MB)		XfDestroyXB((XButton)MB)

#define XfActivateLB(LB, i)	XfActivateXB((XButton)LB, i)
#define XfDeactivateLB(LB)	XfDeactivateXB((XButton)LB)
#define XfDestroyLB(LB)		XfDestroyXB((XButton)LB)


#endif	 /* _XB_H_ */
