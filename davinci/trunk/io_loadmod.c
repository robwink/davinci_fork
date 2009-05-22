#include "io_loadmod.h"

#define IOMOD_EXT ".dvio"
#if defined(_WIN32)
#define IOMOD_EXT ".DLL"
#endif
#include <errno.h>

#ifdef __MINGW32__
#define	strtok_r(s,d,p) strtok(s,d)
#endif


#define DV_MOD_LIBPATH "/usr/local/lib/davinci"

#define DV_MOD_ENV "DV_MOD_PATH"

#define yesno(n) (n ? "yes" : "no")

static int should_lt_dl_init = 1;

IOmodPtr IOmodList = NULL;
IOmodPtr IOmodHeap = NULL;
TypelistPtr AvailableTypes = NULL;

/* Static functions to maintain the IO modules data structures */

/* First off, provide a unified interface to dynamic library support */

#ifdef HAVE_LIBLTDL

static MODHANDLE portable_dlopen(char * fname) {
  /* a simple Cross platform dlopen routine */
  MODHANDLE hand;
  if (should_lt_dl_init) should_lt_dl_init = lt_dlinit();
  hand = lt_dlopen(fname);
  if (hand == NULL) {
    parse_error("lt_dlopen failed: %s", lt_dlerror());
  }
  return hand;
}

static void * portable_dlsym(MODHANDLE module, char * function) {
  /* a simple cross platform dlsym routine */
  return (void *)lt_dlsym(module, function);
}

static int  portable_dlclose(MODHANDLE closeme) {
  return lt_dlclose(closeme);
}
#else /* if we are not using libtool */


static MODHANDLE portable_dlopen(char * fname) {
#if defined(USE_HPUX_SHL)
  return shl_load(fname, BIND_DEFERRED, BIND_VERBOSE);
#elif defined(_WIN32)
  return LoadLibrary(fname);
#else  /* default case */
  return dlopen(fname, RTLD_LAZY);
#endif /* USE_HPUX_SHL */
}

static void * portable_dlsym(MODHANDLE module, char * function) {
#if  defined(USE_HPUX_SHL)
  void * funcptr = NULL;
  if (shl_findsym(&module, function, TYPE_PROCEDURE, &funcptr) < 0) funcptr = NULL;
  return funcptr;
#elif defined(_WIN32)
  return (WIN_DLL_IMPORT)GetProcAddress(module, function);
#else /* default case */
  return dlsym(module, function);
#endif /* USE_HPUX_SHL */
}

static int portable_dlclose(MODHANDLE closeme) {
  /* a simple cross platform dlclose routine */
#ifdef USE_HPUX_SHL
  shl_unload(closeme);
#elif defined(_WIN32)
  FreeLibrary(closeme);
#else
  dlclose(closeme);
#endif 
  return 0;
}

#endif /* HAVE_LIBLTDL */

/* data verification routines */

static int duplicate_name(char * check) {
  /* verifies that a module with this name is not already in the heap */
  IOmodPtr current;
  current = IOmodHeap;
  while (current) {
    if (!strcmp(check, current->modname)) return 1;
    current = current->next_heap;
  }
  return 0;
}

static int duplicate_filetype(char * check) {
  TypelistPtr cur;
  cur = AvailableTypes;
  while (cur != NULL) {
    if (!strcmp(cur->filetype, check)) {
      parse_error("Duplicate type '%s' already handled by module %s. Skipping.",
		  check, cur->handler->modname);
      return 1;
    }
    cur = cur->next;
  }
  return 0;
}

static IOmodPtr handler_for_type (char * type) {
  /* returns a pointer to the IOmod structure that handles the file type
     supplied as the argument */
  TypelistPtr cur;
  cur = AvailableTypes;
  while (cur != NULL) {
    if (!strcmp(cur->filetype, type)) return cur->handler;
    cur = cur->next;
  }
  return NULL;
}

static IOmodPtr module_for_name (char * name) {
  /* returns an IOmodule pointer given its name. gives NULL if not found. */
  IOmodPtr cur;
  cur = IOmodList;
  while (cur != NULL) {
    if (!strcmp(name, cur->modname)) return cur;
    cur = cur->next_list;
  }
  return NULL;
}

