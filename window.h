typedef struct tag_Window {
    int width;
    int height;
    int format;     /* data format of one data value */
    void *data;     /* contiguous memory to hold all data values */
    void **row;     /* pointers into data for start of each row */
} Window;

/*
** Rolling window functions
** 
** 
** Window * create_window(int width, int height, int format);
** 
**  Create a window structure of size width x height.  Format is ignore
**  currently.  Window is always of type FLOAT.  (but pass FLOAT anyway).
** 
** void load_window(Window *w, Var *obj, int x1, int y1, float ignore);
** 
**  Load a window structure with pixels from <obj> centered around 
**  the point <x1,y1>.  Values that fall off the edge of the window 
**  are assigned the <ignore> value.
** 
** void roll_window(Window *w, Var *obj, int x1, int y1, float ignore);
** 
**  Shift the rows in the Window <w> up by one, and fill in the bottom row.
**  This function doesn't do a lot of error checking, so if <x1,y1> isn't
**  exactly equal to <x1,y1-1>, bad things will happen.
** 
** void dump_window(Window *w);
** 
**  Print the contents of a Window 
** 
** void free_window(Window *w);
** 
**  Destroy a Window.
** 
** void load_row(Window *w, Var *obj, int x1, int y1, int row, float ignore);
** 
**  Internal function to load one row of data.
** 
*/

Window * create_window(int width, int height, int format);
void load_window(Window *w, Var *obj, int x1, int y1, float ignore);
void roll_window(Window *w, Var *obj, int x1, int y1, float ignore);
void dump_window(Window *w);
void free_window(Window *w);
void load_row(Window *w, Var *obj, int x1, int y1, int row, float ignore);
