/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/reroute.h>

#include <stdio.h>

#include <jive/common.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/util/vector.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/theta.h>
#include <jive/vsdg/variable.h>

typedef struct jive_input_vector jive_input_vector;
JIVE_DECLARE_VECTOR_TYPE(jive_input_vector, jive_input *)
JIVE_DEFINE_VECTOR_TYPE(jive_input_vector, jive_input *)

static jive_ssavar *
reroute_gamma(jive_shaped_graph * shaped_graph,
	jive_ssavar * ssavar,
	const jive_input_vector * users_below,
	jive_node * anchor_node,
	jive_region * interest_region)
{
	/* if nothing below gamma anchor, then nothing to do */
	if (!jive_input_vector_size(users_below)) {
		return ssavar;
	}
	jive_graph * graph = ssavar->origin->node->graph;
	jive_output * origin = ssavar->origin;
	jive_variable * variable = ssavar->variable;
	jive_ssavar * ssavar_inside_region = ssavar;
	
	const jive_type * type = jive_output_get_type(ssavar->origin);
	char gate_name[80];
	snprintf(gate_name, sizeof(gate_name), "route_%p_%p", ssavar, anchor_node);
	jive_gate * gate = jive_type_create_gate(type, graph, gate_name);
	
	jive_region * region1 = anchor_node->inputs[0]->origin->node->region;
	jive_region * region2 = anchor_node->inputs[1]->origin->node->region;
	jive_input_vector users1, users2;
	jive_input_vector_init(&users1);
	jive_input_vector_init(&users2);
	jive_input * in1 = jive_node_gate_input(anchor_node->inputs[0]->origin->node, gate, ssavar->origin);
	jive_input * in2 = jive_node_gate_input(anchor_node->inputs[1]->origin->node, gate, ssavar->origin);
	jive_input_vector_push_back(&users1, graph->context, in1);
	jive_input_vector_push_back(&users2, graph->context, in2);
	jive_output * out = jive_node_gate_output(anchor_node, gate);
	
	jive_input * user, * next;
	JIVE_LIST_ITERATE_SAFE(ssavar->assigned_inputs, user, next, ssavar_input_list) {
		if (jive_region_contains_node(region1, user->node)) {
			jive_ssavar_unassign_input(ssavar, user);
			jive_input_vector_push_back(&users1, graph->context, user);
		} else if (jive_region_contains_node(region2, user->node)) {
			jive_ssavar_unassign_input(ssavar, user);
			jive_input_vector_push_back(&users2, graph->context, user);
		}
	}
	jive_ssavar * ssavar_below = jive_ssavar_create(out, variable);
	jive_ssavar * ssavar1;
	jive_ssavar * ssavar2;
	
	/* if vars from the two regions have been merged into the
	parent already, then must use the already merged ssavar for
	both regions -- otherwise must create individual ssavars
	
	this check is slightly imprecise -- if there are inputs or
	outputs still assigned to the ssavar (these must be above
	the anchor), then the regions are finished and have been
	merged; the converse could conceivably be false, but
	reroute will not be called in this corner case */
	if (ssavar->assigned_inputs.first || ssavar->assigned_output) {
		ssavar1 = ssavar;
		ssavar2 = ssavar;
	} else {
		ssavar1 = jive_ssavar_create(origin, variable);
		ssavar2 = jive_ssavar_create(origin, variable);
	}
	
	/* everything has been cleared, now redo assignment */
	size_t n;
	jive_ssavar_assign_output(ssavar_below, out);
	for (n = 0; n < jive_input_vector_size(users_below); ++n) {
		jive_input * input = jive_input_vector_item(users_below, n);
		jive_input_divert_origin(input, out);
		jive_ssavar_assign_input(ssavar_below, input);
	}
	for (n = 0; n < jive_input_vector_size(&users1); ++n) {
		jive_input * input = jive_input_vector_item(&users1, n);
		jive_ssavar_assign_input(ssavar1, input);
	}
	for (n = 0; n < jive_input_vector_size(&users2); ++n) {
		jive_input * input = jive_input_vector_item(&users2, n);
		jive_ssavar_assign_input(ssavar2, input);
	}
	
	jive_input_vector_fini(&users2, graph->context);
	jive_input_vector_fini(&users1, graph->context);
	
	jive_shaped_ssavar * shaped_ssavar1 = jive_shaped_graph_map_ssavar(shaped_graph, ssavar1);
	jive_shaped_ssavar * shaped_ssavar2 = jive_shaped_graph_map_ssavar(shaped_graph, ssavar2);
	if (shaped_ssavar1)
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar1, region1->depth);
	if (shaped_ssavar2)
		jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar2, region2->depth);
	
	if (region1 == interest_region)
		ssavar_inside_region = ssavar1;
	else if (region2 == interest_region)
		ssavar_inside_region = ssavar2;
	else
		JIVE_DEBUG_ASSERT(false);
	
	jive_variable_assign_gate(variable, gate);
	
	return ssavar_inside_region;
}

