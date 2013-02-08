/*
 * Copyright 2013 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "test-registry.h"

#include <jive/util/sort.h>

static inline bool
int_compare(int a, int b) {
	return a < b;
}

DEFINE_SORT(int_lt, int, int_compare);

static void
sort_and_verify(
	void (*sort_fn)(int*, size_t),
	int * vec, size_t size)
{
	sort_fn(vec, size);
	size_t n;
	for (n = 1; n < size; ++n) {
		assert(vec[n - 1] <= vec[n]);
	}
}

static void
test_sort(
	void (*sort_fn)(int*, size_t),
	size_t limit)
{
	int vec[limit];
	size_t n = 0;
	for (n = 0; n < limit; ++n)
		vec[n] = n;
	sort_and_verify(int_lt_heapsort, vec, limit);
	
	for (n = 0; n < limit; ++n)
		vec[n] = limit - n;
	sort_and_verify(int_lt_heapsort, vec, limit);
	
	for (n = 0; n < limit; ++n)
		vec[n] = (n&1 << 12) | (n&2 << 5) | (n&4 << 7) | (n&8) |
			 (n&16 << 7) | (n&32 << 3) | (n&64 >> 1) | (n&128 >> 5) |
			 (n&256 << 2) | (n&512 >> 2) | (n&1024 >> 6) | (n&2048 >> 10);
	sort_and_verify(sort_fn, vec, limit);
}

static int
test_main(void)
{
	static const size_t sizes[8] = {0, 1, 2, 4, 8, 16, 1024, 4096};
	size_t n;
	for (n = 0; n < 8; ++n) {
		size_t size = sizes[n];
		test_sort(int_lt_heapsort, size);
		if (size <= 1024)
			test_sort(int_lt_bubblesort, size);
		test_sort(int_lt_sort, size);
	}
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("util/test-sort", test_main);
