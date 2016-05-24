/*
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
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
	for (size_t n = 0; n < node->ninputs; n ++) {
		jive::input * input = node->inputs[n];
		jive_input_auto_merge_variable(input);
		shaped_graph->map_ssavar(input->ssavar)->lower_boundary_region_depth(node->region->depth());
	}
	for (size_t n = 0; n < node->noutputs; n ++) {
		jive::output * output = node->outputs[n];
		jive_output_auto_merge_variable(output);
		shaped_graph->map_ssavar(output->ssavar)->lower_boundary_region_depth(node->region->depth());
	}
	jive_shaped_region * shaped_region = shaped_graph->map_region(node->region);
	jive_cut * cut = shaped_region->create_top_cut();
	return cut->append(node);
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
	
	jive_region * loop_subregion = jive_region_create_subregion(loop_region);

	jive_node * loop_body = jive_test_node_create(loop_subregion,
		{&type, &type}, {loop_head->outputs[0], top->outputs[1]}, {&anchor_type});

	jive_node * loop_body_anchor = jive_test_node_create(loop_region,
		{&anchor_type}, {loop_body->outputs[0]}, {&type});

	jive_node * loop_tail = jive_test_node_create(loop_region,
		{&type}, {loop_body_anchor->outputs[0]}, {&anchor_type});
	loop_region->bottom = loop_tail;

	jive_node * loop_anchor = jive_test_node_create(root,
		{&anchor_type}, {loop_tail->outputs[0]}, {&type});

	jive_node * bottom = jive_test_node_create(root, {&type}, {loop_anchor->outputs[0]}, {});
	
	jive_view(graph, stderr);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_shaped_node * shaped_bottom = shape(shaped_graph, bottom);
	jive_shaped_node * shaped_anchor = shape(shaped_graph, loop_anchor);
	jive_shaped_node * shaped_tail = shape(shaped_graph, loop_tail);
	jive_shaped_node * shaped_body_anchor = shape(shaped_graph, loop_body_anchor);
	jive_shaped_node * shaped_body = shape(shaped_graph, loop_body);
	
	(void) shaped_bottom;
	(void) shaped_anchor;
	(void) shaped_tail;
	(void) shaped_body_anchor;
	(void) shaped_body;
	
	assert(shaped_graph->map_ssavar(loop_body->inputs[1]->ssavar)->is_active_before(shaped_tail));
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-loop-subregion", test_main);
