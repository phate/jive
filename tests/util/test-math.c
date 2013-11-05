/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/util/math.h>

#include <assert.h>

static void
test_gcd()
{
	assert(jive_gcd(1, 0) == 1);
	assert(jive_gcd(0, 1) == 1);
	assert(jive_gcd(2, 4) == 2);
	assert(jive_gcd(9, 6) == 3);
}

static int
test_main(void)
{
	assert(jive_max_unsigned(1, 2) == 2);
	assert(jive_max_signed(-1, -2) == -1);
	assert(jive_min_unsigned(1, 2) == 1);
	assert(jive_min_signed(-1, -2) == -2);

	test_gcd();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("util/test-math", test_main);
