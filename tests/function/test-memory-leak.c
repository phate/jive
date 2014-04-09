/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/context.h>
#include <jive/types/function/fcttype.h>
#include <jive/vsdg/valuetype.h>

static int test_main(void)
{
	jive_context * context = jive_context_create();
	
	JIVE_DECLARE_VALUE_TYPE(value_type);
	
	jive_function_type t1, t2;
	const jive_type * tmparray0[] = {value_type};
	const jive_type * tmparray1[] = {value_type};
	jive_function_type_init(&t1, context,
		1, tmparray0,
		1, tmparray1);
	const jive_type * tmparray2[] = {&t1};
	const jive_type * tmparray3[] = {&t1};
	
	jive_function_type_init(&t2, context,
		1, tmparray2,
		1, tmparray3);
	
	jive_function_type_fini(&t1);
	jive_function_type_fini(&t2);
	
	assert(jive_context_is_empty(context));
	
	jive_context_destroy(context);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("function/test-memory-leak", test_main);
