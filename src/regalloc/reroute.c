/*
 * Copyright 2010 2011 2012 2013 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/reroute.h>

#include <stdio.h>

#include <jive/common.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/theta.h>
#include <jive/vsdg/variable.h>

static jive_ssavar *
reroute_gamma(jive_shaped_graph * shaped_graph,
	jive_ssavar * ssavar,
	const std::vector<jive::input*> & users_below,
	jive_node * anchor_node,
	jive_region * interest_region)
{
	/* if nothing below gamma anchor, then nothing to do */
	if (!users_below.size()) {
		return ssavar;
	}
	jive_graph * graph = ssavar->origin->node()->graph;
	jive::output * origin = ssavar->origin;
	jive_variable * variable = ssavar->variable;
	jive_ssavar * ssavar_inside_region = ssavar;
	
	const jive::base::type * type = &ssavar->origin->type();
	char gate_name[80];
	snprintf(gate_name, sizeof(gate_name), "route_%p_%p", ssavar, anchor_node);
	jive::gate * gate = jive_graph_create_gate(graph, gate_name, *type);
	
	jive_region * region1 = anchor_node->producer(0)->region;
	jive_region * region2 = anchor_node->producer(1)->region;
	std::vector<jive::input*> users1, users2;
	jive::input * in1 = jive_node_gate_input(anchor_node->producer(0), gate,
		ssavar->origin);
	jive::input * in2 = jive_node_gate_input(anchor_node->producer(1), gate,
		ssavar->origin);
	users1.push_back(in1);
	users2.push_back(in2);
	jive::output * out = jive_node_gate_output(anchor_node, gate);
	
	jive::input * user, * next;
	JIVE_LIST_ITERATE_SAFE(ssavar->assigned_inputs, user, next, ssavar_input_list) {
		if (jive_region_contains_node(region1, user->node())) {
			jive_ssavar_unassign_input(ssavar, user);
			users1.push_back(user);
		} else if (jive_region_contains_node(region2, user->node())) {
			jive_ssavar_unassign_input(ssavar, user);
			users2.push_back(user);
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
	jive_ssavar_assign_output(ssavar_below, out);
	for (size_t n = 0; n < users_below.size(); ++n) {
		jive::input * input = users_below[n];
		input->divert_origin(out);
		jive_ssavar_assign_input(ssavar_below, input);
	}
	for (size_t n = 0; n < users1.size(); ++n) {
		jive::input * input = users1[n];
		jive_ssavar_assign_input(ssavar1, input);
	}
	for (size_t n = 0; n < users2.size(); ++n) {
		jive::input * input = users2[n];
		jive_ssavar_assign_input(ssavar2, input);
	}
	
	jive_shaped_ssavar * shaped_ssavar1 = shaped_graph->map_ssavar(ssavar1);
	jive_shaped_ssavar * shaped_ssavar2 = shaped_graph->map_ssavar(ssavar2);
	if (shaped_ssavar1) {
		shaped_ssavar1->lower_boundary_region_depth(region1->depth());
	}
	if (shaped_ssavar2) {
		shaped_ssavar2->lower_boundary_region_depth(region2->depth());
	}

	if (region1 == interest_region) {
		ssavar_inside_region = ssavar1;
	} else if (region2 == interest_region) {
		ssavar_inside_region = ssavar2;
	} else {
		JIVE_DEBUG_ASSERT(false);
	}

	jive_variable_assign_gate(variable, gate);
	
	return ssavar_inside_region;
}

static jive_ssavar *
reroute_theta(jive_shaped_graph * shaped_graph,
	jive_ssavar * ssavar,
	const std::vector<jive::input*> users_below,
	jive_node * anchor_node,
	jive_region * interest_region)
{
	jive_graph * graph = ssavar->origin->node()->graph;
	jive::output * origin = ssavar->origin;
	jive_variable * variable = ssavar->variable;
	
	const jive::base::type * type = &ssavar->origin->type();
	char gate_name[80];
	snprintf(gate_name, sizeof(gate_name), "route_%p_%p", ssavar, anchor_node);
	jive::gate * gate = jive_graph_create_gate(graph, gate_name, *type);
	
	jive_region * loop_region = anchor_node->producer(0)->region;
	jive_node * loop_head = loop_region->top;
	jive_node * loop_tail = loop_region->bottom;
	
	std::vector<jive::input*> loop_users;
	jive::input * user, * next;
	JIVE_LIST_ITERATE(origin->users, user, output_users_list) {
		if (jive_region_contains_node(loop_region, user->node())) {
			if (user->ssavar == ssavar) {
				jive_ssavar_unassign_input(ssavar, user);
				loop_users.push_back(user);
			}
		}
	}
	
	jive::input * into_loop = jive_node_gate_input(loop_head, gate, ssavar->origin);
	jive::output * def_inside = jive_node_gate_output(loop_head, gate);
	loop_users.push_back(jive_node_gate_input(loop_tail, gate, def_inside));
	jive::output * def_outside = jive_node_gate_output(anchor_node, gate);
	
	/* everything has been cleared, now redo assignment */
	
	/* loop interior */
	
	jive_ssavar * ssavar_inside = jive_ssavar_create(def_inside, variable);
	/* assign ssavar to loop entry if it has been shaped already */
	if (shaped_graph->is_node_placed(loop_head)) {
		jive_ssavar_assign_input(ssavar, into_loop);
		jive_ssavar_assign_output(ssavar_inside, def_inside);
	}
	
	/* all loop users must use new definition from inside loop ... */
	JIVE_LIST_ITERATE_SAFE(origin->users, user, next, output_users_list) {
		if (jive_region_contains_node(loop_region, user->node()) && user != into_loop) {
			user->divert_origin(def_inside);
		}
	}
	/* ... but assign ssavar only to those that had an ssavar
	assigned previously */
	for (size_t n = 0; n < loop_users.size(); ++n) {
		jive::input * input = loop_users[n];
		jive_ssavar_assign_input(ssavar_inside, input);
	}
	shaped_graph->map_ssavar(ssavar_inside)->lower_boundary_region_depth(loop_region->depth());
	
	/* below loop */
	
	jive_ssavar * ssavar_below = jive_ssavar_create(def_outside, variable);
	jive_ssavar_assign_output(ssavar_below, def_outside);
	for (size_t n = 0; n < users_below.size(); ++n) {
		jive::input * input = users_below[n];
		input->divert_origin(def_outside);
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
	jive_node * anchor_node = anchor_location->node();
	
	/* FIXME: just returning appears correct and hinges on the fact that
	other anchor node types (subroutine etc.) simply do not allow anything
	to be passed through */
	if (!dynamic_cast<const jive::gamma_op *>(&anchor_node->operation()) && !
		dynamic_cast<const jive::theta_op *>(&anchor_node->operation())) {
		return ssavar;
	}
	
	std::vector<jive::input*> users_below;
	
	/* collect and disconnect all users below anchor */
	for (jive_shaped_node & loc : anchor_location->range_to_end()) {
		jive_node * node = loc.node();
		for (size_t n = 0; n < node->ninputs; n++) {
			jive::input * input = node->inputs[n];
			if (input->ssavar == ssavar) {
				jive_ssavar_unassign_input(ssavar, input);
				users_below.push_back(input);
			}
		}
	}
	jive_ssavar * ssavar_inside_region;
	
	if (dynamic_cast<const jive::gamma_op *>(&anchor_node->operation())) {
		ssavar_inside_region = reroute_gamma(shaped_graph, ssavar, users_below, anchor_node,
			interest_region);
	} else {
		ssavar_inside_region = reroute_theta(shaped_graph, ssavar, users_below, anchor_node,
			interest_region);
	}

	return ssavar_inside_region;
}

static jive_ssavar *
reroute_out_of_region(jive_shaped_graph * shaped_graph, jive_ssavar * ssavar, jive_region * region)
{
	if (ssavar->origin->node()->region != region) {
		ssavar = reroute_out_of_region(shaped_graph, ssavar, region->parent);
	}
	
	if (!region->anchor) {
		return ssavar;
	}
	
	jive_node * anchor_node = region->anchor->node();
	jive_shaped_node * anchor_location = shaped_graph->map_node(anchor_node);
	ssavar = reroute_through_anchor(shaped_graph, ssavar, anchor_location, region);
	/* FIXME: this should be the ssavar as used inside the region */
	return ssavar;
}

jive_ssavar *
jive_regalloc_reroute_at_point(jive_ssavar * ssavar, jive_shaped_node * shaped_node)
{
	jive_region * region = shaped_node->cut()->shaped_region().region();
	if (ssavar->origin->node()->region != region) {
		return reroute_out_of_region(&shaped_node->shaped_graph(), ssavar, region);
	} else {
		return ssavar;
	}
}
