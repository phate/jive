/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/context.h>
#include <jive/util/set.h>

#include <assert.h>

JIVE_DECLARE_SET_TYPE(my_set, void);
JIVE_DEFINE_SET_TYPE(my_set, void);

static int test_main(void)
{
	jive_context * context = jive_context_create();

	struct my_set set;
	my_set_init(&set, context);
	assert(!my_set_contains(&set, (void *)1));

	my_set_insert(&set, (void *)1);
	assert(my_set_contains(&set, (void *)1));

	my_set_remove(&set, (void *)1);
	assert(!my_set_contains(&set, (void *)1));
	assert(my_set_size(&set) == 0);

	my_set_insert(&set, (void *)1);
	my_set_insert(&set, (void *)2);
	my_set_insert(&set, (void *)3);
	assert(my_set_contains(&set, (void *)3));
	my_set_insert(&set, (void *)3);

	assert(my_set_size(&set) == 3);

	struct my_set_iterator iter;
	bool seen_i1, seen_i2, seen_i3;
	seen_i1 = seen_i2 = seen_i3 = false;
	JIVE_SET_ITERATE(my_set, set, iter) {
		if (iter.entry->item == (void *)1)
			seen_i1 = true;
		if (iter.entry->item == (void *)2)
			seen_i2 = true;
		if (iter.entry->item == (void *)3)
			seen_i3 = true;
	}
	assert(seen_i1 && seen_i2 && seen_i3);

	my_set_clear(&set);
	assert(my_set_size(&set) == 0);
	
	my_set_insert(&set, (void *)1);
	my_set_insert(&set, (void *)2);
	assert(my_set_size(&set) == 2);
	
	seen_i1 = seen_i2 = false;
	JIVE_SET_ITERATE(my_set, set, iter) {
		if (iter.entry->item == (void *)1)
			seen_i1 = true;
		if (iter.entry->item == (void *)2)
			seen_i2 = true;
	}
	assert(seen_i1 && seen_i2);

	
	struct my_set set2, res;
	my_set_init(&set2, context);
	my_set_init(&res, context);

	my_set_insert(&set2, (void *)1);
	my_set_insert(&set2, (void *)3);
	my_set_union(&set, &set2, &res);
	assert(my_set_size(&res) == 3);

	my_set_intersection(&set, &set2, &res);
	assert(my_set_size(&res) == 1);

	my_set_fini(&set);
	my_set_fini(&set2);
	my_set_fini(&res);	

	jive_context_assert_clean(context);
	jive_context_destroy(context);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("util/test-set", test_main);
