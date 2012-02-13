/*
 * VisInfo.h
 *
 * Definition of the VisualInfo structure and (pseudo) prototype of the
 * XfCreateVisual() function.
 *
 * $Header$
 *
 * $Log$
 * Revision 1.1  1999/06/16 03:24:24  gorelick
 * Initial revision
 *
 * Revision 1.1  1999/06/16 01:40:49  gorelick
 * Initial install
 *
 * Revision 0.1  91/07/24  18:04:13  18:04:13  rray (Randy Ray)
 * *** empty log message ***
 * 
 *
 */

#ifndef _XF_VISINFO_H
#define _XF_VISINFO_H

/* Values for the type selector in the Visual struct */
#define XfPixmapVisual 0
#define XfTextVisual 1
#define XfXImageVisual 2
#define XfTiledVisual 3
#define XfStippledVisual 4
#define XfOpaqueStippledVisual 5
#define XfSolidVisual 6
#define XfOutlineVisual 7
#define XfHersheyVisual 8

/* A visual element comprised of normal text */
struct TextVisual
{
  int align;                    /* 0 is center, 1 is left, 2 is right */
  XFontStruct *font;              /* The font to use in displaying */
  char text[256];               /* The actual text displayed */
};

/* A visual element comprised of a bitmap or pixmap */
struct PixmapVisual
{
  int depth;                    /* Bit depth of image */
  Pixmap map;                   /* Pixmap data */
};

/* A hershey font string */
struct HersheyVisual
{
	int cset;					/* Character set */
	float scale;				/* scale factor */
	float angle;				/* rotation */
	float align;                /* percentage from center (in positive X dir) */
	char text[256];
};

/* The structure combining image and text visuals into one treatment */
struct VisualInfo
{
  int x_pos, y_pos;             /* position relative to button's X & Y */
  int height, width;            /* Height and width of this little piece */
  unsigned long foreground;     /* Foreground color for this element */
  unsigned long background;     /* Background for element */
  int vtype;                    /* Type of element-- text or image */
  union Visual                  /* The needed data for displaying */
    {
      struct TextVisual t_vis;
      struct PixmapVisual p_vis;
      struct HersheyVisual h_vis;
      XImage* i_vis;
    } visual;
  struct VisualInfo* next;      /* Handle to the next bit of Visual info */
};


struct XfAnyWidget
{
  Display* display;             /* Display that tool is on */
  Window window;                /* Window ID of the tool */
  Window parent;                /* Parent's Window ID */
  int active;                   /* Is this tool active? */
  int width, height;            /* sizing of the tool */
  int x, y;                     /* X and Y position of tool wrt parent */
  int border_width;             /* width (in pixels) of tool's border */
  unsigned long border_color;   /* Color of border */
  char name[256];               /* Name used to identify tool */
  char* ext;                    /* An externally-visible piece of data */
  int* member;                  /* Pseudo-class identifier */
};

struct VisualInfo* XfCreateVisual();
void XfFreeVisual();

#endif /* _XF_VISINFO_H */

