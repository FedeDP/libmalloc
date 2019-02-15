#include <stddef.h>

#define _public_ __attribute__ ((visibility("default")))

_public_ void *malloc_f(size_t size);
_public_ void *calloc_f(int num, size_t size);
_public_ void free_f(void *ptr);
_public_ void *realloc_f(void *ptr, size_t size);
