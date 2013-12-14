/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/regalloc/selector.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/resource-private.h>

extern const jive_register_class gpr;

const jive_register_name
	r0 = {.base = {.name = "r0", .resource_class = &gpr.base}, .code = 0},
	r1 = {.base = {.name = "r1", .resource_class = &gpr.base}, .code = 0},
	r2 = {.base = {.name = "r2", .resource_class = &gpr.base}, .code = 0},
	r3 = {.base = {.name = "r3", .resource_class = &gpr.base}, .code = 0};

static const jive_resource_name * res_names [] = {&r0.base, &r1.base, &r2.base, &r3.base};

const jive_register_class gpr = {
		.base = {
			.class_ = &JIVE_REGISTER_RESOURCE,
			.name = "gpr",
			.limit = 4,
			.names = res_names,
			.parent = &jive_root_resource_class,
			.depth = 1,
			.priority = jive_resource_class_priority_reg_low,
		},
	}
;

static void
shape(jive_shaped_graph * shaped_graph, jive_node * node)
{
	size_t n;
	for(n = 0; n < node->ninputs; n++) {
		jive_input * input = node->inputs[n];
		jive_ssavar * ssavar = jive_input_auto_merge_variable(input);
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar, node->region->depth);
	}
	
	for(n = 0; n < node->noutputs; n++) {
		jive_output * output = node->outputs[n];
		jive_output_auto_merge_variable(output);
	}
	
	jive_shaped_region * shaped_region = jive_shaped_graph_map_region(shaped_graph, node->region);
	jive_cut * cut = jive_shaped_region_create_cut(shaped_region);
	jive_cut_append(cut, node);
}

static jive_node *
create_computation_node(jive_graph * graph,
	size_t noperands, jive_output ** operands,
	size_t noutputs)
{
	JIVE_DECLARE_TYPE(type);
	const jive_type * input_types[noperands];
	const jive_type * output_types[noperands];
	
	size_t n;
	for (n = 0; n < noperands; n++)
		input_types[n] = type;
	for (n = 0; n < noutputs; n++)
		output_types[n] = type;
	
	
	jive_node * node = jive_node_create(graph->root_region,
		noperands, input_types, operands,
		noutputs, output_types);
	
	for (n = 0; n < noperands; n++)
		node->inputs[n]->required_rescls = &gpr.base;
	for (n = 0; n < noutputs; n++)
		node->outputs[n]->required_rescls = &gpr.base;
	
	return node;
}

static jive_node *
create_spill_node(jive_graph * graph,
	jive_output * operand)
{
	JIVE_DECLARE_TYPE(type);
	
	jive_node * node = jive_node_create(graph->root_region,
		1, &type, &operand,
		1, &type);
	
	node->inputs[0]->required_rescls = &gpr.base;
	node->outputs[0]->required_rescls = &jive_root_resource_class;
	
	return node;
}

