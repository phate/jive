/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/variable.h>

#include <jive/common.h>
#include <jive/util/list.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/region-private.h>
#include <jive/vsdg/resource.h>
#include <jive/vsdg/theta.h>

static inline void
jive_variable_inc_use_count(jive_variable * self)
{
	self->use_count = self->use_count + 1;
	if (self->use_count == 1) {
		JIVE_LIST_REMOVE(self->graph->unused_variables, self, graph_variable_list);
		JIVE_LIST_PUSH_BACK(self->graph->variables, self, graph_variable_list);
		jive_graph_notify_variable_create(self->graph, self);
	}
}

static inline void
jive_variable_dec_use_count(jive_variable * self)
{
	self->use_count = self->use_count - 1;
	if (self->use_count == 0) {
		jive_graph_notify_variable_destroy(self->graph, self);
		JIVE_LIST_REMOVE(self->graph->variables, self, graph_variable_list);
		JIVE_LIST_PUSH_BACK(self->graph->unused_variables, self, graph_variable_list);
	}
}

static inline void
jive_ssavar_inc_use_count(jive_ssavar * self)
{
	self->use_count = self->use_count + 1;
	if (self->use_count == 1) {
		JIVE_LIST_REMOVE(self->variable->unused_ssavars, self, variable_ssavar_list);
		JIVE_LIST_PUSH_BACK(self->variable->ssavars, self, variable_ssavar_list);
		JIVE_LIST_PUSH_BACK(self->origin->originating_ssavars, self, originating_ssavar_list);
		
		jive_variable_inc_use_count(self->variable);
		
		jive_graph_notify_ssavar_create(self->variable->graph, self);
	}
}

static inline void
jive_ssavar_dec_use_count(jive_ssavar * self)
{
	self->use_count = self->use_count - 1;
	if (self->use_count == 0) {
		jive_graph_notify_ssavar_destroy(self->variable->graph, self);
		
		jive_variable_dec_use_count(self->variable);
		
		JIVE_LIST_REMOVE(self->origin->originating_ssavars, self, originating_ssavar_list);
		JIVE_LIST_REMOVE(self->variable->ssavars, self, variable_ssavar_list);
		JIVE_LIST_PUSH_BACK(self->variable->unused_ssavars, self, variable_ssavar_list);
	}
}

jive_ssavar *
jive_ssavar_create(jive::output * origin, jive_variable * variable)
{
	jive_ssavar * self = new jive_ssavar;
	
	self->use_count = 0;
	self->variable = variable;
	self->variable_ssavar_list.prev = self->variable_ssavar_list.next = 0;
	self->origin = origin;
	self->assigned_inputs.first = self->assigned_inputs.last = 0;
	self->assigned_output = 0;
	
	JIVE_LIST_PUSH_BACK(variable->unused_ssavars, self, variable_ssavar_list);
	
	return self;
}

void
jive_ssavar_destroy(jive_ssavar * self)
{
	if (self->assigned_output)
		jive_ssavar_unassign_output(self, self->assigned_output);
	while (self->assigned_inputs.first)
		jive_ssavar_unassign_input(self, self->assigned_inputs.first);
	JIVE_DEBUG_ASSERT(self->use_count == 0);
	JIVE_LIST_REMOVE(self->variable->unused_ssavars, self, variable_ssavar_list);
	delete self;
}

void
jive_ssavar_assert_consistent(const jive_ssavar * self)
{
	jive::output * origin = self->origin;
	if (!origin)
		return;
	
	jive::input * input;
	JIVE_LIST_ITERATE(origin->users, input, output_users_list) {
		JIVE_DEBUG_ASSERT(input->ssavar == origin->ssavar);
	}
}

void
jive_ssavar_assign_input(jive_ssavar * self, jive::input * input)
{
	JIVE_DEBUG_ASSERT(input->origin() == self->origin && input->ssavar == 0);

	jive_ssavar_inc_use_count(self);
	if (jive::theta_head_op() == input->node()->operation()) {
		jive_region_add_used_ssavar(input->node()->region->parent, self);
	} else {
		jive_region_add_used_ssavar(input->node()->region, self);
	}

	JIVE_LIST_PUSH_BACK(self->assigned_inputs, input, ssavar_input_list);
	input->ssavar = self;

	jive_graph_notify_ssavar_assign_input(self->variable->graph, self, input);
}

void
jive_ssavar_unassign_input(jive_ssavar * self, jive::input * input)
{
	JIVE_DEBUG_ASSERT(input->ssavar == self);

	jive_graph_notify_ssavar_unassign_input(self->variable->graph, self, input);

	JIVE_LIST_REMOVE(self->assigned_inputs, input, ssavar_input_list);
	input->ssavar = 0;
	if (jive::theta_head_op() == input->node()->operation()) {
		jive_region_remove_used_ssavar(input->node()->region->parent, self);
	} else {
		jive_region_remove_used_ssavar(input->node()->region, self);
	}

	jive_ssavar_dec_use_count(self);
}