static jive_ssavar *
reroute_theta(jive_shaped_graph * shaped_graph,
	jive_ssavar * ssavar,
	const jive_input_vector * users_below,
	jive_node * anchor_node,
	jive_region * interest_region)
{
	jive_graph * graph = ssavar->origin->node->graph;
	jive_output * origin = ssavar->origin;
	jive_variable * variable = ssavar->variable;
	
	const jive_type * type = jive_output_get_type(ssavar->origin);
	char gate_name[80];
	snprintf(gate_name, sizeof(gate_name), "route_%p_%p", ssavar, anchor_node);
	jive_gate * gate = jive_type_create_gate(type, graph, gate_name);
	
	jive_region * loop_region = anchor_node->inputs[0]->origin->node->region;
	jive_node * loop_head = loop_region->top;
	jive_node * loop_tail = loop_region->bottom;
	
	jive_input_vector loop_users;
	jive_input_vector_init(&loop_users);
	jive_input * user, * next;
	JIVE_LIST_ITERATE(origin->users, user, output_users_list) {
		if (jive_region_contains_node(loop_region, user->node)) {
			if (user->ssavar == ssavar) {
				jive_ssavar_unassign_input(ssavar, user);
				jive_input_vector_push_back(&loop_users, graph->context, user);
			}
		}
	}
	
	jive_input * into_loop = jive_node_gate_input(loop_head, gate, ssavar->origin);
	jive_output * def_inside = jive_node_gate_output(loop_head, gate);
	jive_input_vector_push_back(&loop_users, graph->context,
		jive_node_gate_input(loop_tail, gate, def_inside));
	jive_output * def_outside = jive_node_gate_output(anchor_node, gate);
	
	/* everything has been cleared, now redo assignment */
	
	/* loop interior */
	
	jive_ssavar * ssavar_inside = jive_ssavar_create(def_inside, variable);
	/* assign ssavar to loop entry if it has been shaped already */
	if (jive_shaped_graph_map_node(shaped_graph, loop_head)) {
		jive_ssavar_assign_input(ssavar, into_loop);
		jive_ssavar_assign_output(ssavar_inside, def_inside);
	}
	
	/* all loop users must use new definition from inside loop ... */
	JIVE_LIST_ITERATE_SAFE(origin->users, user, next, output_users_list) {
		if (jive_region_contains_node(loop_region, user->node) && user != into_loop) {
			jive_input_divert_origin(user, def_inside);
		}
	}
	/* ... but assign ssavar only to those that had an ssavar
	assigned previously */
	size_t n;
	for (n = 0; n < jive_input_vector_size(&loop_users); ++n) {
		jive_input * input = jive_input_vector_item(&loop_users, n);
		jive_ssavar_assign_input(ssavar_inside, input);
	}
	jive_input_vector_fini(&loop_users, graph->context);
	jive_shaped_ssavar * shaped_ssavar_inside = jive_shaped_graph_map_ssavar(shaped_graph, ssavar_inside);
	jive_shaped_ssavar_lower_boundary_region_depth(shaped_ssavar_inside, loop_region->depth);
	
	/* below loop */
	
	jive_ssavar * ssavar_below = jive_ssavar_create(def_outside, variable);
	jive_ssavar_assign_output(ssavar_below, def_outside);
	for (n = 0; n < jive_input_vector_size(users_below); ++n) {
		jive_input * input = jive_input_vector_item(users_below, n);
		jive_input_divert_origin(input, def_outside);
		jive_ssavar_assign_input(ssavar_below, input);
	}
	
	/* gate */
	
	jive_variable_assign_gate(variable, gate);
	
	return ssavar_inside;
}

