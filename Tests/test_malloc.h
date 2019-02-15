#pragma once

/* CMocka */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void malloc_test(void **state);
void calloc_test(void **state);
void realloc_test(void **state);

