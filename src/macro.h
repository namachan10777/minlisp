#define RESERVE(type, ptr, reserved_size) \
	do { \
		type *buf = malloc(sizeof(type) * reserved_size * 2); \
		for (size_t i = 0; i < reserved_size; ++i) { \
			buf[i] = ptr[i];\
		} \
		reserved_size *= 2; \
		free(ptr); \
	}while(false)
