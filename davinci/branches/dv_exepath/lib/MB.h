#ifndef _MB_H_
#define _MB_H_

struct tagMB {
    int             type;
    Display        *display;
    Window          parent;
    Window          window;
    int             width,
                    height;
    short           hi, lo, fg, bg;             /* 3-D colors */ 
    CallBack        function;                   /* user function */
    XButton         nextB;
    int             state;                      /* current state */

    int             level;      /* level of cascade */
    MButton         popup;      /* stack of popup windows */
    MButton         root;      /* stack of items */
    Window          grab;       /* transparent grab window (only for parent) */
    MenuItem       *menu;       /* menu hierarchy */
    MenuItem       *item;       /* specific menu item we are associated with */
	int selected;
};

struct tagMenuItem {
    char           *str;        /* text string to display */
    int             align;      /* text alignment in cell */
    XFontStruct    *font;       /* font to display string in */
    Pixmap          pixmap;     /* optional pixmap to display */
    int             p_width;    /* width of pixmap */
    int             p_height;   /* height of pixmap */
    void           *id;         /* identiy of this item */
    int             status;     /* display status of this item
                                 *   -1 - disabled
                                 *    0 - normal
                                 *  >=1 - selected (display pixmap)
                                 */

    int             nitems;     /* number of children */
    MenuItem       *items;      /* children */
};

#if defined(__STDC__) && defined(__LINT__)

MenuItem *XfAddMenuItem(MenuItem *,char *,int,
						XFontStruct *,Pixmap,int,int,void *);
char *XfMBSelectedText(MButton);
void *XfMBSelectedValue(MButton);
void SetMenuText(MButton, MenuItem *, char *);
MButton XfCreateMB(Display *,Window,int,int,int,int,short,short,short,short,
					MenuItem *,CallBack);

#else

MenuItem *XfAddMenuItem();
char *XfMBSelectedText();
void *XfMBSelectedValue();
MButton XfCreateMB();
void SetMenuText();

#endif

#endif                          /* _MB_H_ */
