/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <stdio.h>

#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static bool
graph_contains_node(jive::graph * graph, jive::node * node)
{
	bool found = false;
	
	for (jive::node * tmp : jive::topdown_traverser(graph->root())) {
		found = found || (tmp == node);
	}
	
	return found;
}

static int test_main(void)
{
	jive::graph graph;
	
	jive::region * region = graph.root();
	jive_test_value_type type;
	auto n1 = jive::test::node_create(region, {}, {}, {&type});
	auto n2 = jive::test::node_create(region, {&type}, {n1->output(0)}, {&type});
	auto n3 = jive::test::node_create(region, {&type}, {n2->output(0)}, {&type});
	auto bottom = jive::test::node_create(region,
		{&type, &type}, {n2->output(0), n3->output(0)}, {&type});
	
	graph.export_port(bottom->output(0), "dummy");
	
	auto n4 = jive::test::node_create(region, {&type}, {n1->output(0)}, {&type});

	n2->output(0)->replace(n4->output(0));
	assert(n2->output(0)->no_user());
	
	graph.prune();
	
	assert(!graph_contains_node(&graph, n2));

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-prune-replace", test_main);
