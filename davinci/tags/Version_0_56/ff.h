#include "parser.h"


/**
 ** The list of named functions, their handlers and an optional data value
 **/

struct _vfuncptr vfunclist[] = {
    { "nop",        ff_nop,     NULL },
    { "exit",       ff_exit,    NULL },
    { "echo",       ff_echo,    NULL },

    /*
     * By using the third argument to hold a pointer to an actual library
     * function, we can reuse ff_dfunc() to implement ANY function that 
	 * expects a single double argument, and returns a single double 
	 * argument (like all the simple math functions).
	 *
     * Presumably, we could extend this idea to handle all integer and
     * string function (ff_ifunc and ff_sfunc), but they are of less use.
     */

/* math functions  */
    { "cos",        ff_dfunc,    (void *)cos},
    { "sin",        ff_dfunc,    (void *)sin},
    { "tan",        ff_dfunc,    (void *)tan},
    { "acos",       ff_dfunc,    (void *)acos},
    { "asin",       ff_dfunc,    (void *)asin},
    { "atan",       ff_dfunc,    (void *)atan},
    { "sqrt",       ff_dfunc,    (void *)sqrt},
    { "log10",      ff_dfunc,    (void *)log10},
    { "log",        ff_dfunc,    (void *)log},
    { "ln",         ff_dfunc,    (void *)log},
    { "exp",        ff_dfunc,    (void *)exp},
    { "floor",      ff_dfunc,    (void *)floor},
    { "ceil",       ff_dfunc,    (void *)ceil},
    { "abs",        ff_dfunc,    (void *)fabs},

/* new math functions */

    { "pow",        ff_pow,  (void *)NULL},
    
/* format conversion routines.  All just shorthand for ff_conv() */
    
    { "char",       ff_conv, (void *)BYTE},
    { "byte",       ff_conv, (void *)BYTE},
    { "short",      ff_conv, (void *)SHORT},
    { "int",        ff_conv, (void *)INT},
    { "float",      ff_conv, (void *)FLOAT},
    { "double",     ff_conv, (void *)DOUBLE},
    
/* org conversion.  All just shorthand for ff_org() */

    { "bil",        ff_org, (void *)(BIL+10)},
    { "bsq",        ff_org, (void *)(BSQ+10)},
    { "bip",        ff_org, (void *)(BIP+10)},
    
/* some generic information routines */

    { "dim",        ff_dim,     NULL },
    { "format",     ff_format,  NULL },
    { "type",       ff_format,  NULL },
    { "org",        ff_org,     NULL }, 
    { "order",      ff_org,     NULL }, 
    { "create",     ff_create,  NULL },
    
/* i/o */

    { "source",     ff_source,   NULL },
    { "load",       ff_load,     NULL },
    { "read",       ff_load,     NULL },        /* an alias */
    { "import",     ff_load,     NULL },        /* an alias */
	{ "load_ir",   ff_Frame_Grabber_Read, NULL},
	{ "load_vis", ff_GSE_VIS_Read,    NULL},
	{ "load_paci",ff_PACI_Read,NULL},

    { "save",       ff_write,    NULL },
    { "write",      ff_write,    NULL },        /* an alias */
    { "export",     ff_write,    NULL },        /* an alias */
    { "filetype",   ff_filetype, NULL },
    { "ls",         ff_list,     NULL },
    { "list",       ff_list,     NULL },
    
/* string functions */

    { "atoi",       ff_atoi,     NULL },
    { "atof",       ff_atof,     NULL },
    { "string",     ff_string,   NULL },
    { "delim",      ff_delim,    NULL },
    { "issubstring",ff_issubstring,	NULL},
    { "strlen",	    ff_strlen,	NULL},
    { "strstr",	    ff_strstr,	NULL},
	 { "grep",		ff_grep,		NULL},
    
    { "rgb",        ff_rgb,      NULL },
    
/* speccial function */

    { "gnoise",     ff_gnoise,   NULL },
    { "cluster",    ff_cluster,  NULL },
    { "ccount",     ff_ccount,   NULL },

    { "rnoise",     ff_random,   (void *)"rnoise" },
    { "rand",       ff_random,   (void *)"rand" },
    { "random",     ff_random,   NULL },
    
    { "clone",      ff_replicate, NULL },
    { "cat",        ff_cat,       NULL },
    
    { "version",    ff_version,   NULL },
    { "translate",  ff_translate, NULL },
    { "display",    ff_display,   NULL },
    
        /* Could implement avg, stddev, etc, by making ff_moment recognize 
           the third argument and only return the expected values */
    { "moment",     ff_moment,	NULL },   
    { "avg",     ff_avg,    	NULL },   
    { "stddev",  ff_avg,    	NULL },   
    { "sum",     ff_avg,    	NULL },   
    { "min",     ff_min,    	NULL },   
    { "max",     ff_min,    	NULL },   
    { "sort",     ff_sort,      NULL },
    { "minchan",  ff_findmin,   NULL },   
    { "maxchan",  ff_findmin,   NULL },   

