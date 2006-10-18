/*
** Formatting: tabwidth=4 shiftwidth=4
**
** File: ff_modules.c  - tightly dependent on darray.c::Narray
**
** Contains code for loading vanilla modules.
**
*/

#include <stddef.h>
#include "parser.h"
#include <errno.h>
#include "darray.h"
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#else
#include <dirent.h>
#include <libgen.h>
#endif /* _WIN32 */
#include "module_io.h"

extern int debug;
extern int VERBOSE;

/*
** Value of debug must be greater than DEBUG_THRESHOLD to produce
** debug info.
*/
#define DEBUG_THRESHOLD 2

/*
** A global list of modules currently loaded and available.
*/
Narray *loaded_modules = NULL;



static const char ver_any_str[] = "(any)";
#define printable_ver_str(ver) ((ver)? (ver): ver_any_str)

/* daVinci module file extension */
static const char DVM_EXT[] = "so";

/* Environment variable for module lookup */
static const char DV_MOD_PATH_ENV[] = "DV_MOD_PATH";

/* Default module lookup path if the environment is not set */
static const char DV_MOD_PATH_DEF[] = "/usr/local/lib/davinci";

static char DV_MODULE_INIT_FUNC_NAME[] = "dv_module_init";
static char DV_MODULE_FINI_FUNC_NAME[] = "dv_module_fini";

typedef int (* dvModuleInitFunc)(const char *called_as,
				 dvModuleInitStuff *init_stuff);
typedef int (* dvModuleFiniFunc)(const char *called_as);



static int load_dv_module(char *mod_fname, char *ver, int offset, dvModule *mod);
static int init_dv_module(dvModule *m);
Var *new_module(char *module_name);
static int load_dv_module_functions(dvModule *m);
static void free_module_function(vfuncptr fptr);
static int autoload_dep(dvDepAttr *dep, int ndep);
void del_module(Var *mv);
static int uninit_dv_module(dvModule *m);
int unload_dv_module(char *module_name);


/*******************************************************************/
/** daVinci load_module(mod="module_name") function.              **/
/*******************************************************************/

Var *
ff_load_dv_module(
	vfuncptr func,
	Var      *args
	)
{
	int    ac;
	Var  **av;
	Var   *v_return = NULL;
	char  *module_name = NULL;

	Alist  alist[2]; /* arguments list */

	alist[0] = make_alist("mod", ID_STRING, NULL, &module_name);
	alist[1].name = NULL;

	if (parse_args(func, args, alist) == 0) {
		parse_error("%s(): argument parsing failed.", func->name);
		return NULL;
	}

	if (module_name == NULL){
		parse_error("%s(): \"%s\" must be specified.",
			func->name, alist[0].name);
		return NULL;
	}

	if (get_global_sym(module_name)){
		parse_error("%s(): Variable %s already exists in global space. "
			"Module load aborted.", func->name, module_name);
		return NULL;
	}

	v_return = new_module(module_name);
	if (v_return){
		/*
		** Actually load the module into the daVinci variable
		** just created.
		*/
		if (!load_dv_module(module_name, NULL, 1, &V_MODULE(v_return))){
			free_var(v_return);
			return NULL;
		}
		/* stick the symbol into the symbol table */
		/* sym_put(global_scope(), v_return); */
		put_global_sym(v_return);
		V_MODULE(v_return).stage = MOD_VAR_ADDED;
	}

	return v_return;
	/* return NULL; */
}


/*******************************************************************/
/** daVinci unload_module(mod="module_name") function.            **/
/*******************************************************************/

Var *
ff_unload_dv_module(
	vfuncptr func,
	Var *args
)
{
	int ac;
	Var **av;
	char *module_name = NULL;

	Alist alist[2]; /* arguments list */

	alist[0] = make_alist("mod", ID_STRING, NULL, &module_name);
	alist[1].name = NULL;

	if (parse_args(func, args, alist) == 0) {
		parse_error("%s(): argument parsing failed.", func->name);
		return NULL;
	}

	if (module_name == NULL){
		parse_error("%s(): \"%s\" must be specified.",
			func->name, alist[0].name);
		return NULL;
	}

	unload_dv_module(module_name);

	return NULL;
}


/*******************************************************************/
/** daVinci list_modules() function.                              **/
/*******************************************************************/

