#include <unistd.h>
#include <sys/times.h>
#include "parser.h"


/* File for miscellaneous davinci "meta-functions" that give
   information about davinci and its environment. */

Var *
ff_audit(vfuncptr func, Var *arg)
 {
	struct tms faketime;
	clock_t realtime;
	void *ptr;
	Var *s;
	int *s_data;

	realtime = times(&faketime);
	ptr = sbrk(0);

	s = newVar();
    V_TYPE(s) = ID_VAL;

    V_DATA(s) = calloc(4, sizeof(int));
    V_FORMAT(s) = INT;
    V_ORDER(s) = BSQ;
    V_DSIZE(s) = 4;

    V_SIZE(s)[0] = 4;
    V_SIZE(s)[1] = 1;
    V_SIZE(s)[2] = 1;

	s_data = (int *) V_DATA(s);

	s_data[0] = (int)ptr;
	s_data[1] = faketime.tms_utime;
	s_data[2] = faketime.tms_stime;
	s_data[3] = realtime;

	return  s;
 }
