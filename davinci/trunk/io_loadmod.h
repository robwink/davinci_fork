#ifndef IO_LOADMOD_H
#define IO_LOADMOD_H
#include "parser.h"

#define IO_MOD_READ        1
#define IO_MOD_WRITE       2
#define IO_MOD_READWRITE   3

#ifdef HAVE_LIBLTDL
/* Libtool's portable dl interface */
#include <ltdl.h>
#define MODHANDLE lt_dlhandle
#else
/* Homebrew what we need ourselves... less reliable, but should work for many
   platforms */
#ifdef USE_HPUX_SHL
#define MODHANDLE shl_t
#elif defined(_WIN32)
#define MODHANDLE HMODULE
#else 
#define	MODHANDLE void *
#endif /* _WIN32 */
#endif /* HAVE_LIBLTDL */

#define IO_INIT_MOD "dv_iomod_init"

/** structure used to store IO loadable module references **/

typedef struct _IOmod IOmod;
typedef IOmod * IOmodPtr;

/** structure used to maintain state on which modules handle which types **/

typedef struct _TypeList Typelist;
typedef Typelist * TypelistPtr;


struct _TypeList {
  char * filetype;
  IOmodPtr handler;
  TypelistPtr next;
  TypelistPtr prev;
};

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
  Var * (*read_func)(FILE *); /* read: takes filename as arg */
  int (*write_func)(Var *, char *, FILE *); /* write: takes Davinci object,type and a filehandle arg */
  IOmodPtr next_list;
  IOmodPtr prev_list;
  IOmodPtr next_heap;
  IOmodPtr prev_heap;
};

extern IOmodPtr IOmodList;
extern IOmodPtr IOmodHeap;

extern TypelistPtr AvailableTypes;

Var * ff_insmod(struct _vfuncptr *, Var *);
Var * ff_rmmod(struct _vfuncptr *, Var *);
Var * ff_lsmod(struct _vfuncptr *, Var *);
Var * read_from_io_module(FILE *);
Var * write_to_io_module(Var *, char *, char *, int);
int iomod_handler_for_type(char *);

#endif

/* API for the IO module:

You must supply this function:

dv_iomod_init(TypelistPtr *) 

You want to return an array of Typelist which you will define thus:


Typelist t[] = {
   {"type1", NULL, NULL, NULL},
   {"type2", NULL, NULL, NULL},
   {NULL, NULL, NULL, NULL}
};

Where typen  is the file types you want the module to support.  If you have
a read only module, you can just return null, since the types are only to
distinguish which type you wish to write.  A module can support any number
of file types.  Beware though that modules first loaded take priority over
subsequent modules, and if there's type overlap, it is the first module that
is in charge of that filetype.

You also will want to define at least one (and probably both) of the 
following methods. While not *strictly* required, a module is pretty much
worthless without at least one of them:

Var * dv_modread(FILE * fh);
This is for reading, and should return the object read in if successful,
NULL if you can't handle the file, or a Davinci string object if you're sure
you are supposed to handle it, but encounter errors.  The string you supply
will be printed as an error message and the read will abort.  You are passed
an open read_only ANSI file handle which is positioned at the beginning of the 
file. DO NOT CLOSE THE FILEHANDLE IN YOUR CODE.  The caller will handle the 
cleanup for you.

int dv_modwrite(Var * data_to_write, char * filetype, FILE * fh);
This is for writing, and should return 1 for success, 0 for failure. The first
argument is a davinci object you will write, the second argument identifies
the file type the user requested (so you know in case you are handling multiple
types), and the third is an open write_only ANSI file handle to a zero byte 
file. DO NOT CLOSE THE FILEHANDLE IN YOUR CODE.

In both cases, the file is opened in binary mode, which is pretty well
meaningless on UNIX-style systems, but could be important for poor sots
condemned to run Windows.

*/
