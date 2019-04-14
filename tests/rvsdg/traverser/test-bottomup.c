/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/traverser.h>

static void
test_initialization()
{
	jive::graph graph;
	jive::test::valuetype vtype;
	auto n1 = jive::test::simple_node_create(graph.root(), {}, {}, {});
	auto n2 = jive::test::simple_node_create(graph.root(), {}, {}, {vtype});

	graph.add_export(n2->output(0), {n2->output(0)->type(), "dummy"});

	bool n1_visited = false;
	bool n2_visited = false;
	for (const auto & node : jive::bottomup_traverser(graph.root())) {
		if (node == n1) n1_visited = true;
		if (node == n2) n2_visited = true;
	}

	assert(n1_visited);
	assert(n2_visited);
}

static void
test_basic_traversal()
{
	jive::graph graph;
	jive::test::valuetype type;
	auto n1 = jive::test::simple_node_create(graph.root(), {}, {}, {type, type});
	auto n2 = jive::test::simple_node_create(graph.root(), {type, type},
		{n1->output(0), n1->output(1)}, {type});

	graph.add_export(n2->output(0), {n2->output(0)->type(), "dummy"});

	{
		jive::node * tmp;
		jive::bottomup_traverser trav(graph.root());
		tmp = trav.next();
		assert(tmp == n2);
		tmp = trav.next();
		assert(tmp == n1);
		tmp = trav.next();
		assert(tmp == 0);
	}

	assert(!has_active_trackers(&graph));
}

static void
test_order_enforcement_traversal()
{
	jive::graph graph;
	jive::test::valuetype type;
	auto n1 = jive::test::simple_node_create(graph.root(), {}, {}, {type, type});
	auto n2 = jive::test::simple_node_create(graph.root(), {type}, {n1->output(0)}, {type});
	auto n3 = jive::test::simple_node_create(graph.root(), {type, type},
		{n2->output(0), n1->output(1)},
		{type});

	jive::node * tmp;
	{
		jive::bottomup_traverser trav(graph.root());

		tmp = trav.next();
		assert(tmp == n3);
		tmp = trav.next();
		assert(tmp == n2);
		tmp = trav.next();
		assert(tmp == n1);
		tmp = trav.next();
		assert(tmp == nullptr);
	}

	assert(!has_active_trackers(&graph));
}

static int
test_main()
{
	test_initialization();
	test_basic_traversal();
	test_order_enforcement_traversal();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("rvsdg/traverser/test-bottomup", test_main)
