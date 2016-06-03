#include "func.h"
#include "version.h"
#include <errno.h>

Var* ff_version_str(vfuncptr func, Var* arg)
{
	Var* s;
	s           = newVar();
	V_TYPE(s)   = ID_STRING;
	V_STRING(s) = strdup(version + 5);
	return s;
}

Var* ff_version(vfuncptr func, Var* arg)
{
	float* version_num = malloc(sizeof(float));
	if (!version_num) {
		memory_error(errno, sizeof(float));
		return NULL;
	}
	*version_num = atof(version+13);

	return newVal(BSQ, 1, 1, 1, FLOAT, version_num);
}

void dump_version()
{
	printf("%s: (%s) %s\n", version + 5, builder, build);
#include "build_summary"
}