static void delete_types_for_handler (IOmodPtr iomod) {
  /* Purge the type chain of the types the supplied module handles */
  TypelistPtr cur, nextone;
  cur = AvailableTypes;
  while (cur != NULL) {
    if (!strcmp(iomod->modname, cur->handler->modname)) {
      if (cur == AvailableTypes) AvailableTypes = cur->next;
      if (cur->next != NULL) cur->next->prev = cur->prev;
      if (cur->prev->next != NULL) cur->prev->next = cur->next;
      nextone = cur->next;
      free(cur);
      cur = nextone;
    } else {
      cur = cur->next;
    }
  }
}


static void append_typelist(Typelist new[], IOmodPtr newmod) {
  /* Copy an array of typelists to the active type chain... used in
     module initialization */
  TypelistPtr dyntype;
  int tlcount = 0;
  while (1) {
    if (new[tlcount].filetype == NULL) break;
    if (duplicate_filetype(new[tlcount].filetype)) {tlcount++; continue;}
    if ((dyntype = malloc(sizeof(Typelist))) == NULL) {
      parse_error("malloc() failed in append_typelist. Bad things from here on out.");
      return;
    }
    dyntype->filetype = NULL;
    dyntype->handler = NULL;
    dyntype->prev = NULL;
    dyntype->next = NULL;
    if ((dyntype->filetype = malloc(strlen(new[tlcount].filetype)+1)) == NULL) {
      parse_error("malloc() failed in append_typelist. Bad things from here out.");
      return;
    }
    strcpy(dyntype->filetype, new[tlcount].filetype);
    dyntype->handler = newmod;
    if (AvailableTypes == NULL) AvailableTypes = dyntype;
    dyntype->prev = AvailableTypes->prev;
    if (dyntype->prev != NULL) dyntype->prev->next = dyntype;
    AvailableTypes->prev = dyntype;
    dyntype->next = NULL;
    tlcount++;
  }
  return;
}


static void activate_module(IOmodPtr newmod) {
  /* Take a module and and stick it on the active list. */
  if (newmod->prev_list != NULL) {
    parse_error("Module %s is already active.", newmod->modname);
    return;
  }
  if (IOmodList == NULL) { /* first active module */
    IOmodList = newmod;
    IOmodList->prev_list = newmod;
  }
  newmod->prev_list = IOmodList->prev_list;
  newmod->prev_list->next_list = newmod;
  IOmodList->prev_list = newmod;
  newmod->next_list = NULL; /* In case this is the first node */
  return;
}

static void deactivate_module(IOmodPtr killme) {
  /* Deactivates a module, but doesn't delete it.  It is kept loaded,
     but Davinci code cannot get to it until it is reactivated.
  */
  if (killme->prev_list == NULL) {
    parse_error("Module %s is already inactive.", killme->modname);
  }
  if (killme == IOmodList) {
    IOmodList = killme->next_list;
  }
  killme->next_list->prev_list = killme->prev_list;
  if (killme->prev_list->next_list != NULL) 
    killme->prev_list->next_list = killme->next_list;
  killme->prev_list = NULL;
  killme->next_list = NULL;
  return;
}

static IOmodPtr new_io_module() {
  /* return an allocated and initialized IOmod structure, connected into
     the IOmod heap.
  */
  IOmodPtr new_mod;
  if ((new_mod = (IOmodPtr)malloc(sizeof(IOmod))) == NULL) {
    parse_error("Cannot alloc a new IOmod struct.");
    return NULL;
  }
  /* initialize the struct */
  new_mod->modname = NULL;
  new_mod->dlhandle = NULL;
  new_mod->implements = 0;
  new_mod->read_func = NULL;
  new_mod->write_func = NULL;
  new_mod->load_pds_func = NULL;
  new_mod->next_list = NULL;
  new_mod->prev_list = NULL;
  if (IOmodHeap == NULL) {
    IOmodHeap = new_mod;
    new_mod->prev_heap = new_mod;
  }
  else {
    IOmodHeap->prev_heap->next_heap = new_mod;
    new_mod->prev_heap = IOmodHeap->prev_heap;
    IOmodHeap->prev_heap = new_mod;
  }
  new_mod->next_heap = NULL;
  return new_mod;
}

