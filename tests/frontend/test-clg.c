/*
 * Copyright 2013 2014 Nico Reissmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <jive/frontend/clg.h>
#include <jive/frontend/clg_node.h>

static int
test_main(void)
{
	jive::frontend::clg clg;

	jive::frontend::clg_node node1(clg, "foo");
	jive::frontend::clg_node node2(clg, "bar");
	jive_clg_node_add_call(node1, node2);

//	jive_clg_view(clg);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("frontend/test-clg", test_main);
