#ifndef __UTIL_H__
#define __UTIL_H__

#define INIT(type, ptr, reserved_size) \
	do { \
		ptr = malloc(sizeof(type) * reserved_size); \
	}while(false)

#define APPEND(type, ptr, reserved_size, size, x) \
	do { \
		if (size >= reserved_size) { \
			type *buf = malloc(sizeof(type) * reserved_size * 2); \
			for (int i = 0; i < reserved_size; ++i) { \
				buf[i] = ptr[i];\
			} \
			reserved_size *= 2; \
			free(ptr); \
		}\
		ptr[size++] = x; \
	}while(false)

char* format(const char *fmt, ...);
char* deep_copy(const char* str);
#endif
