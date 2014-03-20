/*
 * Copyright 2013 2014 Nico Reissmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>
#include <jive/context.h>

static int
test_main(void)
{
	jive_context * context = jive_context_create();
	jive_clg * clg = jive_clg_create(context);

	jive_clg_node * node1 = jive_clg_node_create(clg, "foo");
	jive_clg_node * node2 = jive_clg_node_create(clg, "bar");
	jive_clg_node_add_call(node1, node2);

//	jive_clg_view(clg);

	jive_clg_destroy(clg);
	jive_context_assert_clean(context);
	jive_context_destroy(context);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("frontend/test-clg", test_main);
