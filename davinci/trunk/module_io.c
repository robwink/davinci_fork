#include <errno.h>
#include <stdlib.h>

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "module_io.h"

#if   defined(USE_HPUX_SHL)
#include <dl.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

extern void parse_error(char *fmt, ...);

/*
** daVinci module handle used for hiding the system specific 
** module handle.
*/
typedef struct {
#if   defined(USE_HPUX_SHL)
	shl_t handle;
#elif defined(_WIN32)
	HMODULE handle;
#else 
	void *handle;
#endif
} dvIModuleHandle;


#if  defined(_WIN32)
typedef __declspec(dllimport) void (*WIN_DLL_IMPORT)(void);
#endif


/*
** Opens a module file's handle, which happens to be a shared library.
*/
dvModuleHandle
open_dv_module_file(
	char *fname
)
{
	dvIModuleHandle *mh;

	mh = (dvIModuleHandle *)calloc(sizeof(dvModuleHandle), 1);
	if (mh == NULL){
		parse_error("%s", strerror(errno));
		return NULL;
	}

#if   defined(USE_HPUX_SHL)
	mh->handle = shl_load(fname, BIND_DEFERRED, BIND_VERBOSE);
#elif defined(_WIN32)
	mh->handle = LoadLibrary(fname);
#else  /* default case */
	mh->handle = dlopen(fname, RTLD_LAZY);
#endif /* USE_HPUX_SHL */

	/* Caution! mh->handle is "int" for HPUX and "void *" for others */

	if (!mh->handle){
	  /* I imagine this needs to be something different for Hockey Pux.
	     I don't have one to play with at the moment. */
		parse_error("Unable to open module %s. Reason: %s.",
					fname, dlerror());

		free(mh);
		return NULL;
	}

	return (dvModuleHandle)mh;
}

void
close_dv_module_file(dvModuleHandle emh)
{
	dvIModuleHandle *mh = (dvIModuleHandle *)emh;
#if   defined(USE_HPUX_SHL)
	shl_unload(mh->handle);
#elif defined(_WIN32)
	FreeLibrary(mh->handle);
#else /* default case */
	dlclose(mh->handle);
#endif /* USE_HPUX_SHL */

	free(mh);
}



/*
** Locates address of the specified function in the shared library
** addressed by the module handle.
*/
void *
locate_dv_module_func_in_slib(
	dvModuleHandle emh,
	char *func_name
)
{
	void *foo = NULL;
	dvIModuleHandle *mh = (dvIModuleHandle *)emh;

#if   defined(USE_HPUX_SHL)
	/*
	** Note! we are searching in functions list only.
	** Variables are handled differently in HPUX
	*/
	if (shl_findsym(&mh->handle, func_name, TYPE_PROCEDURE, &foo) < 0){ foo = NULL; }
#elif defined(_WIN32)
	foo = (WIN_DLL_IMPORT)GetProcAddress(mh->handle, func_name);
#else /* default case */
	foo = dlsym(mh->handle, func_name);
#endif /* USE_HPUX_SHL */

	return foo;
}

