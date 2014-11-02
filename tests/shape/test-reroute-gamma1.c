/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

/* test rerouting when the value is used below the gamma
point and the two gamma regions have been finished */

#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/node-private.h>
#include <jive/view.h>

#include <jive/regalloc/reroute.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-region.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/regalloc/shaped-variable-private.h>
#include <jive/types/bitstring.h>

#include "testnodes.h"

static jive_shaped_node *
shape(jive_shaped_graph * shaped_graph, jive_node * node)
{
	size_t n;
	for (n = 0; n < node->noutputs; n++) {
		jive_ssavar * ssavar = jive_output_auto_merge_variable(node->outputs[n]);
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar, node->region->depth);
	}
	for (n = 0; n < node->ninputs; n++) {
		jive_ssavar * ssavar = 0;
		jive::input * user;
		JIVE_LIST_ITERATE(node->inputs[n]->origin()->users, user, output_users_list) {
			if (user->ssavar && jive_region_contains_node(user->node->region, node)) {
				ssavar = user->ssavar;
				break;
			}
		}
		if (!ssavar)
			ssavar = jive_input_auto_assign_variable(node->inputs[n]);
		else
			jive_ssavar_assign_input(ssavar, node->inputs[n]);
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
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
	
	jive_test_value_type type;
	jive::ctl::type control_type;
	const jive::base::type * tmparray0[] = {&type};
	
	jive_node * top = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		1, tmparray0);
	const jive::base::type * tmparray1[] = {&type};
	const jive::base::type * tmparray2[] = {&control_type};
	
	jive_node * pred = jive_test_node_create(graph->root_region,
		1, tmparray1, &top->outputs[0],
		1, tmparray2);
	const jive::base::type * tmparray3[] = {&type};
	const jive::base::type * tmparray4[] = {&type};
	
	jive_node * l1 = jive_test_node_create(graph->root_region,
		1, tmparray3, &top->outputs[0],
		1, tmparray4);
	const jive::base::type * tmparray5[] = {&type};
	const jive::base::type * tmparray6[] = {&type};
	
	jive_node * l2 = jive_test_node_create(graph->root_region,
		1, tmparray5, &top->outputs[0],
		1, tmparray6);
	const jive::base::type * tmparray7[] = {&type};
	const jive::base::type * tmparray8[] = {&type};
	
	jive_node * r1 = jive_test_node_create(graph->root_region,
		1, tmparray7, &top->outputs[0],
		1, tmparray8);
	const jive::base::type * tmparray9[] = {&type};
	const jive::base::type * tmparray10[] = {&type};
	
	jive_node * r2 = jive_test_node_create(graph->root_region,
		1, tmparray9, &top->outputs[0],
		1, tmparray10);
	
	jive::output * gamma[2];
	const jive::base::type * tmparray11[] = {&type, &type};
	jive::output * tmparray12[] = {l1->outputs[0], l2->outputs[0]};
	jive::output * tmparray13[] = {r1->outputs[0], r2->outputs[0]};
	jive_gamma(pred->outputs[0],
		2, tmparray11,
		tmparray12,
		tmparray13,
		gamma);
	jive_node * gamma_node = gamma[0]->node();
	const jive::base::type * tmparray14[] = {&type, &type, &type};
	jive::output * tmparray15[] = {gamma[0], gamma[1], top->outputs[0]};
	
	jive_node * bottom = jive_test_node_create(graph->root_region,
		3, tmparray14, tmparray15,
		0, NULL);
	
	jive_graph_pull_inward(graph);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	shape(shaped_graph, bottom);
	shape(shaped_graph, gamma_node);
	shape(shaped_graph, gamma_node->producer(0));
	shape(shaped_graph, l1);
	shape(shaped_graph, l2);
	shape(shaped_graph, gamma_node->producer(1));
	shape(shaped_graph, r1);
	jive_shaped_node * p = shape(shaped_graph, r2);
	shape(shaped_graph, pred);
	shape(shaped_graph, top);
	
	jive_ssavar * orig_ssavar = top->outputs[0]->ssavar;
	assert(orig_ssavar);
	jive_variable * var = orig_ssavar->variable;
	assert(bottom->inputs[2]->ssavar == orig_ssavar);
	assert(l1->inputs[0]->ssavar == orig_ssavar);
	assert(l2->inputs[0]->ssavar == orig_ssavar);
	assert(r1->inputs[0]->ssavar == orig_ssavar);
	assert(r2->inputs[0]->ssavar == orig_ssavar);
	
	jive_ssavar * ssavar_p = jive_regalloc_reroute_at_point(orig_ssavar, p);
	assert(ssavar_p == orig_ssavar);
	
	jive::output * new_orig = bottom->inputs[2]->origin();
	jive::gate * reroute_gate = new_orig->gate;
	assert(reroute_gate->variable == var);
	assert(new_orig->node() == gamma_node && reroute_gate);
	
	jive_ssavar * ssavar_below = new_orig->ssavar;
	assert(ssavar_below->variable == var);
	assert(ssavar_below != orig_ssavar && bottom->inputs[2]->ssavar == ssavar_below);
	
	jive::input * in_l = jive_node_get_gate_input(gamma_node->producer(0), reroute_gate);
	assert(in_l->ssavar == orig_ssavar);
	jive::input * in_r = jive_node_get_gate_input(gamma_node->producer(1), reroute_gate);
	assert(in_r->ssavar == orig_ssavar);
	
	assert(l1->inputs[0]->ssavar == orig_ssavar);
	assert(l2->inputs[0]->ssavar == orig_ssavar);
	assert(r1->inputs[0]->ssavar == orig_ssavar);
	assert(r2->inputs[0]->ssavar == orig_ssavar);
	assert(top->outputs[0]->ssavar == orig_ssavar);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-reroute-gamma1", test_main);
