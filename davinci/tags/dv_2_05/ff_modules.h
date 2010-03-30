#ifndef _FF_MODULES_H_
#define _FF_MODULES_H_

#include "module_io.h"
#include "darray.h"

typedef struct {
	char *name;
	char *ver;
} dvDepAttr;     /* daVinci dependency module attributes */

/*
** daVinci module-function description
*/
typedef struct {
	char *name;     /* name of the variable or a function to look up */
	void *ptr;      /* address of function within the loaded shared library */
} dvModuleFuncDesc;   /* description of a daVinci shared-lib member function */

typedef struct {
	dvModuleFuncDesc *fdesc; /* function descriptors */
	int  nfdesc;            /* count of function descriptors */ 
	dvDepAttr *dep;     /* module dependencies */
	int ndep;               /* count of module dependencies */
} dvModuleInitStuff; /* stuff returned by daVinci module init function */

struct _module {
	char *name;         /* module name */
	char *path;         /* "path/file_name" of the located module */
	char *ver;          /* module version */
	dvModuleHandle handle;       /* Opaque pointer to dvModuleHandle.
						   NULL for user modules */
	dvModuleInitStuff init_stuff; /* stuff returned by the module init function */
	Narray *functions;      /* Functions available through this module. */

	int   stage;        /* current stage in loading of the module */
};

/*
** Following is a list of stages that a module passes through
** while loading. The list is in the order of execution.
** These states define the amount of cleanup required on a module
** unload.
*/
enum MOD_LOAD_STAGE {
	MOD_UINIT,       /* uninitialized */
	MOD_LST_ADDED,   /* added to the list of loaded module */
	MOD_LOCATED,     /* located in path */
	MOD_OPENED,      /* module file opened */
	MOD_INIT,        /* init function of module executed successfully */
	MOD_FUNC_LOADED, /* module's functions loaded successfully */
	MOD_DEP_LOADED,  /* dependencies successfully loaded */
	MOD_VAR_ADDED    /* registered in the variable list */
};


typedef struct _module dvModule;

#ifdef _WIN32
#define DV_MOD_EXPORT __declspec(dllexport) 
#define DV_MOD_IMPORT __declspec(dllimport)
#else
#define DV_MOD_EXPORT
#endif /* _WIN32 */

void module_help(char *,  char *);

#endif /* _FF_MODULES_H_ */
