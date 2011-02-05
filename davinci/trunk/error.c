#include <stdio.h>
#include <stdarg.h>

extern int VERBOSE;
char error_buf[16384];

void
parse_error(const char *fmt, ...)
{
	va_list ap;

	if (VERBOSE == 0) {
    return;
  } else {
    if (fmt == NULL) {
      fprintf(stderr, "error: %s\n", error_buf);
    } else {
      va_start(ap, fmt);
      (void) vfprintf(stderr, fmt, ap);
      va_end(ap);
      fprintf(stderr, "\n");
    }
  }
}

//Do a parse error but no new line at the end
void
parse_error2(const char *fmt, ...)
{
	va_list ap;

	if (VERBOSE == 0) { 
    return;
  } else {
    if (fmt == NULL) {
      fprintf(stderr, "error: %s\n", error_buf);
    } else {
      va_start(ap, fmt);
      (void) vfprintf(stderr, fmt, ap);
      va_end(ap);
    }
  }
}
