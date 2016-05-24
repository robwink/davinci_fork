#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern int VERBOSE;
char error_buf[16384];

void parse_error(const char* fmt, ...)
{
	va_list ap;

	if (VERBOSE == 0) {
		return;
	} else {
		if (fmt == NULL) {
			fprintf(stderr, "error: %s\n", error_buf);
		} else {
			va_start(ap, fmt);
			(void)vfprintf(stderr, fmt, ap);
			va_end(ap);
			fprintf(stderr, "\n");
		}
	}
}

// Do a parse error but no new line at the end
void parse_error2(const char* fmt, ...)
{
	va_list ap;

	if (VERBOSE == 0) {
		return;
	} else {
		if (fmt == NULL) {
			fprintf(stderr, "error: %s\n", error_buf);
		} else {
			va_start(ap, fmt);
			(void)vfprintf(stderr, fmt, ap);
			va_end(ap);
		}
	}
}

// call this instead of parse_error directly for memory errors, so
// we'll get uniform memory error output and since memory errors aren't actually
// parse_errors, but runtime errors
void memory_error(int error_num, size_t mem_size)
{
	char err_str[128];
	snprintf(err_str, sizeof(err_str), "Error allocating memory, %zu: ", mem_size);
	parse_error("%s: %s", err_str, strerror(error_num));

	return;
}