Var *
ff_list_dv_modules(
	vfuncptr func,
	Var *args
	)
{
	char  *mod_name;
	Var   *mod_var;
	Var   *mlst;
	int    i, n;
	char **mnlst;
	int    len;
	dvModule *m;
	

	n = Narray_count(loaded_modules);

	mnlst = (char **)calloc(n, sizeof(char *));
	if (mnlst == NULL){ return NULL; }

	for (i = 0; i < n; i++){
		Narray_get(loaded_modules, i, &mod_name, (void **)&mod_var);
		m = &V_MODULE(mod_var);

		len = strlen(mod_name)+1;
		if (m->ver){
			len += strlen(m->ver) + 1;
		}

		mnlst[i] = (char *)calloc(len, sizeof(char));

		if (mnlst[i] == NULL){

			/* memory failure */

			parse_error("Mem allocation error.\n");

			while(--i >= 0){ free(mnlst[i]); }
			free(mnlst);

			return NULL;
		}

		if (m->ver && strcmp(m->ver, "") == 0){
			/* include version */
			sprintf(mnlst[i], "%s.%s", mod_name, m->ver);
		}
		else {
			/* no version associated with the module */
			sprintf(mnlst[i], "%s", mod_name);
		}

	}

	mlst = newText(n, mnlst);

	return mlst;
}

/***************************************************************/
/** Interface Functions                                       **/
/***************************************************************/


Var *
pp_call_dv_module_func(
	vfuncptr f,
	Var *args
)
{
	/*
	** Call function with the specified args-list and return
	** whatever the function returns.
	*/

	return (f->fptr)(f, args);
}

void
print_module(
	dvModule *m
)
{
	int i, n;
	char *func_name;

	parse_error("Module %s:", m->name);
	if (m->ver){ parse_error("\tversion: %s", m->ver); }
	parse_error("\tfull path: %s", m->path);

	n = m->init_stuff.ndep;
	parse_error("\t%d dependencies:", n);
	for (i = 0; i < n; i++){
		parse_error("\t\t%s ver:%s", m->init_stuff.dep[i].name,
			printable_ver_str(m->init_stuff.dep[i].ver));
	}

	n = Narray_count(m->functions);
	parse_error("\t%d functions:", n);
	for (i = 0; i < n; i++){
		Narray_get(m->functions, i, &func_name, NULL);
		parse_error("\t\t%s", func_name);
	}
}

void
pp_print_module_var(
	Var *mv
)
{
	print_module(&V_MODULE(mv));
}


/*
** This function keeps track of the module dependencies
** according to modules that have been loaded thus far.
*/

int
add_to_list_of_loaded_modules(Var *v)
{
	if (loaded_modules == NULL){
		loaded_modules = Narray_create(10);
	}

	if(Narray_add(loaded_modules, v->name, v) == -1){

		/* this element may already exist */

		if (Narray_find(loaded_modules, v->name, NULL) != -1){
			parse_error("Module %s is already loaded. Unload+Reload to refresh.", v->name);
			return 0;
		}
		else {
			parse_error("Probably a mem allocation error while loading module %s.", v->name);
			return 0;
		}
	}

	if (debug > DEBUG_THRESHOLD){
		parse_error("Added module %s to the list of loaded modules.",
					v->name);
	}

	return 1; /* success */
}


Var *
search_in_list_of_loaded_modules(char *name)
{
	Var *v;

	if (loaded_modules == NULL || name == NULL) return NULL;

	if (Narray_find(loaded_modules, name, (void **)&v) != -1){
		return v;
	}

	return NULL;
}



/*
** Complimentry function of add_to_list_of_loaded_modules()
*/

void
remove_from_list_of_loaded_modules(Var *v)
{
	int i;

	if (loaded_modules == NULL){
		if (debug > DEBUG_THRESHOLD){
			parse_error("Cannot remove %s from the list of loaded modules. "
						"Reason: The list is empty.", v->name);
		}

		return;
	}

	if (v->name == NULL){
		if (debug > DEBUG_THRESHOLD){
			parse_error("Cannot remove a module with (null) name "
				"from the list of active modules.");
		}
		return;
	}

	if((i = Narray_find(loaded_modules, v->name, NULL)) != -1){
		
		/*
		** Specified module exists in the list of loaded modules.
		** Remove it!
		*/

		Narray_delete(loaded_modules, v->name);
	}
	else {
		parse_error("Cannot remove %s from the list of loaded modules. "
					"Reason: Module is not loaded.", v->name);
	}


	if (debug > DEBUG_THRESHOLD){
		parse_error("Removing %s from the list of loaded modules.",
					v->name);
	}
}



