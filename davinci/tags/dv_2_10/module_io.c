#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "module_io.h"

#include "ltdl.h"

extern void parse_error(char *fmt, ...);

/*
** daVinci module handle used for hiding the system specific 
** module handle.
*/
typedef struct {
	lt_dlhandle handle;
} dvIModuleHandle;


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

	lt_dlinit();
	mh->handle = lt_dlopen(fname);

	if (!mh->handle){
		parse_error("Unable to open module %s. Reason: %s.",
					fname, lt_dlerror());

		free(mh);
		return NULL;
	}

	return (dvModuleHandle)mh;
}

void
close_dv_module_file(dvModuleHandle emh)
{
	dvIModuleHandle *mh = (dvIModuleHandle *)emh;
	lt_dlclose(mh->handle);
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

	foo = lt_dlsym(mh->handle, func_name);
	return foo;
}

