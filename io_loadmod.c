#include "io_loadmod.h"
#ifdef USE_HPUX_SHL
#include <dl.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

IOmodPtr IOmodList = NULL;
IOmodPtr IOmodHeap = NULL;

static void portable_dlclose(MODHANDLE closeme) {
  /* a simple cross platform dlclose routine */
#ifdef USE_HPUX_SHL
  shl_unload(closeme);
#elif defined(_WIN32)
  FreeLibrary(closeme);
#else
  dlclose(closeme);
#endif 
}

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

static void activate_module(IOmodPtr newmod) {
  if (newmod->prev_list != NULL) {
    parse_error("Module %s is already active.\n", newmod->modname);
    return;
  }
  if (IOmodList == NULL) { /* first active module */
    IOmodList = newmod;
    IOmodList->prev_list = newmod;
  }
  newmod->prev_list = IOmodList->prev_list;
  IOmodList->prev_list = newmod;
  newmod->prev_list->next_list = newmod;
  newmod->next_list = NULL; /* In case this is the first node */
  return;
}

static void deactivate_module(IOmodPtr killme) {
  if (killme->prev_list == NULL) {
    parse_error("Module %s is already inactive.\n", killme->modname);
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

static MODHANDLE portable_dlopen(char * fname) {
  /* a simple Cross platform dlopen routine */
  MODHANDLE rtn;
#ifdef USE_HPUX_SHL
  rtn = shl_load(fname, BIND_DEFERRED, BIND_VERBOSE);
  if (rtn == NULL) {
    parse_error("Error opening shared lib %s: %s\n", fname, strerror(errno));
  }
#elif defined(_WIN32)
  rtn = LoadLibrary(fname);
  if (rtn == NULL) {
    parse_error("Error opening DLL %s: Get a real operating system.\n");
  }
#else
  rtn = dlopen(fname, RTLD_LAZY); 
  if (rtn == NULL) {
    parse_error("Error opening shared lib %s: %s\n", fname, dlerror());
  }
#endif
  return rtn;
}

static void * portable_dlsym(MODHANDLE solib, char * function) {
  /* a simple cross platform dlsym routine */
  void * rtnfunc = NULL;
#ifdef USE_HPUX_SHL
  if (shl_findsym(solib, function, TYPE_PROCEDURE, &rtnfunc) < 0)
    rtnfunc = NULL;
#elif defined(_WIN32)
  rtnfunc = (WIN_DLL_IMPORT)GetProcAddress(solib, function);
#else
  rtnfunc = dlsym(solib, function);
#endif
}

IOmodPtr new_io_module() {
  /* return an allocated IOmod structure */
  IOmodPtr new_mod;
  if ((new_mod = (IOmodPtr)malloc(sizeof(IOmod))) == NULL) {
    parse_error("Cannot alloc a new IOmod struct.\n");
    return NULL;
  }
  /* initialize the struct */
  new_mod->modname = NULL;
  new_mod->dlhandle = NULL;
  new_mod->implements = 0;
  new_mod->read_func = NULL;
  new_mod->write_func = NULL;
  new_mod->next_list = NULL;
  new_mod->prev_list = NULL;
  if (IOmodHeap == NULL) {
    IOmodHeap = new_mod;
    new_mod->prev_heap = new_mod;
  }
  else {
    IOmodHeap->prev_heap->next_heap = new_mod;
    IOmodHeap->prev_heap = new_mod;
  }
  new_mod->next_heap = NULL;
  return new_mod;
}

void destroy_io_module(IOmodPtr killme) {
  if (killme == IOmodHeap) {
    IOmodHeap = killme->next_heap;
  }
  if (killme == IOmodList) {
    IOmodList = killme->next_list;
  }
  if (killme->prev_heap != NULL)
    killme->prev_heap->next_heap = killme->next_heap;
  if (killme->next_heap != NULL) 
    killme->next_heap->prev_heap = killme->prev_heap;
  if (killme->prev_list != NULL)
    killme->prev_list->next_list = killme->next_list;
  if (killme->next_list != NULL) 
    killme->next_list->prev_list = killme->prev_list;

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

IOmodPtr open_io_module(char * modname, char * modpath) {
  /* Open an IO module specified in the mod argument.
     if successful, returns a new IOmod structure, NULL if it fails.
  */
  IOmodPtr new_module = NULL;
  MODHANDLE cand;
  char * funcname;

  cand = portable_dlopen(modpath);
  if (cand == NULL) goto error_exit;
  new_module = new_io_module();
  new_module->dlhandle = cand;
  if ((new_module->modname = malloc(strlen(modname)+1)) == NULL) {
    parse_error("Can't malloc a string for IO module.\n");
    goto error_exit;
  }
  if ((new_module->modpath = malloc(strlen(modpath)+1)) == NULL) {
    parse_error("Can't malloc string for IO module.\n");
    goto error_exit;
  }
  if (duplicate_name(modname)) {
    parse_error("Duplicate module name '%s'.\n", modname);
    goto error_exit;
  }
  if ((funcname = malloc(strlen(modname)+8)) == NULL) {
    parse_error("Can't malloc string for IO module.\n");
    goto error_exit;
  }
  
  /* determine the module I/O function names, and if present,
     set the appropriate pointers. */

  sprintf(funcname, "%s_read", modname);
  if ((new_module->read_func = portable_dlsym(cand, funcname)) != NULL) {
    new_module->implements |= IO_MOD_READ;
  }
  sprintf(funcname, "%s_write", modname);
  if ((new_module->write_func = portable_dlsym(cand, funcname)) != NULL) {
    new_module->implements |= IO_MOD_WRITE;
  }


  return new_module;
 error_exit:
  /* destroy_io_module will handle the cleanup of any sort of
     shared library opens or memeory allocs we do in this routine 
  */
  if (new_module != NULL) destroy_io_module(new_module);
  return NULL;
}
  