static void destroy_io_module(IOmodPtr killme) {
  /* blow the IOmod away, fixing the linked lists to maintain integrity */
  delete_types_for_handler(killme);
  if (killme == IOmodHeap) {
    IOmodHeap = killme->next_heap;
  }
  if (killme == IOmodList) {
    IOmodList = killme->next_list;
  }
  if (IOmodHeap != NULL) {
    if (killme->prev_heap != NULL)
      if (killme->prev_heap->next_heap != NULL) 
	killme->prev_heap->next_heap = killme->next_heap;
    if (killme->next_heap != NULL) 
      killme->next_heap->prev_heap = killme->prev_heap;
    if (killme == IOmodList->prev_heap)
      IOmodList->prev_heap = killme->prev_heap;
  }
  if (IOmodList != NULL) {
    if (killme->prev_list != NULL)
      if (killme->prev_heap->next_heap != NULL)
	killme->prev_list->next_list = killme->next_list;
    if (killme->next_list != NULL) 
      killme->next_list->prev_list = killme->prev_list;
    if (killme == IOmodList->prev_list)
      IOmodList->prev_list = killme->prev_list;
  }
  if (killme->dlhandle != NULL) {
    portable_dlclose(killme->dlhandle);
    killme->dlhandle = NULL;
  }
  if (killme->modname != NULL) {
    free(killme->modname);
    killme->modname = NULL;
  }
  if (killme->modpath != NULL) {
    free(killme->modpath);
    killme->modpath = NULL;
  }

  free(killme);
  return;
} 


static int close_io_module(char * modname) {
  /* close an IOmod identifed by name */
  IOmodPtr cur;

  cur = IOmodList;
  while (cur != NULL) {
    if (!strcmp(cur->modname, modname)) {
      destroy_io_module(cur);
      break;
    }
    cur = cur->next_list;
  }
  if (cur == NULL) {
    parse_error("Module '%s' is not loaded.", modname);
    return 0;
  }
  return 1;
}

static IOmodPtr open_io_module(char * modname, char * modpath) {
  /* Open an IO module specified in the mod argument.
     if successful, returns a new IOmod structure, NULL if it fails.
  */
  IOmodPtr new_module = NULL;
  MODHANDLE cand;
  void (*init_func)(TypelistPtr *);
  TypelistPtr init_list = NULL;

  cand = portable_dlopen(modpath);
  if (cand == NULL) {
      parse_error("portable_dlopen of %s failed.", modpath);
      goto error_exit;
  }
  new_module = new_io_module();
  new_module->dlhandle = cand;
  if ((new_module->modname = malloc(strlen(modname)+1)) == NULL) {
    parse_error("Can't malloc a string for IO module.");
    goto error_exit;
  }
  if ((new_module->modpath = malloc(strlen(modpath)+1)) == NULL) {
    parse_error("Can't malloc string for IO module.");
    goto error_exit;
  }
  if (duplicate_name(modname)) {
    parse_error("Duplicate module name '%s'.", modname);
    goto error_exit;
  }
  strcpy(new_module->modname, modname);
  strcpy(new_module->modpath, modpath);

  /* determine the module I/O function names, and if present,
     set the appropriate pointers. */

  if ((new_module->read_func = portable_dlsym(cand, "dv_modread")) != NULL) {
    new_module->implements |= IO_MOD_READ;
  }
  if ((new_module->write_func = portable_dlsym(cand, "dv_modwrite")) != NULL) {
    new_module->implements |= IO_MOD_WRITE;
  }
  if ((new_module->load_pds_func = portable_dlsym(cand, "dv_modload_pds")) != NULL) {
    new_module->implements |= IO_MOD_LOAD_PDS;
  }
  if ((init_func = portable_dlsym(cand, IO_INIT_MOD)) != NULL) {
    /* call the initial function */
    init_func(&init_list);
  }
  else {
    parse_error("ERROR: No %s function in module.\nIs %s really a Davinci IO module?", IO_INIT_MOD, modpath);
    goto error_exit;
  }
  
  if (new_module->implements == 0) {
    parse_error("ERROR: Module '%s' doesn't implement any usable methods!");
    goto error_exit;
  }
  append_typelist(init_list, new_module);
  /* Happy!  It all works...  activate the module */
  activate_module(new_module);
  
  return new_module;
 error_exit:
  /* destroy_io_module will handle the cleanup of any sort of
     shared library opens or memeory allocs we do in this routine 
  */
  if (new_module != NULL) destroy_io_module(new_module);
  return NULL;
}

