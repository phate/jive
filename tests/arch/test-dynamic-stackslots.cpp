/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.hpp"

#include <assert.h>

#include <jive/arch/stackslot.hpp>

static int test_main(void)
{
	auto cls1 = jive_stackslot_size_class_get(1, 1);
	assert(cls1 == &jive_stackslot_class_1_1);
	
	cls1 = jive_stackslot_size_class_get(2, 2);
	assert(cls1 == &jive_stackslot_class_2_2);
	
	cls1 = jive_stackslot_size_class_get(4, 4);
	assert(cls1 == &jive_stackslot_class_4_4);
	
	cls1 = jive_stackslot_size_class_get(8, 8);
	assert(cls1 == &jive_stackslot_class_8_8);
	
	cls1 = jive_stackslot_size_class_get(8, 4);
	assert(cls1);
	auto cls2 = jive_stackslot_size_class_get(8, 4);
	assert(cls1 == cls2);
	
	cls1 = jive_fixed_stackslot_class_get(4, 4, 0);
	assert(cls1->parent() == &jive_stackslot_class_4_4);
	assert(cls1->nresources() == 1);
	
	cls2 = jive_fixed_stackslot_class_get(4, 4, 0);
	assert(cls1 == cls2);
	
	cls2 = jive_fixed_stackslot_class_get(4, 4, 4);
	assert(cls2->parent() == &jive_stackslot_class_4_4);

	cls2 = jive_fixed_stackslot_class_get(4, 4, -4);
	assert(cls2->parent() == &jive_stackslot_class_4_4);
	
	cls2 = jive_fixed_stackslot_class_get(4, 2, 2);
	assert(cls2->parent() != &jive_stackslot_class_4_4);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("arch/test-dynamic-stackslots", test_main)