    { "interp",     ff_interp,    NULL },
    { "gplot",      ff_gplot,     NULL },
    { "plot",       ff_plot,      NULL },
    { "splot",      ff_splot,     NULL },

    { "fit",        ff_fit,      NULL },
    { "hasvalue",   HasValue,    NULL },
    { "HasValue",   HasValue,    NULL },
    { "edit",       ufunc_edit,  NULL },
    { "header",     ff_header,   NULL },	/* read stuff from file headers */
    { "ascii",      ff_ascii,    NULL },	
    { "read_ascii", ff_ascii,    NULL },
    { "read_text",  ff_text,     NULL },
	 { "read_lines",ff_textarray,NULL},

#ifndef __MSDOS__
    {"popen",	ff_popen,	NULL},
    {"pprint",	ff_pprint,	NULL},
    {"pplot",	ff_pplot,	NULL},
    {"ptext",	ff_ptext,	NULL},
    {"pline",	ff_pline,	NULL},
    {"pbox",	ff_pbox,	NULL},
    {"pzoom",	ff_pzoom,	NULL},
#endif
    
#if 0
    { "bbr",        ff_bbr,      NULL },    /* blackbody radiance   */
    { "bbrf",       ff_bbrf,     NULL },    /* blackbody radiance[] */
    { "btemp",      ff_btemp,    NULL },	/* added by kaq */
#endif

    { "bbr",        ff_bop,      (void *)bbr   },
    { "btemp",      ff_bop,      (void *)btemp },
    { "atan2",      ff_bop,      (void *)atan2 },

    { "vignette",   ff_vignette, NULL },
    { "pause",      ff_pause,    NULL },	/* get input from user */
    { "ifill",      ff_ifill,    NULL },	/* interpolated fill */
    { "jfjll",      ff_jfill,    NULL },	/* jnterpolated fjll */

    { "printf",     ff_printf,    NULL },
    { "sprintf",    ff_sprintf,   NULL },
    { "fprintf",    ff_fprintf,   NULL },
    { "system",     ff_system,    NULL },
    { "fsize",      ff_fsize,    NULL },
    { "history",    ff_history,    NULL },
    { "h",     		ff_history,    NULL },

/* add additional functions here.  Don't forget the trailing comma. */

    { "basis",     	ff_basis,  			NULL },
    { "mxm",     	ff_mxm,    			NULL },
    { "histogram",	ff_histogram,  		NULL },
    { "rgb2hsv",    ff_rgb2hsv,    		NULL },
    { "hsv2rgb",    ff_hsv2rgb,    		NULL },

    { "isis",	ff_read_suffix_plane, NULL},
    { "resize",     ff_resize,    		NULL },
    { "fork",     	ff_fork,    		NULL },

    { "minvert",     	ff_minvert,     NULL },
    { "dct",     	ff_dct,     NULL },
    { "entropy",     	ff_entropy,     NULL },
#ifdef HAVE_LIBPROJ   
    { "projection",	ff_projection,   NULL },
#endif

#ifdef HAVE_LIBMAGICK 
    { "XIdisplay",	ff_XImage_Display, NULL},
#endif

    { "fft",     	ff_fft,     (void *)1 },
    { "ifft",     	ff_fft,     (void *)0 },
    { "rfft",     	ff_realfft,     (void *)1 },
    { "irfft",     	ff_realfft,     (void *)0 },
    { "rfft2",     	ff_realfft2,     (void *)1 },
    { "irfft2",     ff_realfft2,     (void *)0 },
    { "rfft3",     	ff_realfft3,     (void *)1 },
    { "irfft3",     ff_realfft3,     (void *)0 },

    { "self_convolve",     ff_self_convolve,    NULL },
    { "convolve",      ff_convolve,     		NULL },
    { "convolve3",     ff_convolve3,     		NULL },

#ifdef HAVE_XM_XR3DT_H
    { "xrt3d",     	ff_xrt3d,    NULL },
#endif

    { "struct",         ff_struct,         NULL },
    { "eval",           ff_eval,           NULL },
    { "add_struct",     ff_add_struct,     NULL },
    { "get_struct",     ff_get_struct,     NULL },
	{ "syscall",        ff_syscall,        NULL},
    { "basename",       ff_filename,       (void *)1},
    { "dirname",        ff_filename,       (void *)2},
    { "strsub",         ff_stringsubst,    NULL},
	{ "dump",           ff_dump,           NULL},
	{ "global",         ff_global,         NULL},
	{ "delete",         ff_delete,         NULL},

    { NULL,         NULL,        NULL }
};