/*
** Create a daVinci module variable with the specified name.
*/

Var *
new_module(char *module_name)
{
	Var *v;

	if (module_name == NULL){
		if (debug > DEBUG_THRESHOLD){
			parse_error("Cannot create a module with (null) name.");
		}
		return NULL;
	}

	if (search_in_list_of_loaded_modules(module_name) != NULL){
		/* If module already loaded, don't load it again */
		parse_error("Module %s already loaded.", module_name);
		return NULL;
	}

	v = newVar();
	memset(&V_MODULE(v), 0, sizeof(dvModule));

	/* make scope not clean this variable */
	mem_claim(v);

	V_TYPE(v) = ID_MODULE;

	V_NAME(v) = strdup(module_name);
	if (V_NAME(v) == NULL){
		parse_error("Mem allocation error in new_module().");
		free_var(v);
		return NULL;
	}

	V_MODULE(v).functions = Narray_create(10);
    V_MODULE(v).name = V_NAME(v);

	/* keep track of available modules */
	if (!add_to_list_of_loaded_modules(v)){

		/* Addition into the list of loaded modules failed. Abort. */

		free_var(v);
		return NULL;
	}
	V_MODULE(v).stage = MOD_LST_ADDED;

	return v;
}



/*
** CAUTION!
** Verify module dependencies before calling this function.
**
** Delete the "dvModule" portion of a Var* 
** It gets called from free_var() -- which is called at various
** points in this file.
*/

void
del_module(Var *v)
{
	dvModule *m;

	if (debug > DEBUG_THRESHOLD){
		parse_error("Deleting variable %s.", v->name);
	}

	m = &V_MODULE(v);

	if (m->stage >= MOD_INIT){
		/* if module's init function was called, call the uninit function */
		uninit_dv_module(m);
	}

	if (m->stage >= MOD_LST_ADDED){
		/* remove the module from the list of loaded functions */
		if (v->name){ remove_from_list_of_loaded_modules(v); }
	}

	if (m->stage >= MOD_VAR_ADDED){
		/* remove module variable from the global symbol table */
		rm_symtab(v);
	}

	if (m->functions){ Narray_free(m->functions, free_module_function); }
	if (m->path){ free(m->path); }
	if (m->handle){ close_dv_module_file(m->handle); }
}


int
print_dependents(char *module_name)
{
	int i, n;
	int j, m;
	int ct;
	Var *v;
	char *dep_name;

	ct = 0; /* keep a count of dependents */

	if (VERBOSE){
		parse_error("Dependents of %s:", module_name);
	}

	n = Narray_count(loaded_modules);

	for (i = 0; i < n; i++){
		Narray_get(loaded_modules, i, &dep_name, (void **)&v);
		m = V_MODULE(v).init_stuff.ndep;

		for (j = 0; j < m; j++){
			if (strcmp(V_MODULE(v).init_stuff.dep[j].name, module_name) == 0){
				if (VERBOSE){
					parse_error("%s%s", ((ct==0)? "": ", "), dep_name);
				}
				ct++;
			}
		}
	}

	if (VERBOSE){
		parse_error("Total %d dependents.", ct);
	}

	return ct;
}


/*
** Implementation of ff_unload_dv_module() above.
*/
int 
unload_dv_module(
	char *module_name
)
{
	Var *v;

	/* v = get_global_sym(module_name); */
	v = search_in_list_of_loaded_modules(module_name);

	if (v){
		if (VERBOSE){
			parse_error("Unloading module %s.", module_name);
		}

		/*
		** Print module dependencies. So that the user knows
		** that unloding this module can cause problems.
		*/
		print_dependents(module_name);

		/*
		** free_var() bounces back to ff_modules.c::del_module() above
		** where uninit_dv_module() is called.
		*/
		free_var(v);
	}
	else {
		parse_error("Module %s is not loaded. So, it cannot be unloaded.",
			module_name);
	}

	return 1;
}




/*
** Search for a function within a module.
*/

