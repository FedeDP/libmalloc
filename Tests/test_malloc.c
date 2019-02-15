#include "test_malloc.h"
#include "malloc/alloc.h"
#include <unistd.h>
#include <string.h>

static void *ptr;

void malloc_test(void **state) {	
	ptr = malloc_f(0);
	assert_null(ptr);
	
	ptr = malloc_f(1024);
	assert_non_null(ptr);
	
	free_f(ptr);
}

void calloc_test(void **state) {
	ptr = calloc_f(1, 0);
	assert_null(ptr);
	
	ptr = calloc_f(0, 1);
	assert_null(ptr);
	
	ptr = calloc_f(1, 1024);
	assert_non_null(ptr);
	assert_true(*(unsigned char *)ptr == 0);
}

void realloc_test(void **state) {
	void *tmp = realloc_f(ptr, 5);
	/* Shrinking memory is done in place */
	assert_ptr_equal(tmp, ptr);
	
	/* Same as free_f */
	tmp = realloc_f(ptr, 0);
}
