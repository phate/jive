/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>
#include <jive/arch/registers.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>

#include "testnodes.h"

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();
	
	jive_region * region = graph->root_region;
	
	jive_test_value_type type;
	
	jive_node * node = jive_test_node_create(region,
		0, NULL, NULL,
		0, NULL);
	
	jive::gate * g1 = type.create_gate(graph, "g1");
	jive::gate * g2 = type.create_gate(graph, "g2");
	jive::gate * g3 = type.create_gate(graph, "g3");
	
	jive_node_gate_output(node, g1);
	jive_node_gate_output(node, g2);
	jive::output * o = jive_node_gate_output(node, g3);
	
	jive_variable * v1 = jive_variable_create(graph);
	jive_variable * v2 = jive_variable_create(graph);
	jive_variable * v3 = jive_variable_create(graph);
	
	jive_variable_assign_gate(v1, g1);
	jive_variable_assign_gate(v2, g2);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v1),
		jive_shaped_graph_map_variable(shaped_graph, v2)));
	
	jive_variable_assign_gate(v3, g3);
	
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v1),
		jive_shaped_graph_map_variable(shaped_graph, v3)));
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v2),
		jive_shaped_graph_map_variable(shaped_graph, v3)));
	
	jive_variable_unassign_gate(v1, g1);
	
	assert(!jive_shaped_graph_map_variable(shaped_graph, v1));
	
	delete o;
	
	assert(!jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v2),
		jive_shaped_graph_map_variable(shaped_graph, v3)));
	
	jive_node_gate_output(node, g3);
	
	assert(jive_shaped_variable_interferes_with(jive_shaped_graph_map_variable(shaped_graph, v2),
		jive_shaped_graph_map_variable(shaped_graph, v3)));
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-gate", test_main);
