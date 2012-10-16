/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/util/rangemap.h>

typedef struct int_map int_map;
JIVE_DECLARE_RANGEMAP_TYPE(int_map, int, -17);
JIVE_DEFINE_RANGEMAP_TYPE(int_map, int, -17);

static void
test_int_map(void)
{
	int_map a;
	int_map_init(&a);
	
	int * x = int_map_lookup(&a, 42);
	assert(a.low <= 42 && a.high > 42);
	assert(*x == -17);
	
	*x = 0;
	
	int * y = int_map_lookup(&a, -5);
	assert(a.low <= -5 && a.high > 42);
	assert(*y == -17);
	
	int * z = int_map_lookup(&a, 42);
	assert(*z == 0);
	
	int_map_fini(&a);
}

static int test_main(void)
{
	test_int_map();
	return 0;
}

JIVE_UNIT_TEST_REGISTER("util/test-rangemap", test_main);
