/*
 * Copyright 2010 2011 2012 2013 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/vsdg/resource.h>

#include "testarch.h"

void test_rescls_count_addsub()
{
	jive_resource_class_count count;
	
	auto overflow = count.add(&jive_testarch_regcls_r0);
	assert(!overflow);
	
	overflow = count.check_add(&jive_testarch_regcls_r1);
	assert(!overflow);
	
	overflow = count.check_add(&jive_testarch_regcls_r0);
	assert(overflow);
	
	overflow = count.add(&jive_testarch_regcls_evenreg);
	assert(!overflow);
	
	overflow = count.check_add(&jive_testarch_regcls_r2);
	assert(overflow == &jive_testarch_regcls_evenreg);
	
	overflow = count.check_change(&jive_testarch_regcls_evenreg, &jive_testarch_regcls_oddreg);
	assert(!overflow);
	
	overflow = count.check_change(&jive_testarch_regcls_evenreg, &jive_testarch_regcls_r2);
	assert(!overflow);
	
	overflow = count.check_change(&jive_testarch_regcls_evenreg, &jive_testarch_regcls_r0);
	assert(overflow == &jive_testarch_regcls_r0);
}

void test_rescls_count_compound()
{
	jive_resource_class_count a, b, c;
	
	assert(a == b);
	
	a.add(&jive_testarch_regcls_r0);
	a.add(&jive_testarch_regcls_r1);
	b.add(&jive_testarch_regcls_r0);
	assert(a != b);
	
	c = b;
	assert(b == c);
	
	c.add(&jive_testarch_regcls_r1);
	assert(a == c);
	
	a.clear();
	b.clear();
	c.clear();
	
	a.add(&jive_testarch_regcls_r0);
	a.add(&jive_testarch_regcls_r1);
	b.add(&jive_testarch_regcls_r1);
	b.add(&jive_testarch_regcls_r2);
	a.update_intersection(b);
	c.add(&jive_testarch_regcls_r1);
	c.add(&jive_testarch_regcls_evenreg);
	assert(a.counts().size()== 6);
	assert(a == c);
	
	a.clear();
	b.clear();
	c.clear();
	
	a.add(&jive_testarch_regcls_r0);
	a.add(&jive_testarch_regcls_r1);
	b.add(&jive_testarch_regcls_r1);
	b.add(&jive_testarch_regcls_r2);
	a.update_union(b);
	c.add(&jive_testarch_regcls_r0);
	c.add(&jive_testarch_regcls_r1);
	c.add(&jive_testarch_regcls_r2);
	c.sub(&jive_testarch_regcls_evenreg);
	assert(a.counts().size() == 8);
	assert(a == c);
}

void test_rescls_count_prio()
{
	jive_resource_class_count a;
	
	a.add(&jive_testarch_regcls_r0);
	a.add(&jive_testarch_regcls_r1);
	
	jive_rescls_prio_array prio;
	jive_rescls_prio_array_compute(&prio, &a);
	jive_rescls_prio_array reference = {};
	reference.count[jive_resource_class_priority_reg_low] = 2;
	reference.count[jive_resource_class_priority_lowest] = 2;
	assert( jive_rescls_prio_array_compare(&prio, &reference) == 0);
	
	a.sub(&jive_testarch_regcls_r1);
	jive_rescls_prio_array_compute(&prio, &a);
	assert( jive_rescls_prio_array_compare(&prio, &reference) == -1);
	
	a.add(&jive_testarch_regcls_cc);
	jive_rescls_prio_array_compute(&prio, &a);
	assert( jive_rescls_prio_array_compare(&prio, &reference) == +1);
}

static int test_main(void)
{
	test_rescls_count_addsub();
	test_rescls_count_compound();
	test_rescls_count_prio();
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-rescls-count", test_main);
