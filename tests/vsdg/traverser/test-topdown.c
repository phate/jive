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
	jive_test_value_type vtype;

	jive_graph graph;
	auto i = graph.import(vtype, "i");

	auto constant = jive_test_node_create(graph.root(), {}, {}, {&vtype});
	auto unary = jive_test_node_create(graph.root(), {&vtype}, {i}, {&vtype});
	auto binary = jive_test_node_create(graph.root(), {&vtype, &vtype}, {i, unary->output(0)},
		{&vtype});

	graph.export_port(constant->output(0), "c");
	graph.export_port(unary->output(0), "u");
	graph.export_port(binary->output(0), "b");

	bool unary_visited = false;
	bool binary_visited = false;
	bool constant_visited = false;
	for (const auto & node : jive::topdown_traverser(graph.root())) {
		if (node == unary) unary_visited = true;
		if (node == constant) constant_visited = true;
		if (node == binary && unary_visited) binary_visited = true;
	}

	assert(unary_visited);
	assert(binary_visited);
	assert(constant_visited);
}

static int
test_main(void)
{
	test_initialization();

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/traverser/test-topdown", test_main);
