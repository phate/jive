/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/collector.h>
#include <jive/context.h>

#include <assert.h>

static bool fini_called = false;

static void
fini(void * unused)
{
	fini_called = true;
}

static int
test_main(void)
{
	jive_context * context = jive_context_create();
	jive_collector * collector = jive_collector_create(context);

	size_t * foobar = jive_context_malloc(context, sizeof(*foobar));
	jive_collector_register(collector, foobar, fini);

	jive_collector_destroy(collector);
	assert(fini_called);

	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("test-collector", test_main);
