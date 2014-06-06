/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include "test-registry.h"
#include "testtypes.h"

#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/theta.h>
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
		jive_ssavar * ssavar = jive_input_auto_merge_variable(node->inputs[n]);
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
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_test_value_type type;
	jive::ctl::type ctl;
	jive::achr::type anchor_type;
	const jive::base::type * tmparray0[] = {&type};
	
	jive_node * dummy = jive_test_node_create(graph->root_region,
		0, NULL, NULL,
		1, tmparray0);
	
	jive_region * r1 = jive_region_create_subregion(graph->root_region);
	const jive::base::type * tmparray1[] = {&type};
	const jive::base::type * tmparray2[] = {&type, &type};
	
	jive_node * top = jive_test_node_create(r1,
		1, tmparray1, dummy->outputs,
		2, tmparray2);
	
	jive_theta theta = jive_theta_begin(graph);
	jive_region * loop_region = theta.region;
	
	jive_theta_loopvar loopvar1 = jive_theta_loopvar_enter(theta, top->outputs[0]);
	const jive::base::type * tmparray3[] = {&type, &type};
	jive_output * tmparray4[] = {loopvar1.value, top->outputs[1]};
	const jive::base::type * tmparray5[] = {&ctl, &type};
	
	jive_node * theta_op = jive_test_node_create(loop_region,
		2, tmparray3, tmparray4,
		2, tmparray5);
	
	jive_theta_loopvar_leave(theta, loopvar1.gate, theta_op->outputs[1]);
	jive_node * theta_node = jive_theta_end(theta, theta_op->outputs[0],
		1, &loopvar1);
	jive_node * theta_head = loop_region->top;
	jive_node * theta_tail = loop_region->bottom;
	const jive::base::type * tmparray6[] = {&type, &type};
	jive_output * tmparray7[] = {loopvar1.value, top->outputs[1]};
	const jive::base::type * tmparray8[] = {&anchor_type};
	
	jive_node * bottom = jive_test_node_create(r1,
		2, tmparray6, tmparray7,
		1, tmparray8);
	const jive::base::type * tmparray9[] = {&anchor_type};
	
	jive_node * subroutine_anchor = jive_test_node_create(graph->root_region,
		1, tmparray9, bottom->outputs,
		0, NULL);
	
	jive_shaped_graph * shaped_graph = jive_shaped_graph_create(graph);
	
	shape(shaped_graph, subroutine_anchor);
	shape(shaped_graph, bottom);
	shape(shaped_graph, theta_node);
	jive_shaped_node * p = shape(shaped_graph, theta_tail);
	
	jive_view(graph, stdout);
	
	jive_ssavar * orig_ssavar = bottom->inputs[1]->ssavar;
	jive_variable * var = orig_ssavar->variable;
	
	jive_ssavar * ssavar_p = jive_regalloc_reroute_at_point(bottom->inputs[1]->ssavar, p);
	assert(ssavar_p != orig_ssavar);
	assert(orig_ssavar->variable == var);
	assert(ssavar_p->variable == var);
	
	assert(theta_head->ninputs == 2 && theta_head->noutputs == 3);
	assert(theta_tail->ninputs == 3);
	assert(theta_node->noutputs == 2);
	
	jive_gate * gate = theta_head->inputs[1]->gate;
	assert(gate);
	assert(theta_head->inputs[1]->gate == gate && theta_head->outputs[2]->gate == gate);
	assert(theta_tail->inputs[2]->gate == gate);
	assert(theta_node->outputs[1]->gate == gate);
	
	assert(theta_head->inputs[1]->origin() == top->outputs[1]);
	assert(theta_tail->inputs[2]->origin() == theta_head->outputs[2]);
	assert(bottom->inputs[1]->origin() == theta_node->outputs[1]);
	
	assert(top->outputs[1]->ssavar == NULL);
	assert(theta_head->inputs[1]->ssavar == NULL);
	assert(theta_head->outputs[2]->ssavar == NULL);
	assert(theta_tail->inputs[2]->ssavar == ssavar_p);
	assert(theta_op->inputs[1]->origin() == theta_head->outputs[2]);
	assert(theta_op->inputs[1]->ssavar == NULL);
	
	jive_ssavar * ssavar_below = bottom->inputs[1]->ssavar;
	assert(theta_node->outputs[1]->ssavar == ssavar_below);
	assert(bottom->inputs[1]->ssavar == ssavar_below);
	
	jive_view(graph, stdout);
	
	jive_shaped_graph_destroy(shaped_graph);
	
	jive_graph_destroy(graph);
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	return 0;
}

JIVE_UNIT_TEST_REGISTER("shape/test-reroute-theta2", test_main);