static jive_node *
create_restore_node(jive_graph * graph,
	jive_output * operand)
{
	JIVE_DECLARE_TYPE(type);
	
	jive_node * node = jive_node_create(graph->root_region,
		1, &type, &operand,
		1, &type);
	
	node->inputs[0]->required_rescls = &jive_root_resource_class;
	node->outputs[0]->required_rescls = &gpr.base;
	
	return node;
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_node * n1 = create_computation_node(graph, 0, NULL, 2);
	jive_node * l1 = create_computation_node(graph, 1, (jive_output *[]){n1->outputs[0]}, 1);
	jive_node * l2 = create_computation_node(graph, 1, (jive_output *[]){n1->outputs[1]}, 1);
	jive_node * a = create_computation_node(graph, 1, (jive_output *[]){l1->outputs[0]}, 1);
	jive_node * b = create_computation_node(graph, 1, (jive_output *[]){l1->outputs[0]}, 1);
	jive_node * c = create_computation_node(graph, 1, (jive_output *[]){l1->outputs[0]}, 1);
	jive_node * d = create_computation_node(graph, 1, (jive_output *[]){l1->outputs[0]}, 1);
	jive_node * e = create_computation_node(graph, 1, (jive_output *[]){l1->outputs[0]}, 1);
	jive_node * f = create_computation_node(graph, 1, (jive_output *[]){l2->outputs[0]}, 1);
	
	jive_node * s1 = create_computation_node(
		graph, 2, (jive_output *[]){a->outputs[0], b->outputs[0]}, 1);
	jive_node * s2 = create_computation_node(
		graph, 2, (jive_output *[]){c->outputs[0], s1->outputs[0]}, 1);
	jive_node * s3 = create_computation_node(
		graph, 2, (jive_output *[]){s2->outputs[0], d->outputs[0]}, 1);
	jive_node * s4 = create_computation_node(
		graph, 2, (jive_output *[]){e->outputs[0], s3->outputs[0]}, 1);
	jive_node * s5 = create_computation_node(
		graph, 2, (jive_output *[]){s4->outputs[0], f->outputs[0]}, 1);
	
	jive_node * bottom = create_computation_node(graph, 1, (jive_output *[]){s5->outputs[0]}, 0);
	
	//jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	jive_master_shaper_selector * master_selector;
	master_selector = jive_master_shaper_selector_create(shaped_graph);
	
	jive_region_shaper_selector * region_selector;
	region_selector = jive_master_shaper_selector_map_region(master_selector, graph->root_region);
	
	{
		jive_node_cost * n1_cost = jive_master_shaper_selector_map_node(master_selector, n1);
		assert(n1_cost->prio_array.count[0] == jive_resource_class_priority_lowest);
		assert(n1_cost->force_tree_root == true);
		
		/* test invalidation -- artifically modify stored count to make
		sure count after recomputation differs */
		jive_resource_class_count_add(&n1_cost->rescls_cost, &jive_root_resource_class);
		n1_cost->prio_array.count[0] = 0;
		jive_master_shaper_selector_invalidate_node(master_selector, n1);
		n1_cost = jive_master_shaper_selector_map_node(master_selector, n1);
		assert(n1_cost->prio_array.count[0] == jive_resource_class_priority_lowest);
	}
	
	{
		assert(jive_master_shaper_selector_map_node(master_selector, l1)->force_tree_root == true);
		assert(jive_master_shaper_selector_map_node(master_selector, l2)->force_tree_root == false);
		
		assert(jive_master_shaper_selector_map_node(master_selector, l1)->blocked_rescls_priority
			== jive_root_resource_class.priority);
		assert(jive_master_shaper_selector_map_node(master_selector, bottom)->blocked_rescls_priority
			== jive_root_resource_class.priority);
		
		assert(jive_master_shaper_selector_map_node(master_selector, bottom)->state
			== jive_node_cost_state_queue);
		assert(jive_master_shaper_selector_map_node(master_selector, bottom)->index
			== 0);
	}

	jive_node * node;
	
	assert(region_selector->prio_heap.nitems == 1);
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == bottom);
	assert(jive_master_shaper_selector_map_node(master_selector, bottom)->state
		== jive_node_cost_state_stack);
	shape(shaped_graph, node);
	
	assert(jive_master_shaper_selector_map_node(master_selector, s5)->prio_array.count[0]
		== gpr.base.priority);
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == s5);
	shape(shaped_graph, node);
	
	assert(jive_master_shaper_selector_map_node(master_selector, f)->prio_array.count[0]
		== gpr.base.priority);
	assert(region_selector->prio_heap.nitems == 2);
	assert(region_selector->prio_heap.items[0]->node == f);
	assert(region_selector->prio_heap.items[1]->node == s4);
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == f);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == l2);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == s4);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == e);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == s3);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == d);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == s2);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == c);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == s1);
	shape(shaped_graph, node);
	
	jive_node * tmp = jive_region_shaper_selector_select_node(region_selector);
	assert(tmp == a || tmp == b);
	assert(jive_master_shaper_selector_map_node(master_selector, tmp)->state
		== jive_node_cost_state_stack);
	jive_node * spill = create_spill_node(graph, tmp->outputs[0]);
	assert(jive_master_shaper_selector_map_node(master_selector, spill)->state
		== jive_node_cost_state_stack);
	jive_node * restore = create_restore_node(graph, spill->outputs[0]);
	assert(jive_master_shaper_selector_map_node(master_selector, restore)->state
		== jive_node_cost_state_stack);
	jive_ssavar * ssavar = tmp->outputs[0]->users.first->ssavar;
	jive_ssavar_divert_origin(ssavar, restore->outputs[0]);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == restore);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == spill);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == tmp);
	shape(shaped_graph, node);
	
	node = jive_region_shaper_selector_select_node(region_selector);
	assert(node == a || node == b);
	shape(shaped_graph, node);
	
	jive_master_shaper_selector_destroy(master_selector);
	jive_shaped_graph_destroy(shaped_graph);
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}
JIVE_UNIT_TEST_REGISTER("shape/test-selector", test_main);
