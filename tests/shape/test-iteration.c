/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/types/bitstring.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static void
shape(jive_shaped_graph * shaped_graph, jive_node * node)
{
	jive_shaped_region * shaped_region = shaped_graph->map_region(node->region);
	jive_cut * cut = shaped_region->create_top_cut();
	cut->append(node);
}

template<typename T>
static std::vector<jive_node *>
record_iterate(const T & range)
{
	std::vector<jive_node *> result;
	for (const jive_shaped_node & v : range) {
		result.push_back(v.node());
	}
	return result;
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();
	
	jive_test_value_type type;
	jive::achr::type anchor_type;
	jive_node * n1 = jive_test_node_create(graph->root_region, {}, {}, {&type});
	
	jive_region * r1 = new jive_region(graph->root_region, graph);

	jive_node * n2 = jive_test_node_create(r1, {&type}, {n1->outputs[0]}, {&type});

	jive_region * r2 = new jive_region(r1, graph);
	jive_region * r3 = new jive_region(r1, graph);

	jive_node * n3 = jive_test_node_create(r2, {&type}, {n2->outputs[0]}, {&anchor_type});
	jive_node * n4 = jive_test_node_create(r3, {&type}, {n2->outputs[0]}, {&anchor_type});
	jive_node * n5 = jive_test_node_create(r1, {&anchor_type, &anchor_type},
		{n3->outputs[0], n4->outputs[0]}, {&type});
	jive_node * n6 = jive_test_node_create(r1, {&type}, {n5->outputs[0]}, {&anchor_type});
	jive_node * n7 = jive_test_node_create(graph->root_region, {&anchor_type}, {n6->outputs[0]}, {});

	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	shape(shaped_graph, n7);
	shape(shaped_graph, n6);
	shape(shaped_graph, n5);
	shape(shaped_graph, n4);
	shape(shaped_graph, n3);
	shape(shaped_graph, n2);
	shape(shaped_graph, n1);
	
	{
		std::vector<jive_node *> nodes = record_iterate(
			shaped_graph->map_node(n1)->range_to_end());
		
		assert(nodes.size() == 7);
		assert(nodes[0] == n1);
		assert(nodes[1] == n2);
		assert(nodes[2] == n3 || nodes[2] == n4);
		assert(nodes[3] == n4 || nodes[3] == n3);
		assert(nodes[4] == n5);
		assert(nodes[5] == n6);
		assert(nodes[6] == n7);
	}

	{
		std::vector<jive_node *> nodes = record_iterate(
			shaped_graph->map_node(n2)->range_to_end());
		
		assert(nodes.size() == 5);
		assert(nodes[0] == n2);
		assert(nodes[1] == n3 || nodes[2] == n4);
		assert(nodes[2] == n4 || nodes[3] == n3);
		assert(nodes[3] == n5);
		assert(nodes[4] == n6);
	}
	
	{
		std::vector<jive_node *> nodes = record_iterate(
			shaped_graph->map_node(n3)->range_to_end());

		assert(nodes.size() == 1);
		assert(nodes[0] == n3);
	}
	
	{
		std::vector<jive_node *> nodes = record_iterate(
			shaped_graph->map_node(n4)->range_to_end());

		assert(nodes.size() == 1);
		assert(nodes[0] == n4);
	}
	
	{
		std::vector<jive_node *> nodes = record_iterate(
			shaped_graph->map_node(n5)->range_to_end());

		assert(nodes.size() == 2);
		assert(nodes[0] == n5);
		assert(nodes[1] == n6);
	}
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-iteration", test_main);