static char * get_path_for_module(char * modname) {
  /* attmepts to locate a module in the DV_MOD_PATH path to use as the
     path for the module */
  char * env, * lib_path = NULL, * strtok_rbuf;
  char * path_rtn = NULL, * curdir;
  struct stat statbuf;
  if ((env = getenv(DV_MOD_ENV)) == NULL) 
    lib_path = strdup(DV_MOD_LIBPATH);
  else 
    lib_path = strdup(env);
  if (lib_path == NULL) {
    parse_error("strdup() failed!");
    goto error_exit;
  }
  /* prime the strtok_r */
  curdir = strtok_r(lib_path, ":", &strtok_rbuf);
  while (curdir != NULL) {
    /* fprintf(stderr, "Parsing path:current dir: %s\n", curdir); */
    if ((path_rtn = malloc(strlen(curdir)+strlen(modname)+strlen(IOMOD_EXT)+2))
	== NULL) {
      parse_error("malloc() failed!");
      goto error_exit;
    }
    sprintf(path_rtn, "%s/%s%s", curdir, modname, IOMOD_EXT);
    /* stat the file. If it exists, return it... else ditch it and try the
       next directory */
    if (stat(path_rtn, &statbuf) == 0) goto error_exit; /*not really an error */
    curdir = strtok_r(NULL, ":", &strtok_rbuf);
    free(path_rtn); /* not hit, so deallocate this buffer and move on with life */
    path_rtn = NULL;
  }
  
 error_exit:
  if (lib_path != NULL) free(lib_path);
  /* if (strtok_rbuf != NULL) free(strtok_rbuf); */
  return path_rtn;
}
  


/* Now, for the public interface to this mess. :-) */

/* The command interface to load, unload, and list modules */

Var * ff_insmod(struct _vfuncptr * f, Var *args) {
  Alist alist[2];
  char * mname, * mpath;
  alist[0] = make_alist("module", ID_STRING, NULL, &mname);
  alist[1].name = NULL;
  if (parse_args(f, args, alist) == 0) {
    parse_error("Argument parse failed.");
    return NULL;
  }
  if (mname == NULL) {
    parse_error("Module name required.");
    return NULL;
  }
  /* build the module path from the module name. */
  if ((mpath = get_path_for_module(mname)) == NULL) {
    parse_error("Module '%s%s' not found in module path.", mname, IOMOD_EXT);
    return NULL;
  }
  /* fprintf(stderr, "Found module @: %s", mpath); */
  /* try to activate the IO module */
  if (open_io_module(mname, mpath) == NULL) {
    parse_error("insmod for %s failed.", mname);
    return NULL;
  }
  return NULL;
}

Var * ff_rmmod(struct _vfuncptr *f, Var *args) {
  Alist alist[2];
  char *mname = NULL;
  alist[0] = make_alist("module", ID_STRING, NULL, &mname);
  alist[1].name = NULL;
  if (parse_args(f, args, alist) == 0) {
    parse_error("Argument parse failed.");
    return NULL;
  }
  if (mname == NULL) {
    parse_error("Module name required.");
    return NULL;
  }
  /* delete the module */
  close_io_module(mname);
  return NULL;
}

Var * ff_lsmod(struct _vfuncptr *f, Var *args) {
  /* takes no arguments */
  IOmodPtr curmod;
  TypelistPtr curtype;
  Alist alist[2];
  char read, write, loadpds, * mname = NULL;
  if (IOmodList == NULL) {
    parse_error("No IO modules currently resident.");
    return NULL;
  }
  alist[0] = make_alist("module", ID_STRING, NULL, &mname);
  alist[1].name = NULL;
  if (parse_args(f, args, alist) == 0) {
    parse_error("Argument parse failed.");
    return NULL;
  }
  if (mname == NULL) { 
    /* no args, list all modules */
    curmod = IOmodList;
    printf("Currently loaded IO modules:\n");
    printf("Module name              Read Write LoadPDS Path\n");
    printf("-----------              ---- ----- ------- ----\n");
    while (curmod != NULL) {
      if (curmod->implements & IO_MOD_READ) read = 'X'; else read = ' ';
      if (curmod->implements & IO_MOD_WRITE) write = 'X'; else write = ' ';
      if (curmod->implements & IO_MOD_LOAD_PDS) loadpds = 'X'; else loadpds = ' ';
      printf("%-24.24s  %c     %c     %c    %s\n", curmod->modname, read,
		  write, loadpds, curmod->modpath);
      curmod = curmod->next_list;
    }
  }
  else {
    /* mname is set, list the detail of the module requested */
    curtype = AvailableTypes;
    if ((curmod = module_for_name(mname)) == NULL) {
      parse_error("No module '%s' is currently loaded.", mname);
      return NULL;
    }
    printf("Module name   : %s\n", curmod->modname);
    printf("Module path   : %s\n", curmod->modpath);
    printf("Supports read : %s\n", yesno(curmod->implements & IO_MOD_READ));
    printf("Supports write: %s\n", yesno(curmod->implements & IO_MOD_WRITE));
    printf("File Types    : ");
    while (curtype != NULL) {
      if (!strcmp(curtype->handler->modname, curmod->modname))
	printf("%s ", curtype->filetype);
      curtype = curtype->next;
    }
    printf("\n");
  }
  return NULL;  
}