vfuncptr 
find_module_func(dvModule *m, char *name)
{
	int idx;
	vfuncptr fptr;

	if (m == NULL || name == NULL) return NULL;

	if ((idx = Narray_find(m->functions, name, (void **)&fptr)) == -1){
		return NULL; /* not found! */
	}

	return fptr;
}





/***************************************************************/
/** implementation functions                                  **/
/***************************************************************/

static int
uninit_dv_module(
	dvModule *m     /* module to be uninitialized */
)
{
	dvModuleFiniFunc fini_func;

	fini_func = (dvModuleFiniFunc)locate_dv_module_func_in_slib(m->handle,
                                           DV_MODULE_FINI_FUNC_NAME);
	if (fini_func == NULL){
		if (debug > DEBUG_THRESHOLD){
			parse_error("Cannot find module uninit function %s for %s. Reason: %s.",
				DV_MODULE_FINI_FUNC_NAME, m->name, strerror(errno));
		}
	}
	else {
		(fini_func)(m->name);
	}

	return 1; /* success */
}


static void
free_module_function(vfuncptr fptr)
{
	/* free(fptr->name); */
	free(fptr);
}


/*
** Add the specified function into the name addressable
** array.
*/

static vfuncptr 
add_func_to_module(dvModule *m, vfuncptr func)
{
	vfuncptr old;
    int i;

	if (Narray_add(m->functions, func->name, func) == -1) {
		/* this element may already exist */
		if ((i = Narray_find(m->functions, func->name, NULL)) != -1){
			Narray_replace(m->functions, i, func, (void **)&old);
			if(old){ return old; }
		}
		else {
			return NULL;
		}
	}

	return func;
}

/*
** Extract module's version from its name file name.
*/

static char *
extract_dv_mod_ver_from_fname(
	const char *mod_name, 
	char *mod_file_name
)
{
	char *p;

	if (mod_file_name){
		/* skip the name and ".dvm" extension */
		p = &mod_file_name[strlen(mod_name)+1+strlen(DVM_EXT)];

		/* skip the first dot -- if there is one */
		if (*p) p++;

		return p;
	}

	return NULL;
}


#ifndef _WIN32
static struct dirent *
find_next_file_with_prefix(DIR *d, const char *pfx)
{
	struct dirent *de;
	int pfx_len = strlen(pfx);

	errno = 0; /* reset error number */
	while(de = readdir(d)){
		if (strncmp(de->d_name, pfx, pfx_len) == 0){
			return de;
		}
	}

	return NULL;
}
#endif /* _WIN32 */

static int
cmp_mod_ver(
	const char *v1,
	const char *v2
)
{
	if (v1 == NULL && v2 == NULL) return 0;
	if (v1 == NULL && v2 != NULL) return -1;
	if (v1 != NULL && v2 == NULL) return 1;
	return strcmp(v1, v2);
}


/*
** Returns the fully qualified path name of the latest module file
** if one exists. The name returned is allocated using malloc(). It
** is the caller's responsibility to free() it.
**
** Returns NULL if the name is not found or an error occurrs.
*/

static int
get_module_versions(
	const char *mod_name, /* Name of the module to be found */
	char *path,           /* A ":" separated list of dirs to find the name in  */
	char ***retlist			/* array of strings to be returned */
)
{
	char *q;
	DIR  *d;
	struct dirent *de;
	char *dir_name;
	int   strsz;
	char  name[1024];
	/* char *lasts = NULL; */
	char *fname;

	int nlist = 0;
	int retsize = 0;
	char **ret;

	ret = NULL;

	/*
	** Process each of the directories in the search path
	** to find the module with the latest version.
	*/
	sprintf(name, "%s.%s", mod_name, DVM_EXT);
	for(q = path; dir_name = strtok(q, ":"); q = NULL){
		d = opendir(dir_name);
		if (d == NULL){
			/*
			** Directory does not exist: warn and continue.
			*/

			if (debug > DEBUG_THRESHOLD){
				parse_error("Unusable dir %s in path. Reason: %s.",
					dir_name, strerror(errno));
			}
			continue;
		}

		errno = 0;  /* clear error indicator */

		while(de = find_next_file_with_prefix(d, name)){
			fname = de->d_name;
			if (nlist >= retsize) {
				if (retsize == 0) retsize = 16;
				else retsize *= 2;
				ret = (char **)realloc(ret, retsize * sizeof(char *));
			}
			strsz = strlen(dir_name) + strlen(fname) + 2;
			ret[nlist] = realloc(ret[nlist], strsz);
			sprintf(ret[nlist++], "%s/%s", dir_name, fname);
		}
	}
	closedir(d);
	*retlist = ret;
	return(nlist);
}

