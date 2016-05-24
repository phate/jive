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
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static int
test_main()
{
	setlocale(LC_ALL, "");

	jive_graph * graph = jive_graph_create();

	jive_test_value_type vtype;
	jive_region * inner_region = new jive_region(graph->root_region, graph);
	jive_node * inner_node = jive_test_node_create(inner_region, {}, {}, {&vtype});
	jive::node_normal_form * normal_form =
		jive_graph_get_nodeclass_form(graph, typeid(test_operation));

	test_operation op;
	jive_node * outer_node = jive_node_cse_create(normal_form, graph->root_region, op, {});

	assert(inner_node != outer_node);

	jive_view(graph, stderr);

	jive_graph_destroy(graph);

	return 0;
}

JIVE_UNIT_TEST_REGISTER("vsdg/test-node_cse", test_main);