/* The read and write methods to be called from the read and write commands
   in Davinci */

Var * load_pds_from_io_module(FILE * fh, char * fname) {
  /* this function puts a hook into the load_pds functions. */
  IOmodPtr use_module;
  Var * rtnval = NULL;
  if (IOmodList == NULL) goto error_exit;
  use_module = IOmodList;
  while (use_module != NULL) {
      if (use_module->implements & IO_MOD_LOAD_PDS)
          rtnval = use_module->load_pds_func(fh, fname);
      if (rtnval != NULL) break;
      fseek(fh, 0, SEEK_SET);
      use_module=use_module->next_list;
  }
  if (rtnval == NULL) {
      goto error_exit;
  }
  error_exit:
  return rtnval;
}  

Var * read_from_io_module(FILE * fh, char * fname) {
  /* this function implements the read function for a module.  It determines
     the proper IO module by calling the read method on each module with a 
     read method until one succeeds.
  */
  IOmodPtr use_module;
  Var * rtnval = NULL;
  if (IOmodList == NULL) goto error_exit; /* silently exit if there are no IO modules */

  /* Iterate through modules that implement a reader
     until one can read the file successfully */
  use_module = IOmodList;
  while (use_module != NULL) {
    if (use_module->implements & IO_MOD_READ) 
      rtnval = use_module->read_func(fh, fname);
    if (rtnval != NULL) break;
    fseek(fh, 0, SEEK_SET); /* rewind the file to the start */
    use_module=use_module->next_list;
  }
  if (rtnval == NULL) {
    parse_error("Could not find appropriate module to read file.\n");
    goto error_exit;
  }
  /* TODO: check to see if rtnval is a string object, and print the string
     object as an error message */
 error_exit:
  return rtnval;
}

Var * write_to_io_module(Var * dv_obj, char * filename, 
				char * type, int force) {

  /* this is the implementation of writing using the io module function.
     It selects the proper module based on the type argument, and then
     calls that module's write method.  If the module doesn't support
     writing, the write aborts */
  IOmodPtr use_module;
  int call_return = 0;
  FILE * fh;
  struct stat stat_buf;
  if ((use_module = handler_for_type(type)) == NULL) {
    parse_error("No handler for type '%s'.", type);
    goto error_exit;
  }
  if (!(use_module->implements & IO_MOD_WRITE)) {
    parse_error("Module '%s' for handling file type '%s' doesn't support file writing.", 
		use_module->modname, type);
    goto error_exit;
  }
  /* stat the file if we're not forcing overwrite to see if it exists.
     Abort if it does. */

  if ((force == 0) && (stat(filename, &stat_buf) == 0)) {
    parse_error("File '%s' already exists.  Use force option to overwrite.",
		filename);
    goto error_exit;
  }
  
  /* check if we can open this file for write.  If not, abort */

  if ((fh = fopen(filename, "wb")) == NULL) {
    parse_error("Cannot open '%s' for writing: %s", filename, strerror(errno));
    goto error_exit;
  }

  /* good to go!  call the write module's write method */
  call_return = use_module->write_func(dv_obj, type, fh, filename);
  fclose(fh);
 error_exit:
  return newInt(call_return);
}

int iomod_handler_for_type(char * type) {
  /* a public routine to check if there's a module handler for a type */
  if (handler_for_type(type) != NULL) return 1; else return 0;
}
