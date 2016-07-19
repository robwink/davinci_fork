#ifdef _WIN32
#include <time.h>
#else
#include <sys/times.h>
#include <unistd.h>
#endif /* _WIN32 */
#include "parser.h"

/* File for miscellaneous davinci "meta-functions" that give
   information about davinci and its environment. */

Var* ff_audit(vfuncptr func, Var* arg)
{

#ifdef _WIN32
	parse_error("This function is not supported under windows.");
	return NULL;
#else
	struct tms faketime;
	clock_t realtime;
	void* ptr;
	Var* s;
	int* s_data;

	realtime = times(&faketime);
	ptr      = sbrk(0);

	s_data = calloc(4, sizeof(int));
	s      = newVal(BSQ, 4, 1, 1, DV_INT32, s_data);

	s_data[0] = (long)ptr;
	s_data[1] = faketime.tms_utime;
	s_data[2] = faketime.tms_stime;
	s_data[3] = realtime;

	return s;
#endif /* _WIN32 */
}
