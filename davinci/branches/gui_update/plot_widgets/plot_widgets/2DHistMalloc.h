#ifndef _2DHISTMALLOC_H
#define _2DHISTMALLOC_H
/* next macros are introduced to avoid compatibily problems
** concerning memory management. All memory handling INSIDE the
** widget are performing by them. So they can be changed if needed,
** but in a such way that fullfil requerments below
*/

/*
** MYMALLOC - should either reuturn a pointer to allocated space or
** call exeption handler by itself if allocation impossible.
** That handler should not return, but should generate (and process) 
** a fatal error in the widget.
** In current implementation it is supposed that on the systems
** with virtual memory XtMalloc can always  allocate memory sucsessfuly
** so no error handler calls provided
*/ 

#define MYMALLOC(TYPE,POINTER,ARRAY_SIZE)				\
POINTER=(TYPE*)XtMalloc(sizeof(TYPE)*(ARRAY_SIZE))

#define MYCALLOC(TYPE,POINTER,ARRAY_SIZE)				\
POINTER=(TYPE*)XtCalloc((ARRAY_SIZE),sizeof(TYPE))

/* MYFREE should work in pare with MYMALLOC correctly and should
** do nothing and return NULL in case of its argumnent is NULL
*/

#define MYFREE(POINTER) XtFree((char*)POINTER)

/* XTDESTROYIMAGE should releaze XIMAGE->data too, assuming they were
** allocated by MYMALLOC. It should process XIMAGE==NULL case too.
** It is clamed that XtDestroyImage releaze this data provided XIMAGE 
** was created by XtCreateImage. The last function still does not allocate 
** data array by itself so some collisions with an "external" allocation 
** function (MYMALLOC) are possible in theory.
** This macro is introduced to solve such collision, if any, in an easy way
*/
#endif