void
jive_ssavar_assign_output(jive_ssavar * self, jive::output * output)
{
	JIVE_DEBUG_ASSERT( (self->origin == output) && (output->ssavar == 0) );
	
	jive_ssavar_inc_use_count(self);
	self->assigned_output = output;
	output->ssavar = self;
	jive_graph_notify_ssavar_assign_output(self->variable->graph, self, output);
}

void
jive_ssavar_unassign_output(jive_ssavar * self, jive::output * output)
{
	JIVE_DEBUG_ASSERT(output->ssavar == self && self->assigned_output == output);
	
	jive_graph_notify_ssavar_unassign_output(self->variable->graph, self, output);
	output->ssavar = 0;
	self->assigned_output = 0;
	jive_ssavar_dec_use_count(self);
}

void
jive_ssavar_merge(jive_ssavar * self, jive_ssavar * other)
{
	JIVE_DEBUG_ASSERT(self->variable == other->variable);
	JIVE_DEBUG_ASSERT(self->origin == other->origin);
	
	jive::output * assigned_output = other->assigned_output;
	struct {
		jive::input * first;
		jive::input * last;
	} assigned_inputs = {0, 0};
	
	/* undo all assignments, then redo assignments: there must never
	be a state when "self" and "other" are alive at the same time,
	but doing "piecemeal" reassignment might cause this */
	if (assigned_output)
		jive_ssavar_unassign_output(other, assigned_output);
	while (other->assigned_inputs.first) {
		jive::input * input = other->assigned_inputs.first;
		jive_ssavar_unassign_input(other, input);
		JIVE_LIST_PUSH_BACK(assigned_inputs, input, ssavar_input_list);
	}
	
	if (assigned_output)
		jive_ssavar_assign_output(self, assigned_output);
	while (assigned_inputs.first) {
		jive::input * input = assigned_inputs.first;
		JIVE_LIST_REMOVE(assigned_inputs, input, ssavar_input_list);
		jive_ssavar_assign_input(self, input);
	}
}

void
jive_ssavar_divert_origin(jive_ssavar * self, jive::output * new_origin)
{
	JIVE_DEBUG_ASSERT(new_origin->ssavar == 0);
	
	jive::output * old_origin = self->origin;
	self->origin = new_origin;
	
	if (self->assigned_output) {
		self->assigned_output->ssavar = 0;
		self->assigned_output = new_origin;
		self->assigned_output->ssavar = self;
	}
	
	jive::input * input;
	JIVE_LIST_ITERATE(self->assigned_inputs, input, ssavar_input_list) {
		input->internal_divert_origin(new_origin);
	}
	
	if (self->use_count) {
		JIVE_LIST_REMOVE(old_origin->originating_ssavars, self, originating_ssavar_list);
		JIVE_LIST_PUSH_BACK(new_origin->originating_ssavars, self, originating_ssavar_list);
	}
		
	jive_graph_notify_ssavar_divert_origin(self->variable->graph, self, old_origin, new_origin);
}

void
jive_ssavar_split(jive_ssavar * self)
{
	/* split off this SSA variable from others
	assigned to the same variable */
	jive_variable * old_variable = self->variable;
	jive_variable * new_variable = jive_variable_create(self->variable->graph);
	jive_variable_set_resource_class(new_variable, jive_variable_get_resource_class(self->variable));
	jive_variable_set_resource_name(new_variable, jive_variable_get_resource_name(self->variable));
		
	if (self->use_count == 0) {
		self->variable = new_variable;
		JIVE_LIST_REMOVE(old_variable->unused_ssavars, self, variable_ssavar_list);
		JIVE_LIST_PUSH_BACK(new_variable->unused_ssavars, self, variable_ssavar_list);
		return;
	}
	
	JIVE_LIST_REMOVE(old_variable->ssavars, self, variable_ssavar_list);
	self->variable = new_variable;
	JIVE_LIST_PUSH_BACK(new_variable->ssavars, self, variable_ssavar_list);
	
	jive_variable_inc_use_count(new_variable);
	jive_graph_notify_ssavar_variable_change(self->variable->graph, self, old_variable, new_variable);
	jive_variable_dec_use_count(old_variable);
}

jive_variable *
jive_variable_create(struct jive_graph * graph)
{
	jive_variable * self = new jive_variable;
	
	self->graph = graph;
	self->graph_variable_list.prev = self->graph_variable_list.next = 0;
	self->use_count = 0;
	
	self->ssavars.first = self->ssavars.last = 0;
	
	self->unused_ssavars.first = self->unused_ssavars.last = 0;
	
	self->gates.first = self->gates.last = 0;
	self->rescls = &jive_root_resource_class;
	self->resname = 0;
	
	JIVE_LIST_PUSH_BACK(graph->unused_variables, self, graph_variable_list);
	
	return self;
}