static jive_ssavar *
reroute_through_anchor(jive_shaped_graph * shaped_graph,
	jive_ssavar * ssavar,
	jive_shaped_node * anchor_location,
	jive_region * interest_region)
{
	jive_graph * graph = ssavar->origin->node->graph;
	jive_context * context = graph->context;
	jive_node * anchor_node = anchor_location->node;
	
	/* FIXME: just returning appears correct and hinges on the fact that
	other anchor node types (subroutine etc.) simply do not allow anything
	to be passed through */
	if (!jive_node_isinstance(anchor_node, &JIVE_GAMMA_NODE) && !jive_node_isinstance(anchor_node, &JIVE_THETA_NODE))
		return ssavar;
	JIVE_DEBUG_ASSERT(jive_node_isinstance(anchor_node, &JIVE_GAMMA_NODE) || jive_node_isinstance(anchor_node, &JIVE_THETA_NODE));
	
	jive_input_vector users_below;
	jive_input_vector_init(&users_below);
	
	/* collect and disconnect all users below anchor */
	jive_shaped_node_downward_iterator i;
	jive_shaped_node_downward_iterator_init(&i, anchor_location);
	for(;;) {
		jive_shaped_node * loc = jive_shaped_node_downward_iterator_next(&i);
		if (!loc)
			break;
		jive_node * node = loc->node;
		size_t n;
		for(n = 0; n < node->ninputs; n++) {
			jive_input * input = node->inputs[n];
			if (input->ssavar == ssavar) {
				jive_ssavar_unassign_input(ssavar, input);
				jive_input_vector_push_back(&users_below, context, input);
			}
		}
	}
	jive_shaped_node_downward_iterator_fini(&i);
	
	jive_ssavar * ssavar_inside_region;
	
	if (jive_node_isinstance(anchor_node, &JIVE_GAMMA_NODE)) {
		ssavar_inside_region = reroute_gamma(shaped_graph, ssavar, &users_below, anchor_node, interest_region);
	} else {
		ssavar_inside_region = reroute_theta(shaped_graph, ssavar, &users_below, anchor_node, interest_region);
	}
	
	jive_input_vector_fini(&users_below, context);
	
	return ssavar_inside_region;
}

static jive_ssavar *
reroute_out_of_region(jive_shaped_graph * shaped_graph, jive_ssavar * ssavar, jive_region * region)
{
	if (ssavar->origin->node->region != region)
		ssavar = reroute_out_of_region(shaped_graph, ssavar, region->parent);
	
	if (!region->anchor)
		return ssavar;
	
	jive_node * anchor_node = region->anchor->node;
	jive_shaped_node * anchor_location = jive_shaped_graph_map_node(shaped_graph, anchor_node);
	ssavar = reroute_through_anchor(shaped_graph, ssavar, anchor_location, region);
	/* FIXME: this should be the ssavar as used inside the region */
	return ssavar;
}

jive_ssavar *
jive_regalloc_reroute_at_point(jive_ssavar * ssavar, jive_shaped_node * shaped_node)
{
	jive_shaped_graph * shaped_graph = shaped_node->shaped_graph;
	jive_shaped_region * shaped_region = shaped_node->cut->shaped_region;
	if (ssavar->origin->node->region != shaped_region->region)
		return reroute_out_of_region(shaped_graph, ssavar, shaped_region->region);
	else
		return ssavar;
}
