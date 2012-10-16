/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>

#include <jive/context.h>
#include <jive/vsdg/resource-private.h>

#include "testarch.h"

void test_rescls_count_addsub(jive_context * ctx)
{
	jive_resource_class_count count;
	jive_resource_class_count_init(&count, ctx);
	
	const jive_resource_class * overflow;
	
	overflow = jive_resource_class_count_add(&count, &jive_testarch_regcls[cls_r0].base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_add(&count, &jive_testarch_regcls[cls_r1].base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_add(&count, &jive_testarch_regcls[cls_r0].base);
	assert(overflow);
	
	overflow = jive_resource_class_count_add(&count, &jive_testarch_regcls[cls_evenreg].base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_add(&count, &jive_testarch_regcls[cls_r2].base);
	assert(overflow == &jive_testarch_regcls[cls_evenreg].base);
	
	overflow = jive_resource_class_count_check_change(&count, &jive_testarch_regcls[cls_evenreg].base, &jive_testarch_regcls[cls_oddreg].base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_change(&count, &jive_testarch_regcls[cls_evenreg].base, &jive_testarch_regcls[cls_r2].base);
	assert(!overflow);
	
	overflow = jive_resource_class_count_check_change(&count, &jive_testarch_regcls[cls_evenreg].base, &jive_testarch_regcls[cls_r0].base);
	assert(overflow == &jive_testarch_regcls[cls_r0].base);
	
	jive_resource_class_count_fini(&count);
}

void test_rescls_count_compound(jive_context * ctx)
{
	jive_resource_class_count a, b, c;
	jive_resource_class_count_init(&a, ctx);
	jive_resource_class_count_init(&b, ctx);
	jive_resource_class_count_init(&c, ctx);
	
	assert(jive_resource_class_count_equals(&a, &b));
	
	jive_resource_class_count_add(&a, &jive_testarch_regcls[cls_r0].base);
	jive_resource_class_count_add(&a, &jive_testarch_regcls[cls_r1].base);
	jive_resource_class_count_add(&b, &jive_testarch_regcls[cls_r0].base);
	assert(!jive_resource_class_count_equals(&a, &b));
	
	jive_resource_class_count_copy(&c, &b);
	assert(jive_resource_class_count_equals(&b, &c));
	
	jive_resource_class_count_add(&c, &jive_testarch_regcls[cls_r1].base);
	assert(jive_resource_class_count_equals(&a, &c));
	
	jive_resource_class_count_clear(&a);
	jive_resource_class_count_clear(&b);
	jive_resource_class_count_clear(&c);
	
	jive_resource_class_count_add(&a, &jive_testarch_regcls[cls_r0].base);
	jive_resource_class_count_add(&a, &jive_testarch_regcls[cls_r1].base);
	jive_resource_class_count_add(&b, &jive_testarch_regcls[cls_r1].base);
	jive_resource_class_count_add(&b, &jive_testarch_regcls[cls_r2].base);
	jive_resource_class_count_update_intersection(&a, &b);
	jive_resource_class_count_add(&c, &jive_testarch_regcls[cls_r1].base);
	jive_resource_class_count_add(&c, &jive_testarch_regcls[cls_evenreg].base);
	assert(a.nitems == 6);
	assert(jive_resource_class_count_equals(&a, &c));
	
	jive_resource_class_count_clear(&a);
	jive_resource_class_count_clear(&b);
	jive_resource_class_count_clear(&c);
	
	jive_resource_class_count_add(&a, &jive_testarch_regcls[cls_r0].base);
	jive_resource_class_count_add(&a, &jive_testarch_regcls[cls_r1].base);
	jive_resource_class_count_add(&b, &jive_testarch_regcls[cls_r1].base);
	jive_resource_class_count_add(&b, &jive_testarch_regcls[cls_r2].base);
	jive_resource_class_count_update_union(&a, &b);
	jive_resource_class_count_add(&c, &jive_testarch_regcls[cls_r0].base);
	jive_resource_class_count_add(&c, &jive_testarch_regcls[cls_r1].base);
	jive_resource_class_count_add(&c, &jive_testarch_regcls[cls_r2].base);
	jive_resource_class_count_sub(&c, &jive_testarch_regcls[cls_evenreg].base);
	assert(a.nitems == 8);
	assert(jive_resource_class_count_equals(&a, &c));
	
	jive_resource_class_count_fini(&a);
	jive_resource_class_count_fini(&b);
	jive_resource_class_count_fini(&c);
}

void test_rescls_count_prio(jive_context * ctx)
{
	jive_resource_class_count a;
	jive_resource_class_count_init(&a, ctx);
	
	jive_resource_class_count_add(&a, &jive_testarch_regcls[cls_r0].base);
	jive_resource_class_count_add(&a, &jive_testarch_regcls[cls_r1].base);
	
	jive_rescls_prio_array prio;
	jive_rescls_prio_array_compute(&prio, &a);
	jive_rescls_prio_array reference = {
		.count = { [jive_resource_class_priority_reg_low] = 2, [jive_resource_class_priority_lowest] = 2 }
	};
	assert( jive_rescls_prio_array_compare(&prio, &reference) == 0);
	
	jive_resource_class_count_sub(&a, &jive_testarch_regcls[cls_r1].base);
	jive_rescls_prio_array_compute(&prio, &a);
	assert( jive_rescls_prio_array_compare(&prio, &reference) == -1);
	
	jive_resource_class_count_add(&a, &jive_testarch_regcls[cls_cc].base);
	jive_rescls_prio_array_compute(&prio, &a);
	assert( jive_rescls_prio_array_compare(&prio, &reference) == +1);
	
	jive_resource_class_count_fini(&a);
}

static int test_main(void)
{
	jive_context * ctx = jive_context_create();
	
	test_rescls_count_addsub(ctx);
	test_rescls_count_compound(ctx);
	test_rescls_count_prio(ctx);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-rescls-count", test_main);
