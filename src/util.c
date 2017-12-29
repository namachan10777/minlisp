#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

char* format(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	size_t len = strlen(fmt) + 1;
	char* buf = malloc(sizeof(char) * len);
	sprintf(buf, fmt, ap);
	va_end(ap);
	return buf;
}

char* deep_copy(const char *str) {
	char* buf = malloc(sizeof(char) * strlen(str));
	strcpy(buf, str);
	return buf;
}
