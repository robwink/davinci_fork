#include "parser.h"
#include "version.h"

Var *
ff_version(vfuncptr func, Var *arg)
{
	Var *s;
	extern char *version;

	s = newVar();
	V_TYPE(s) = ID_STRING;
	V_STRING(s) = strdup(version+5);
	return(s);
}

void
dump_version()
{
	printf("%s: (%s) %s\n", version+5, builder, build);
	printf("Options: ");

#ifdef INCLUDE_API
		printf("%s ", "plplot");
#endif
#ifdef HAVE_XRT 
		printf("%s ", "XRT");
#endif
#ifdef HAVE_LIBPROJ 
		printf("%s ", "libproj");
#endif
#ifdef HAVE_LIBMAGICK
		printf("%s ", "libmagick");
#endif
#ifdef HAVE_LIBX11
		printf("%s ", "libx11");
#endif
#ifdef HAVE_LIBXM
		printf("%s ", "libxm");
#endif
#ifdef HAVE_LIBXT
		printf("%s ", "libxt");
#endif
#ifdef HAVE_LIBHDF5
		printf("%s ", "libhdf5");
#endif
#ifdef HAVE_LIBJPEG
		printf("%s ", "libjpeg");
#endif
#ifdef HAVE_LIBREADLINE
		printf("%s ", "libreadline");
#endif
#ifdef HAVE_LIBTIFF
		printf("%s ", "libtiff");
#endif
#ifdef HAVE_LIBUSDS
		printf("%s ", "libusds");
#endif
#ifdef HAVE_LIBZ
		printf("%s ", "libz");
#endif
	printf("\n");
}
