/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static int
test_main()
{
	setlocale(LC_ALL, "");

	jive_graph graph;

	jive_test_value_type vtype;
	jive::region * inner_region = new jive::region(graph.root(), &graph);
	jive::node * inner_node = jive_test_node_create(inner_region, {}, {}, {&vtype});
	auto normal_form = graph.node_normal_form(typeid(test_operation));

	test_operation op;
	jive::node * outer_node = jive_node_cse_create(normal_form, graph.root(), op, {});

	assert(inner_node != outer_node);

	jive_view(&graph, stderr);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-node_cse", test_main);
