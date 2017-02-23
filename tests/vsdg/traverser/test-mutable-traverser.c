/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <stdio.h>

#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

void test_mutable_traverse_topdown(jive_graph * graph, jive::node * n1, jive::node * n2,
	jive::node * n3)
{
	bool seen_n1 = false;
	bool seen_n2 = false;
	bool seen_n3 = false;
	
	for (jive::node * tmp : jive::topdown_traverser(graph)) {
		seen_n1 = seen_n1 || (tmp == n1);
		seen_n2 = seen_n2 || (tmp == n2);
		seen_n3 = seen_n3 || (tmp == n3);
		if (n3->input(0)->origin() == n1->output(0))
			n3->input(0)->divert_origin(n2->output(0));
		else
			n3->input(0)->divert_origin(n1->output(0));
	}
	
	assert(seen_n1);
	assert(seen_n2);
	assert(seen_n3);
}

static int test_main(void)
{
	jive_graph graph;
	
	jive::region * region = graph.root();
	jive_test_value_type type;
	jive::node * n1 = jive_test_node_create(region, {}, {}, {&type});
	jive::node * n2 = jive_test_node_create(region, {}, {}, {&type});
	jive::node * bottom = jive_test_node_create(region, {&type}, {n1->output(0)}, {});

	test_mutable_traverse_topdown(&graph, n1, n2, bottom);
	
	test_mutable_traverse_topdown(&graph, n1, n2, bottom);

	/* should also write a test that exercises the bottom-up
	traverser, but since bottom-up traversal is inherently
	more robust I do not have a non-contrived "error scenario"
	yet */
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/traverser/test-mutable-traverser", test_main);
