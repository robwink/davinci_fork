#ifndef IO_LOADMOD_H
#define IO_LOADMOD_H
#include "parser.h"

#define IO_MOD_READ        1
#define IO_MOD_WRITE       2
#define IO_MOD_READWRITE   3

#ifdef USE_HPUX_SHL
#define MODHANDLE shl_t
#elif defined(_WIN32)
#define MODHANDLE HMODULE
#else 
#define	MODHANDLE void *
#endif


/** structure used to store IO loadable module references **/

typedef struct _IOmod IOmod;
typedef IOmod * IOmodPtr;

/* modules are created and added to a heap list, and then when
   activated, are added to an active list.  Both lists are
   doubly linked lists. In both cases, the prev pointers are
   in a ring, and the next pointers end with the end of the list
   (the forward pointer is NULL). The top of the heap list is
   pointed to by the global pointer IOmodHeap and the top of the
   active list is pointed to by the global pointer IOmodList.
   The end of the active list is at IOmodList->prev_list and the
   end of the heap is IOmodHeap->prev_heap.

*/

struct _IOmod {
  char * modname;
  char * modpath;
  MODHANDLE dlhandle;
  unsigned char implements;
  Var * (*read_func)(char *); /* read: takes filename as arg */
  Var * (*write_func)(Var *, char *, char *, int); /* write: takes Davinci object, filename, type, and a force argument */
  IOmodPtr next_list;
  IOmodPtr prev_list;
  IOmodPtr next_heap;
  IOmodPtr prev_heap;
};

extern IOmodPtr IOmodList;
extern IOmodPtr IOmodHeap;

IOmodPtr new_io_module();
void destroy_io_module(IOmodPtr);
void init_io_modules();
IOmodPtr open_io_module(char *, char *);
Var * read_from_io_module(char *);
Var * write_to_io_module(Var *, char *, char *, int);
void finish_io_modules();
int close_io_module(char *);

#endif
