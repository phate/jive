/*
 * Copyright 2017 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testnodes.h"
#include "testtypes.h"

#include <jive/vsdg/graph.h>
#include <jive/vsdg/traverser.h>

static void
test_initialization()
{
	jive::graph graph;
	jive_test_value_type vtype;
	auto n1 = jive::test::node_create(graph.root(), {}, {}, {});
	auto n2 = jive::test::node_create(graph.root(), {}, {}, {&vtype});

	graph.export_port(n2->output(0), "dummy");

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
	jive_test_value_type type;
	auto n1 = jive::test::node_create(graph.root(), {}, {}, {&type, &type});
	auto n2 = jive::test::node_create(graph.root(), {&type, &type}, {n1->output(0), n1->output(1)},
		{&type});

	graph.export_port(n2->output(0), "dummy");

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

	assert(!graph.has_active_traversers());
}

static void
test_order_enforcement_traversal()
{
	jive::graph graph;
	jive_test_value_type type;
	auto n1 = jive::test::node_create(graph.root(), {}, {}, {&type, &type});
	auto n2 = jive::test::node_create(graph.root(), {&type}, {n1->output(0)}, {&type});
	auto n3 = jive::test::node_create(graph.root(), {&type, &type}, {n2->output(0), n1->output(1)},
		{&type});

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

	assert(!graph.has_active_traversers());
}

static int
test_main()
{
	test_initialization();
	test_basic_traversal();
	test_order_enforcement_traversal();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/traverser/test-bottomup", test_main);