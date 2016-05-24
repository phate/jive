/*
 * Copyright 2010 2011 2012 2013 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <locale.h>
#include <stdio.h>

#include <jive/arch/registers.h>
#include <jive/regalloc/selector-cost.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/node-private.h>

#include "testnodes.h"

extern const jive_register_class gpr;

const jive_register_name
	r0 = {base : {name : "r0", resource_class : &gpr.base}, code : 0},
	r1 = {base : {name : "r1", resource_class : &gpr.base}, code : 0},
	r2 = {base : {name : "r2", resource_class : &gpr.base}, code : 0},
	r3 = {base : {name : "r3", resource_class : &gpr.base}, code : 0};

static const jive_resource_name * res_names [] = {&r0.base, &r1.base, &r2.base, &r3.base};

const jive_register_class gpr = {
		base : {
			class_ : &JIVE_REGISTER_RESOURCE,
			name : "gpr",
			limit : 4,
			names : res_names,
			parent : &jive_root_resource_class,
			depth : 1,
			priority : jive_resource_class_priority_reg_low,
		},
	}
;

static void
shape(jive_shaped_graph * shaped_graph, jive_node * node)
{
	for (size_t n = 0; n < node->ninputs; n++) {
		jive::input * input = node->inputs[n];
		jive_ssavar * ssavar = jive_input_auto_merge_variable(input);
		shaped_graph->map_ssavar(ssavar)->lower_boundary_region_depth(node->region->depth());
	}
	
	for (size_t n = 0; n < node->noutputs; n++) {
		jive::output * output = node->outputs[n];
		jive_output_auto_merge_variable(output);
	}
	
	jive_shaped_region * shaped_region = shaped_graph->map_region(node->region);
	jive_cut * cut = shaped_region->create_top_cut();
	cut->append(node);
}

static jive_node *
create_computation_node(jive_graph * graph,
	size_t noperands, jive::output ** operands,
	size_t noutputs)
{
	jive_test_value_type type;
	std::vector<jive::output*> operands_(operands, operands+noperands);
	std::vector<const jive::base::type*> input_types(noperands, &type);
	std::vector<const jive::base::type*> output_types(noutputs, &type);

	jive_node * node = jive_test_node_create(graph->root_region,
		input_types, operands_, output_types);

	for (size_t n = 0; n < noperands; n++)
		node->inputs[n]->required_rescls = &gpr.base;
	for (size_t n = 0; n < noutputs; n++)
		node->outputs[n]->required_rescls = &gpr.base;
	
	return node;
}

static jive_node *
create_spill_node(jive_graph * graph,
	jive::output * operand)
{
	jive_test_value_type type;
	jive_node * node = jive_test_node_create(graph->root_region, {&type}, {operand}, {&type});
	node->inputs[0]->required_rescls = &gpr.base;
	node->outputs[0]->required_rescls = &jive_root_resource_class;
	
	return node;
}

static jive_node *
create_restore_node(jive_graph * graph,
	jive::output * operand)
{
	jive_test_value_type type;
	jive_node * node = jive_test_node_create(graph->root_region, {&type}, {operand}, {&type});
	node->inputs[0]->required_rescls = &jive_root_resource_class;
	node->outputs[0]->required_rescls = &gpr.base;
	
	return node;
}

static int test_main(void)
{
	setlocale(LC_ALL, "");
	jive_graph * graph = jive_graph_create();
	
	jive_node * n1 = create_computation_node(graph, 0, NULL, 2);
	jive::output * tmparray0[] = {n1->outputs[0]};
	jive_node * l1 = create_computation_node(graph, 1, tmparray0, 1);
	jive::output * tmparray1[] = {n1->outputs[1]};
	jive_node * l2 = create_computation_node(graph, 1, tmparray1, 1);
	jive::output * tmparray2[] = {l1->outputs[0]};
	jive_node * a = create_computation_node(graph, 1, tmparray2, 1);
	jive::output * tmparray3[] = {l1->outputs[0]};
	jive_node * b = create_computation_node(graph, 1, tmparray3, 1);
	jive::output * tmparray4[] = {l1->outputs[0]};
	jive_node * c = create_computation_node(graph, 1, tmparray4, 1);
	jive::output * tmparray5[] = {l1->outputs[0]};
	jive_node * d = create_computation_node(graph, 1, tmparray5, 1);
	jive::output * tmparray6[] = {l1->outputs[0]};
	jive_node * e = create_computation_node(graph, 1, tmparray6, 1);
	jive::output * tmparray7[] = {l2->outputs[0]};
	jive_node * f = create_computation_node(graph, 1, tmparray7, 1);
	jive::output * tmparray8[] = {a->outputs[0], b->outputs[0]};
	
	jive_node * s1 = create_computation_node(
		graph, 2, tmparray8, 1);
	jive::output * tmparray9[] = {c->outputs[0], s1->outputs[0]};
	jive_node * s2 = create_computation_node(
		graph, 2, tmparray9, 1);
	jive::output * tmparray10[] = {s2->outputs[0], d->outputs[0]};
	jive_node * s3 = create_computation_node(
		graph, 2, tmparray10, 1);
	jive::output * tmparray11[] = {e->outputs[0], s3->outputs[0]};
	jive_node * s4 = create_computation_node(
		graph, 2, tmparray11, 1);
	jive::output * tmparray12[] = {s4->outputs[0], f->outputs[0]};
	jive_node * s5 = create_computation_node(
		graph, 2, tmparray12, 1);
	
	jive_node_add_input(graph->root_region->bottom, &s5->outputs[0]->type(), s5->outputs[0]);
	jive_node * bottom = graph->root_region->bottom;
	
	//jive_view(graph, stdout);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	{
		jive::regalloc::master_selector_cost master_selector(shaped_graph);
		
		jive::regalloc::region_selector_cost * region_selector;
		region_selector = master_selector.map_region(graph->root_region);
		
		{
			jive::regalloc::node_selection_order * n1_cost = master_selector.map_node(n1);
			assert(n1_cost->prio_array().count[0] == jive_resource_class_priority_lowest);
			assert(n1_cost->force_tree_root() == true);
			
			/* test invalidation -- artifically modify stored count to make
			sure count after recomputation differs */
			const_cast<jive_resource_class_count &>(n1_cost->rescls_cost()).add(
				&jive_root_resource_class);
			const_cast<uint16_t &>(n1_cost->prio_array().count[0]) = 0;
			master_selector.invalidate_node(n1);
			n1_cost = master_selector.map_node(n1);
			assert(n1_cost->prio_array().count[0] == jive_resource_class_priority_lowest);
		}
		
		{
			assert(master_selector.map_node(l1)->force_tree_root() == true);
			assert(master_selector.map_node(l2)->force_tree_root() == false);
			
			assert(master_selector.map_node(l1)->blocked_rescls_priority()
				== jive_root_resource_class.priority);
			assert(master_selector.map_node(bottom)->blocked_rescls_priority()
				== jive_root_resource_class.priority);
			
			assert(master_selector.map_node(bottom)->state()
				== jive::regalloc::node_selection_order::state_queue);
			assert(master_selector.map_node(bottom)->queue_index()
				== master_selector.map_region(graph->root_region)->node_queue().begin());
		}

		jive_node * node;
		
		assert(region_selector->node_queue().size() == 1);
		node = region_selector->select_node();
		assert(node == bottom);
		assert(master_selector.map_node(bottom)->state()
			== jive::regalloc::node_selection_order::state_stack);
		shape(shaped_graph, node);
		
		assert(master_selector.map_node(s5)->prio_array().count[0]
			== gpr.base.priority);
		node = region_selector->select_node();
		assert(node == s5);
		shape(shaped_graph, node);
		
		assert(master_selector.map_node(f)->prio_array().count[0]
			== gpr.base.priority);
		assert(region_selector->node_queue().size() == 2);
		assert((*region_selector->node_queue().begin())->node() == f);
		assert((*region_selector->node_queue().rbegin())->node() == s4);
		node = region_selector->select_node();
		assert(node == f);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == l2);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == s4);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == e);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == s3);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == d);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == s2);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == c);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == s1);
		shape(shaped_graph, node);
		
		jive_node * tmp = region_selector->select_node();
		assert(tmp == a || tmp == b);
		assert(master_selector.map_node(tmp)->state()
			== jive::regalloc::node_selection_order::state_stack);
		jive_node * spill = create_spill_node(graph, tmp->outputs[0]);
		assert(master_selector.map_node(spill)->state()
			== jive::regalloc::node_selection_order::state_stack);
		jive_node * restore = create_restore_node(graph, spill->outputs[0]);
		assert(master_selector.map_node(restore)->state()
			== jive::regalloc::node_selection_order::state_stack);
		jive_ssavar * ssavar = tmp->outputs[0]->users.first->ssavar;
		jive_ssavar_divert_origin(ssavar, restore->outputs[0]);
		
		node = region_selector->select_node();
		assert(node == restore);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == spill);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == tmp);
		shape(shaped_graph, node);
		
		node = region_selector->select_node();
		assert(node == a || node == b);
		shape(shaped_graph, node);
	}

	jive_shaped_graph_destroy(shaped_graph);
	jive_graph_destroy(graph);

	return 0;
}
JIVE_UNIT_TEST_REGISTER("shape/test-selector", test_main);