static char *
locate_latest_dv_module_in_path(
	const char *mod_name, /* Name of the module to be found */
	char *path            /* A ":" separated list of dirs to find the name in  */
)
{
	char *latest = NULL;
	char *vcurr = NULL, *vlatest = NULL;
	char  name[1024];
	/* char *lasts = NULL; */
	char *fname;

	int i;
	char **modules = NULL;
	int nmodules = get_module_versions(mod_name, path, &modules);

	for (i = 0 ; i < nmodules ; i++) {
		fname = modules[i];
		vcurr = extract_dv_mod_ver_from_fname(mod_name, fname);

		if (cmp_mod_ver(vcurr, vlatest) > 0) {
			if (latest) free(latest);
			latest = strdup(fname);
			vlatest = vcurr;
		}
		free(fname);
	}
	free(modules);
	return latest;
}

/*
** Returns the fully qualified path name of the latest module file
** if one exists. The name returned is allocated using malloc(). It
** is the caller's responsibility to free() it.
**
** Returns NULL if the name is not found or an error occurrs.
*/

static char *
locate_versioned_dv_module_in_path(
	const char *mod_name, /* Name of the module to be found */
	const char *ver,      /* Version of the module to be found */
	int   offset,         /* 0: match the version exactly,
						     >0: find latest version >= "ver"
					    	 <0: find latest version <= "ver"
					      */
	char *path            /* A ":" separated list of dirs to find the name in  */
)
{
	char *q;
	DIR  *d;
	struct dirent *de;
	char *latest = NULL;
	char *vcurr = NULL, *vlatest = NULL;
	char *dir_name;
	int   strsz;
	int   replace_reqd;
	char  name[1024];
	/* char *lasts = NULL; */
	char *fname;
	int i;

	char **modules = NULL;
	int nmodules = get_module_versions(mod_name, path, &modules);

	for (i = 0 ; i < nmodules ; i++) {
		fname = modules[i];
		vcurr = extract_dv_mod_ver_from_fname(mod_name, fname);

		replace_reqd = 0;
		if ((offset == 0 && cmp_mod_ver(vcurr, ver) == 0) ||
			(offset > 0 && cmp_mod_ver(vcurr, ver) >= 0 && cmp_mod_ver(vcurr, vlatest) > 0) ||
			(offset < 0 && cmp_mod_ver(vcurr, ver) <= 0 && cmp_mod_ver(vcurr, vlatest) > 0)){

			if (latest) free(latest);
			latest = strdup(fname);
			vlatest = vcurr;
			free(fname);
		}
	}
	free(modules);


	return latest;
}


/*
** Locate the specified module using the name and the
** version information provided. Use the shell variable
** DV_MOD_PATH for a list of directories to look at, if
** defined. If the variable is not set, use a default
** path.
**
** On success a path to the specified file is returned. It
** is allocated using malloc(). It is the caller's responsibility
** to free it.
**
** On error NULL is returned.
*/

