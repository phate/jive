/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>

#include <jive/types/function/fcttype.h>
#include <jive/vsdg/valuetype.h>

static int test_main(void)
{
	jive_test_value_type value_type;
	const jive_type * value_type_ptr = &value_type;
	jive::fct::type t1(1, &value_type_ptr, 1, &value_type_ptr);

	const jive_type * tmparray2[] = {&t1};
	const jive_type * tmparray3[] = {&t1};
	jive::fct::type t2(1, tmparray2, 1, tmparray3);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-memory-leak", test_main);
