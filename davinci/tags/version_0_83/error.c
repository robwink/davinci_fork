#include <stdio.h>
#include <stdarg.h>

extern int VERBOSE;
char error_buf[256];

void
parse_error(char *fmt, ...)
{
	va_list ap;

	if (VERBOSE == 0) return;

	if (fmt == NULL) {
		fprintf(stderr, "error: %s\n", error_buf);
	} else {
		va_start(ap, fmt);
		(void) vfprintf(stderr, fmt, ap);
		va_end(ap);
		fprintf(stderr, "\n");
	}
}