static char *
locate_dv_module(
	const char *name,
	const char *ver,	/* NULL: load latest version,
						   non-NULL: load specified version based on offset
						*/
	int offset          /* if (ver != NULL)
							   0: load the specified version of module
							  >0: load latest module version s.t. version >= ver
							  <0: load latest module version s.t. version <= ver
						*/
)
{
	char *dv_mod_path = NULL;
	const char *dv_mod_path_save = NULL;
	char *loc = NULL;

	dv_mod_path_save = dv_mod_path = getenv(DV_MOD_PATH_ENV);

	/*
	** Always make a copy of the original string. We are going
	** to do some destructive stuff on it.
	** Q: since when does making a copy of a pointer to a string make
	**    a copy of the string itself?  If we're doing strtok games on
	**    dv_mod_path or its aliases down the call chain, we are playing
	**    a dangerous game indeed since getenv() returns a pointer to the
	**    actual environment!  I don't have time to fully audit this code
	**    but it looks EXTREMELY dangerous to me, offhand. -rsk 9 May 2002
    **
	** Well, those stdups below here look an awful lot like they're
	** making copies to me.  -nsg June 10, 2004.
	*/

	if (dv_mod_path == NULL){
		if (debug > DEBUG_THRESHOLD){
			parse_error("%s env is not set using default path %s.",
						DV_MOD_PATH_ENV, DV_MOD_PATH_DEF);
		}

		/* If DV_MOD_PATH environment is not set use the default path */
		dv_mod_path_save = DV_MOD_PATH_DEF;
		dv_mod_path = strdup(DV_MOD_PATH_DEF);
	}
	else {
		if (debug > DEBUG_THRESHOLD){
			parse_error("%s env found as being set to %s.",
						DV_MOD_PATH_ENV, dv_mod_path);
		}

		dv_mod_path = strdup(dv_mod_path);
	}

	if (dv_mod_path == NULL){
		parse_error("Mem allocation error.");
		return NULL;
	}

	if (ver == NULL){
		loc = locate_latest_dv_module_in_path(name, dv_mod_path);
	}
	else {
		loc = locate_versioned_dv_module_in_path(name, ver, offset, dv_mod_path);
	}

	if (loc){ parse_error("Loading module %s from %s.", name, loc); }
	else { parse_error("Module %s not found in path %s.", name, dv_mod_path_save); }

	free(dv_mod_path);

	return loc;
}


/*
** load_dv_module() calls open_dv_module() and init_dv_module()
** to complete the loading of a module.
*/

static int
load_dv_module(
	char     *mod_name,     /* name of the module being loaded < mod_fname */
	char     *mod_ver,      /* module version = NULL for "unspecified/any" */
	int       offset,       /* if(mod_ver != NULL)
								  0: load the exact specified version
								 >0: load the latest version s.t. version >= mod_ver
								 <0: load the latest version s.t. version <= mod_ver
							*/
	dvModule *mod           /* pointer to the module */
)
{
	dvModuleHandle *mh;

	/*
	** Look thru DV_MOD_PATH and locate the first occurrance
	** of the specified module.
	*/

	mod->path = locate_dv_module(mod_name, mod_ver, offset);
	if (mod->path == NULL){
		parse_error("Unable to locate module %s.", mod_name);
		return 0;
	}
	mod->stage = MOD_LOCATED;

	mod->path = strdup(mod->path);
	if (mod->path == NULL){
		parse_error("Mem allocation error, while loading %s.", mod_name);
		return 0;
	}

	mod->handle = open_dv_module_file(mod->path);
	if (mod->handle == NULL){
		parse_error("Unable to open module file %s. See previous messages.",
			mod->path);
		return 0;
	}
	mod->stage = MOD_OPENED;

    mod->name = strdup(mod_name);
	if (mod->name == NULL){
		parse_error("Mem allocation error.");

		/* next two lines are taken care of in del_module() as well --
			close_dv_module_file(mod->handle); mod->handle = NULL;
			free(mod_path); mod_path = NULL;
		*/

		return 0;
	}
    
	/*
	** Extract module version from module's name
	** mymodule.dvm.1.0 -> 1.0
	** hismodule.dvm -> (null)
	**
	** We could make the following line conditional on 
	** mod_ver != NULL.
	*/

    mod->ver = extract_dv_mod_ver_from_fname(mod_name, basename(mod->path));
	if (!mod->ver){
		parse_error("Unable to extract module version for %s. See previous messages.", mod->path);

		/* next three lines are taken care of in del_module() as well --
			close_dv_module_file(mod->handle); mod->handle = NULL;
			free(mod->name); mod->name = NULL;
			free(mod->path); mod->path = NULL;
		*/

		return 0;
	}

    
	/*
	** Initialize the module by calling its initialization function.
    ** and loading the functions exported by the module. At the
    ** end of init_dv_module(), the "mod" variable will have the
    ** "init_stuff" filled in and the functions exported by the
    ** module loaded in "mod.functions".
    */
    
	if (!init_dv_module(mod)){

		parse_error("Module initialization failed for %s. See previous messages. ",
					mod->name);

		/* next three lines are taken care of in del_module() as well --
			close_dv_module_file(mod->handle); mod->handle = NULL;
			free(mod->name); mod->name = NULL;
			free(mod->path); mod->path = NULL;
		*/

		return 0;
	}
	mod->stage = MOD_INIT;

	/* load module functions into a name addressable array */
	if (!load_dv_module_functions(mod)){
		parse_error("Loading of module functions failed for %s. ", mod->name);
		parse_error("See previous messages.");

		/* del_module() takes care of the functions loaded thus far */

		return 0;
	}
	mod->stage = MOD_FUNC_LOADED;

    /* ????????????????????????????????????????????????????????
    ** MAY BE: autoload_dep() should be moved to init_dv_module
    ** or the function loading part moved out of init_dv_module
    */

	if (!autoload_dep(mod->init_stuff.dep, mod->init_stuff.ndep)){
	    parse_error("Atleast one of the module dependencies "
			"for %s v:%s failed loading. See previous messages.",
			mod->name, printable_ver_str(mod->ver));

		/* next three lines are taken care of in del_module() as well --
			close_dv_module_file(mod->handle); mod->handle = NULL;
			free(mod->name); mod->name = NULL;
			free(mod->path); mod->path = NULL;
		*/

        /*
        ** Return whatever we have till now. Let the user take
        ** care of the freeing the memory by deleting the variable.
        */
	    return 0;
	}
	mod->stage = MOD_DEP_LOADED;

	return 1;
}

