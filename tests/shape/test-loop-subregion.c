/*
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>
#include <jive/context.h>
#include <jive/arch/registers.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/resource-private.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg.h>
#include <jive/view.h>

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
	jive_context * ctx = jive_context_create();
	
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_region * root = graph->root_region;
	
	jive_test_value_type type;
	jive::achr::type anchor_type;
	const jive::base::type *  tmparray0[] = {&type, &type};
	
	jive_node * top = jive_test_node_create(root,
		0, NULL, NULL,
		2, tmparray0);
	
	jive_region * loop_region = jive_region_create_subregion(root);
	loop_region->attrs.is_looped = true;
	const jive::base::type *  tmparray1[] = {&type};
	jive::output * tmparray2[] = {top->outputs[0]};
	const jive::base::type *  tmparray3[] = {&type};
	
	jive_node * loop_head = jive_test_node_create(loop_region,
		1, tmparray1, tmparray2,
		1, tmparray3);
	loop_region->top = loop_head;
	
	jive_region * loop_subregion = jive_region_create_subregion(loop_region);
	const jive::base::type *  tmparray4[] = {&type, &type};
	jive::output * tmparray5[] = {loop_head->outputs[0], top->outputs[1]};
	const jive::base::type *  tmparray6[] = {&anchor_type};
	
	jive_node * loop_body = jive_test_node_create(loop_subregion,
		2, tmparray4, tmparray5,
		1, tmparray6);
	const jive::base::type *  tmparray7[] = {&anchor_type};
	jive::output * tmparray8[] = {loop_body->outputs[0]};
	const jive::base::type *  tmparray9[] = {&type};
	
	jive_node * loop_body_anchor = jive_test_node_create(loop_region,
		1, tmparray7, tmparray8,
		1, tmparray9);
	const jive::base::type *  tmparray10[] = {&type};
	jive::output * tmparray11[] = {loop_body_anchor->outputs[0]};
	const jive::base::type *  tmparray12[] = {&anchor_type};
	
	jive_node * loop_tail = jive_test_node_create(loop_region,
		1, tmparray10, tmparray11,
		1, tmparray12);
	loop_region->bottom = loop_tail;
	const jive::base::type *  tmparray13[] = {&anchor_type};
	jive::output * tmparray14[] = {loop_tail->outputs[0]};
	const jive::base::type *  tmparray15[] = {&type};
	
	jive_node * loop_anchor = jive_test_node_create(root,
		1, tmparray13, tmparray14,
		1, tmparray15);
	const jive::base::type *  tmparray16[] = {&type};
	jive::output * tmparray17[] = {loop_anchor->outputs[0]};
	
	jive_node * bottom = jive_test_node_create(root,
		1, tmparray16, tmparray17,
		0, NULL);
	
	// jive_node_reserve(bottom);
	
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
	
	assert(jive_shaped_ssavar_is_active_before(jive_shaped_graph_map_ssavar(shaped_graph, loop_body->inputs[1]->ssavar), shaped_tail));
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-loop-subregion", test_main);
