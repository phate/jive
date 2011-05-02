#include <jive/regalloc/reroute.h>

#include <stdio.h>

#include <jive/common.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/variable.h>

static jive_ssavar *
reroute_through_anchor(jive_shaped_graph * shaped_graph, jive_ssavar * ssavar, jive_shaped_node * anchor_location)
{
	/* check if there are references to "origin" below "anchor_location" */
	bool found = false;
	jive_shaped_node_downward_iterator i;
	jive_shaped_node_downward_iterator_init(&i, anchor_location);
	for(;;) {
		jive_shaped_node * loc = jive_shaped_node_downward_iterator_next(&i);
		if (!loc) break;
		jive_node * user = loc->node;
		size_t n;
		for(n = 0; n < user->ninputs; n++) {
			jive_input * input = user->inputs[n];
			if (input->origin == ssavar->origin) {
				found = true;
				break;
			}
		}
	}
	jive_shaped_node_downward_iterator_fini(&i);
	if (!found)
		return ssavar;
	
	jive_graph * graph = ssavar->origin->node->graph;
	
	/* if yes, then reroute variable through subregion(s)
	attached to anchor, and use value passed into and
	out of region */
	
	jive_node * anchor_node = anchor_location->node;
	JIVE_DEBUG_ASSERT(jive_node_isinstance(anchor_node, &JIVE_GAMMA_NODE) || jive_node_isinstance(anchor_node, &JIVE_THETA_NODE));
	
	jive_output * origin_inside_region = ssavar->origin;
	const jive_type * type = jive_output_get_type(ssavar->origin);
	char gate_name[80];
	snprintf(gate_name, sizeof(gate_name), "route_%p_%p", ssavar, anchor_location);
	jive_gate * gate = jive_type_create_gate(type, graph, gate_name);
	
	if (jive_node_isinstance(anchor_node, &JIVE_THETA_NODE)) {
		/* FIXME: could check if it is passed through already */
		jive_region * region = anchor_node->inputs[0]->origin->node->region;
		jive_input * input_into_region = jive_node_gate_input(region->top, gate, ssavar->origin);
		
		origin_inside_region = jive_node_gate_output(region->top, gate);
		jive_input * user, * next;
		JIVE_LIST_ITERATE_SAFE(ssavar->origin->users, user, next, output_users_list) {
			if (!jive_region_contains_node(region, user->node))
				continue;
			if (user->node == region->top)
				continue;
			if (user->ssavar)
				jive_ssavar_unassign_input(ssavar, user);
			jive_input_divert_origin(user, origin_inside_region);
		}
		
		jive_input * input_endof_region = jive_node_gate_input(region->bottom, gate, origin_inside_region);
		(void) input_into_region;
		(void) input_endof_region;
	} else if (jive_node_isinstance(anchor_node, &JIVE_GAMMA_NODE)) {
		/* FIXME: could check if it is passed through already */
		jive_input * in_false = jive_node_gate_input(anchor_node->inputs[0]->origin->node, gate, ssavar->origin);
		jive_input * in_true = jive_node_gate_input(anchor_node->inputs[1]->origin->node, gate, ssavar->origin);
		(void) in_false;
		(void) in_true;
	} else {
		JIVE_DEBUG_ASSERT(false);
	}
	
	jive_output * new_origin = jive_node_gate_output(anchor_node, gate);
	
	/* undo variable assignment to nodes below to avoid conflicts due
	to variable class overflow while references to old/new
	origin may coexist */
	jive_shaped_node_downward_iterator_init(&i, anchor_location);
	for(;;) {
		jive_shaped_node * loc = jive_shaped_node_downward_iterator_next(&i);
		if (!loc) break;
		jive_node * user = loc->node;
		size_t n;
		for(n = 0; n < user->ninputs; n++) {
			jive_input * input = user->inputs[n];
			if (input->origin == ssavar->origin) {
				if (input->ssavar != ssavar)
					continue;
				jive_input_unassign_ssavar(input);
				jive_input_divert_origin(input, new_origin);
			}
		}
	}
	jive_shaped_node_downward_iterator_fini(&i);
	
	/* from now on, there cannot be references to this origin/variable
	below the anchor ... */
	JIVE_DEBUG_ASSERT( ! jive_shaped_variable_is_crossing(jive_shaped_graph_map_variable(shaped_graph, ssavar->variable), jive_shaped_graph_map_node(shaped_graph, anchor_node)) );
	/* ... therefore it is now safe to redo variable assignment */
	jive_variable_assign_gate(ssavar->variable, gate);
	
	jive_input * input;
	JIVE_LIST_ITERATE(gate->inputs, input, gate_inputs_list) {
		JIVE_DEBUG_ASSERT( (input->ssavar == NULL) || (input->ssavar->variable == ssavar->variable) );
		jive_input_auto_assign_variable(input);
	}
	jive_output * output;
	JIVE_LIST_ITERATE(gate->outputs, output, gate_outputs_list) {
		JIVE_DEBUG_ASSERT( (output->ssavar == NULL) || (output->ssavar->variable == ssavar->variable) );
		jive_output_auto_assign_variable(output);
	}
	
	if (origin_inside_region != ssavar->origin) {
		JIVE_DEBUG_ASSERT(origin_inside_region->ssavar->variable == ssavar->variable);
		jive_input * user;
		JIVE_LIST_ITERATE(origin_inside_region->users, user, output_users_list) {
			if (user->gate == gate)
				continue;
			jive_input_auto_assign_variable(user);
		}
	}
	
	JIVE_LIST_ITERATE(new_origin->users, input, output_users_list)
		jive_input_auto_assign_variable(input);
	
	return origin_inside_region->ssavar;
}

static jive_ssavar *
reroute_out_of_region(jive_shaped_graph * shaped_graph, jive_ssavar * ssavar, jive_region * region)
{
	if (ssavar->origin->node->region != region)
		ssavar = reroute_out_of_region(shaped_graph, ssavar, region->parent);
	
	jive_node * anchor_node = region->anchor->node;
	jive_shaped_node * anchor_location = jive_shaped_graph_map_node(shaped_graph, anchor_node);
	return reroute_through_anchor(shaped_graph, ssavar, anchor_location);
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