void
jive_variable_destroy(jive_variable * self)
{
	while(self->ssavars.first)
		jive_ssavar_destroy(self->ssavars.first);
	while(self->unused_ssavars.first)
		jive_ssavar_destroy(self->unused_ssavars.first);
	while(self->gates.first)
		jive_variable_unassign_gate(self, self->gates.first);
	
	JIVE_DEBUG_ASSERT(self->use_count == 0);
	JIVE_LIST_REMOVE(self->graph->unused_variables, self, graph_variable_list);
	
	delete self;
}

void
jive_variable_assign_gate(jive_variable * self, jive::gate * gate)
{
	JIVE_DEBUG_ASSERT(gate->variable == 0);
	jive_variable_inc_use_count(self);
	JIVE_LIST_PUSH_BACK(self->gates, gate, variable_gate_list);
	gate->variable = self;
	jive_graph_notify_variable_assign_gate(self->graph, self, gate);
}

void
jive_variable_unassign_gate(jive_variable * self, jive::gate * gate)
{
	JIVE_DEBUG_ASSERT(gate->variable == self);
	jive_graph_notify_variable_unassign_gate(self->graph, self, gate);
	JIVE_LIST_REMOVE(self->gates, gate, variable_gate_list);
	gate->variable = 0;
	jive_variable_dec_use_count(self);
}

void
jive_variable_merge(jive_variable * self, jive_variable * other)
{
	if ((other == 0) || (self == 0) || (other == self))
		return;
	
	const jive_resource_class * new_rescls;
	new_rescls = jive_resource_class_intersection( self->rescls, other->rescls);
	JIVE_DEBUG_ASSERT(new_rescls);
	
	jive_variable_set_resource_class(other, new_rescls);
	jive_variable_set_resource_class(self, new_rescls);
	
	JIVE_DEBUG_ASSERT(self->resname == 0 || other->resname == 0 || (self->resname == other->resname));
	if (other->resname && !self->resname)
		jive_variable_set_resource_name(self, other->resname);
	if (self->resname && !other->resname)
		jive_variable_set_resource_name(other, self->resname);
	
	while (other->gates.first) {
		jive::gate * gate = other->gates.first;
		jive_variable_unassign_gate(other, gate);
		jive_variable_assign_gate(self, gate);
	}
	
	while(other->ssavars.first) {
		jive_ssavar * ssavar = other->ssavars.first;
		
		jive_variable_inc_use_count(self);
		
		JIVE_DEBUG_ASSERT(ssavar->use_count != 0);
		JIVE_LIST_REMOVE(other->ssavars, ssavar, variable_ssavar_list);
		JIVE_LIST_PUSH_BACK(self->ssavars, ssavar, variable_ssavar_list);
		ssavar->variable = self;
		
		jive_graph_notify_ssavar_variable_change(self->graph, ssavar, other, self);
		
		jive_variable_dec_use_count(other);
	}
	
	while(other->unused_ssavars.first) {
		jive_ssavar * ssavar = other->unused_ssavars.first;
		
		JIVE_DEBUG_ASSERT(ssavar->use_count == 0);
		JIVE_LIST_REMOVE(other->unused_ssavars, ssavar, variable_ssavar_list);
		JIVE_LIST_PUSH_BACK(self->unused_ssavars, ssavar, variable_ssavar_list);
		ssavar->variable = self;
	}
}

void
jive_variable_set_resource_class(jive_variable * self, const jive_resource_class * new_rescls)
{
	const jive_resource_class * old_rescls = self->rescls;
	if (new_rescls == old_rescls)
		return;
	self->rescls = new_rescls;
	
	jive_graph_notify_variable_resource_class_change(self->graph, self, old_rescls, new_rescls);
}

void
jive_variable_recompute_rescls(jive_variable * self)
{
	const jive_resource_class * rescls = &jive_root_resource_class;
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(self->ssavars, ssavar, variable_ssavar_list) {
		jive::input * input;
		JIVE_LIST_ITERATE(ssavar->assigned_inputs, input, ssavar_input_list) {
			rescls = jive_resource_class_intersection(rescls, input->required_rescls);
		}
		if (ssavar->assigned_output)
			rescls = jive_resource_class_intersection(rescls, ssavar->assigned_output->required_rescls);
	}
	jive::gate * gate;
	JIVE_LIST_ITERATE(self->gates, gate, variable_gate_list) {
		rescls = jive_resource_class_intersection(rescls, gate->required_rescls);
	}
	
	jive_variable_set_resource_class(self, rescls);
}

void
jive_variable_set_resource_name(jive_variable * self, const jive_resource_name * new_resname)
{
	const jive_resource_name * old_resname = self->resname;
	if (old_resname == new_resname)
		return;
	
	jive_variable_set_resource_class(self, new_resname->resource_class);
	
	self->resname = new_resname;
	
	jive_graph_notify_variable_resource_name_change(self->graph, self, old_resname, new_resname);
}

bool
jive_variable_may_spill(const jive_variable * self)
{
	jive::gate * gate;
	JIVE_LIST_ITERATE(self->gates, gate, variable_gate_list) {
		if (!gate->may_spill)
			return false;
	}
	return true;
}
