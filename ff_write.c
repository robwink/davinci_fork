#include "dvio.h"
#include "io_loadmod.h"
#include "parser.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* FIX: put these in a header */

#ifdef HAVE_LIBPNG
static char const* iom_filetypes[] = {"gif", "jpg", "jpeg", "tif", "tiff", "png", "bmp", 0};
#else
static char const* iom_filetypes[] = {"gif", "jpg", "jpeg", "tif", "tiff", "bmp", 0};
#endif

Var* ff_write(vfuncptr func, Var* arg)
{

	Var* ob         = NULL;
	char* filename  = NULL;
	char* title     = NULL;
	char* type      = NULL;
	char* separator = NULL; /* for csv */
	int header      = 0;    /* for csv */
	int force       = 0;    /* Force file overwrite */
	int hdf_old     = 0;    // write hdf file backward like davinci used to
	unsigned short iom_type_idx, iom_type_found;

	Alist alist[9];
	alist[0]      = make_alist("object", ID_UNK, NULL, &ob);
	alist[1]      = make_alist("filename", ID_STRING, NULL, &filename);
	alist[2]      = make_alist("type", ID_ENUM, NULL, &type);
	alist[3]      = make_alist("title", ID_STRING, NULL, &title);
	alist[4]      = make_alist("force", DV_INT32, NULL, &force);
	alist[5]      = make_alist("separator", ID_STRING, NULL, &separator);
	alist[6]      = make_alist("header", DV_INT32, NULL, &header);
	alist[7]      = make_alist("hdf_old", DV_INT32, NULL, &hdf_old);
	alist[8].name = NULL;

	if (parse_args(func, arg, alist) == 0) return (NULL);

	/**
	** Make sure user specified an object
	**/
	if (ob == NULL) {
		parse_error("%s: No object specified.", func->name);
		return (NULL);
	}

	/**
	** get filename.  Verify type
	**/
	if (filename == NULL) {
		parse_error("No filename specified.");
		return (NULL);
	}
	filename = dv_locate_file(filename);

	if (type == NULL) {
		parse_error("No type specified.");
		return (NULL);
	}

	/*
	** Get title string
	*/
	if (title == NULL) {
		title = (char*)"DV data product";
	}

	/* Check type against list of types supported by iomedley. */

	iom_type_idx = iom_type_found = 0;

	while (iom_filetypes[iom_type_idx]) {
		if (!strcasecmp(type, iom_filetypes[iom_type_idx])) {
			iom_type_found = 1;
			break;
		}
		iom_type_idx++;
	}

	if (iom_type_found)
		dv_WriteIOM(ob, filename, type, force);
	else if (!strcasecmp(type, "raw"))
		dv_WriteRaw(ob, filename, force);
	else if (!strcasecmp(type, "vicar"))
		dv_WriteVicar(ob, filename, force);
	else if (!strcasecmp(type, "grd"))
		dv_WriteGRD(ob, filename, force, title, (char*)"davinci");
	/*    else if (!strcasecmp(type, "pnm"))    dv_WritePNM(ob, filename, force); */
	else if (!strcasecmp(type, "pgm"))
		dv_WritePGM(ob, filename, force);
	else if (!strcasecmp(type, "ppm"))
		dv_WritePPM(ob, filename, force);
	else if (!strcasecmp(type, "ascii"))
		WriteAscii(ob, filename, force);
	else if (!strcasecmp(type, "csv"))
		dv_WriteCSV(ob, filename, separator, header, force);
	else if (!strcasecmp(type, "ers"))
		dv_WriteERS(ob, filename, force);
	else if (!strcasecmp(type, "imath"))
		dv_WriteIMath(ob, filename, force);
	else if (!strcasecmp(type, "isis"))
		dv_WriteISIS(ob, filename, force, title);
	else if (!strcasecmp(type, "envi"))
		dv_WriteENVI(ob, filename, force);
	else if (!strcasecmp(type, "specpr")) {
		if (!force && file_exists(filename)) {
			parse_error("File %s already exists.\n", filename);
			return NULL;
		}
		WriteSpecpr(ob, filename, title);
	}

/*
** Below here are optional packages
*/
#ifdef HAVE_LIBHDF5
	else if (!strcasecmp(type, "hdf")) {
		struct stat statbuf;
		if (!force && !stat(filename, &statbuf)) {
			parse_error("File %s already exists.\n", filename);
			return NULL;
		}

		/* force ? */
		WriteHDF5(-1, filename, ob, hdf_old);
	}
#endif

#ifdef BUILD_MODULE_SUPPORT
	else if (iomod_handler_for_type(type))
		write_to_io_module(ob, filename, type, force);
#endif

#if 0
#ifdef HAVE_LIBMAGICK
	// else if (dvio_ValidGfx(type, GFX_type))    dv_WriteGFX_Image(ob, filename, force, GFX_type);
	else if (1) paramdvWriteImage(ob, filename, type, force);
#endif
#endif
	else {
		sprintf(error_buf, "Unrecognized type: %s", type);
		parse_error(NULL);
		return (NULL);
	}

	free(filename);

	return (NULL);
}