static int
autoload_dv_module(char *name, const char *ver)
{
	Var *v_return;

	if (VERBOSE){
		parse_error("autoloading %s ...", name);
	}

	/*
	** TODO:
	**
	** Create a daVinci module variable. Pass that variable
	** into load_module() to actually load the module. Check
	** the error returned by load_module() if any. On error
	** collapse the module we were trying to load.
	*/

	if (get_global_sym(name)){
		parse_error("Variable %s already exists in global space. "
			"Module load aborted.", name);
		return 0;
	}

	v_return = new_module(name);
	if (v_return){
		/*
		** Actually load the module into the daVinci variable
		** just created.
		*/
		if (!load_dv_module(V_NAME(v_return), NULL, 1, &V_MODULE(v_return))){
			free_var(v_return);
			return 0;
		}

		/* stick the named module variable into the symbol table */
		/* sym_put(global_scope(), v_return); */
		put_global_sym(v_return);
		V_MODULE(v_return).stage = MOD_VAR_ADDED;
	}

	return 1;
}

static int
dv_module_ver_matches_reqd_ver(
	const char *candidate,
	const char *reqd
)
{
	if (reqd == NULL || candidate == NULL || reqd[0] == 0 || candidate[0] == 0){
		/*
		** If we aren't looking for any specific version,
		** then this version will do as good as any other.
		*/
		return 1;
	}

	return (strcmp(reqd, candidate) == 0);
}



/*
** Load specified dependency modules. The function fails, if any
** of the specified dependencies fail to load. However, the
** dependencies loaded uptill that point are not unloaded.
*/

static int
autoload_dep(
	dvDepAttr *dep,
	int ndep
)
{
	Var *m;
    int i;

	/* autoload dependencies */
	for (i = 0; i < ndep; i++){
		if (!(m = search_in_list_of_loaded_modules(dep[i].name))){
			if (!autoload_dv_module(dep[i].name, dep[i].ver)){
				
				parse_error("Unable to load %s ver:%s.", dep[i].name,
							printable_ver_str(dep[i].ver));

				return 0;
			}
		}
		else {
			if (!dv_module_ver_matches_reqd_ver(V_MODULE(m).ver, dep[i].ver)){

				parse_error("%s v:%s already loaded, cannot load %s v:%s. ",
					m->name, printable_ver_str(V_MODULE(m).ver), dep[i].name,
					printable_ver_str(dep[i].ver));

				parse_error("Unload %s ver:%s first.",
					m->name, printable_ver_str(V_MODULE(m).ver));
				
				return 0;
			}
			else {
				if (debug > DEBUG_THRESHOLD){
					parse_error("Dependency module %s already loaded", dep[i].name);
				}
			}
		}
	}

	return 1;
}


/*
** This function is a part of the module loading operation.
** It should be performed after opening the module. It calls
** the inialization function of the module, obtains info
** about the functions exported by the module and loads this
** information in a name accessable form.
*/

