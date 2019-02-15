#include "test_malloc.h"

int main(void) {
	const struct CMUnitTest tests[] = {
// 		/* Test module_register failures */
		cmocka_unit_test(malloc_test),
		cmocka_unit_test(calloc_test),
		cmocka_unit_test(realloc_test)
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

