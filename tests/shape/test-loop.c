/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>

#include <jive/arch/registers.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

static jive_shaped_node *
shape(jive_shaped_graph * shaped_graph, jive_node * node)
{
	size_t n;
	for(n = 0; n < node->ninputs; n ++) {
		jive::input * input = node->inputs[n];
		jive_input_auto_merge_variable(input);
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, input->ssavar);
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar, node->region->depth);
	}
	for(n = 0; n < node->noutputs; n ++) {
		jive::output * output = node->outputs[n];
		jive_output_auto_merge_variable(output);
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, output->ssavar);
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar, node->region->depth);
	}
	jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph, node->region);
	jive_cut * cut = jive_shaped_region_create_cut(shaped_region);
	return jive_cut_append(cut, node);
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();
	
	jive_region * root = graph->root_region;
	
	jive_test_value_type type;
	jive::achr::type anchor_type;
	jive_node * top = jive_test_node_create(root, {}, {}, {&type, &type});

	jive_region * loop_region = jive_region_create_subregion(root);
	loop_region->attrs.is_looped = true;

	jive_node * loop_head = jive_test_node_create(loop_region, {&type}, {top->outputs[0]}, {&type});
	loop_region->top = loop_head;
	
	jive_node * loop_body = jive_test_node_create(loop_region,
		{&type, &type}, {loop_head->outputs[0], top->outputs[1]}, {&type});
	
	jive_node * loop_tail = jive_test_node_create(loop_region,
		{&type}, {loop_body->outputs[0]}, {&anchor_type});
	loop_region->bottom = loop_tail;

	jive_node * loop_anchor = jive_test_node_create(root,
		{&anchor_type}, {loop_tail->outputs[0]}, {&type});

	jive_node * bottom = jive_test_node_create(root, {&type}, {loop_anchor->outputs[0]}, {&type});
	jive_graph_export(graph, bottom->outputs[0]);
	
	jive_view(graph, stderr);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_node * shaped_bottom = shape(shaped_graph, bottom);
	jive_shaped_node * shaped_anchor = shape(shaped_graph, loop_anchor);
	jive_shaped_node * shaped_tail = shape(shaped_graph, loop_tail);
	jive_shaped_node * shaped_body = shape(shaped_graph, loop_body);
	
	(void) shaped_bottom;
	(void) shaped_anchor;
	(void) shaped_tail;
	(void) shaped_body;
	
	assert(jive_shaped_ssavar_is_active_before(jive_shaped_graph_map_ssavar(shaped_graph,
		loop_body->inputs[1]->ssavar), shaped_tail));
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-loop", test_main);