static int
init_dv_module(
	dvModule *m     /* module to be initialized */
)
{
	dvModuleInitFunc init_func;

	init_func = (dvModuleInitFunc)locate_dv_module_func_in_slib(m->handle,
                                           DV_MODULE_INIT_FUNC_NAME);
	if (init_func == NULL){
		parse_error("Cannot find module init function %s for %s. Reason: %s.",
			DV_MODULE_INIT_FUNC_NAME, m->name, strerror(errno));
		return 0; /* error */
	}

	if ((init_func)(m->name, &m->init_stuff) != 1){
		parse_error("Module initialization failed for %s.", m->name);
		return 0; /* error */
	}
	
	return 1; /* success */
}



/*
** Allocates a vfunptr structure and initializes it with the
** specified values.
*/

static vfuncptr
make_vfuncptr(
	char *func_name,   /* function name */
	vfunc func_ptr,   /* function pointer */
	void *func_data    /* function data */
	)
{
	vfuncptr fptr;
	
	fptr = (vfuncptr)calloc(1, sizeof(struct _vfuncptr));
	if (fptr == NULL){
		return NULL;
	}
	
	fptr->name = func_name;
	fptr->fptr = func_ptr;
	fptr->fdata = func_data;

	return fptr;
}


/*
** This function is a part of module initialization.
**
** Loads function information into the name-addressable
** array within the module.
*/

static int
load_dv_module_functions(
	dvModule *m
)
{
	int i;
	char *func_name;
	void *func_ptr;
	vfuncptr fptr, rval;
	int ct;          /* count of functions actually loaded */
    char *mod_name = m->name;

	dvModuleFuncDesc *fdesc;
	int nfdesc;

	fdesc = m->init_stuff.fdesc;
	nfdesc = m->init_stuff.nfdesc;

	ct = 0;
	for (i = 0; i < nfdesc; i++){
		/* get the name of the next daVinci function */
		func_name = fdesc[i].name;

#ifdef _CANNOT_PASS_FUNCTION_PTRS_
		/* retrieve the function pointer from the shared library */
		func_ptr = find_dv_module_func(m->handle, func_name);

		/*
		** If the function could not be found within the library,
		** generate a warning and move on.
		*/
		if (func_ptr == NULL){
			parse_error("%s not found in %s. Reason: %s.",
				func_name, m->name, strerror(errno));
			continue;
		}
#else
		func_ptr = fdesc[i].ptr;
#endif

		/*
		** Create daVinci function description using the
		** supplied function-name, function-pointer, and
		** associate the data "h" with it.
		*/
		fptr = make_vfuncptr(func_name, (vfunc)func_ptr, NULL);
		if (fptr == NULL){
			parse_error("Mem allocation error, while loading %s.",
						mod_name);

			/*
			** This will lead to a partial module function
			** list, which should be destroyed by destroying
			** the module itself.
			*/

			return -1;
		}


		/*
		** Make the daVinci function descriptor (vfuncptr)
		** available through name lookup by way of the
		** functions Narray.
		**
		** During execution the module will only look at
		** the functions Narray to find a particular 
		** function belonging to the module.
		*/

		if (!(rval = add_func_to_module(m, fptr))){
			parse_error("Error adding %s() to the %s.",
						fptr->name, mod_name);
			free(fptr);
		}
		else {
			if (rval != fptr){
				parse_error("Duplicate definition of %s in %s.",
							fptr->name, mod_name);
				free(rval);
			}
			else {
				if (debug > DEBUG_THRESHOLD){
					parse_error("Adding function %s.%s() ...", mod_name, fptr->name);
				}

				/* update count of functions actually loaded */
				ct++;
			}
		}
	}
	
	return ct;
}



void
module_help(char *module,  char *keyword)
{
	char *path = NULL;
	char *dv_mod_path = NULL;

	/*
	This is the same logic from dv_mod_path, but I didn't want
	to try to chop off the module object file extensions
	*/

	/* If DV_MOD_PATH environment is not set use the default path */
	if ((dv_mod_path = getenv(DV_MOD_PATH_ENV)) == NULL) {
		dv_mod_path = DV_MOD_PATH_DEF;
	}
	path = malloc(strlen(dv_mod_path)+strlen(module)+6);
	sprintf(path, "%s/%s.gih", dv_mod_path, module);

	if (access(path, R_OK) != 0) {
		parse_error("Unable to open help file: %s", path);
		free(path);
		return;
	}
	do_help(keyword, path);
}
