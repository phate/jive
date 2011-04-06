#include <jive/vsdg/variable.h>

#include <jive/util/list.h>
#include <jive/vsdg/basetype-private.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/region-private.h>
#include <jive/vsdg/region-ssavar-use-private.h>
#include <jive/vsdg/resource.h>

#include "debug.h"

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
jive_ssavar_create(jive_output * origin, jive_variable * variable)
{
	jive_context * context = variable->graph->context;
	
	jive_ssavar * self = jive_context_malloc(context, sizeof(*self));
	
	self->use_count = 0;
	self->variable = variable;
	self->variable_ssavar_list.prev = self->variable_ssavar_list.next = 0;
	self->origin = origin;
	self->assigned_inputs.first = self->assigned_inputs.last = 0;
	self->assigned_output = 0;
	jive_ssavar_region_hash_init(&self->assigned_regions, context);
	
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
	DEBUG_ASSERT(self->use_count == 0);
	jive_ssavar_region_hash_fini(&self->assigned_regions);
	JIVE_LIST_REMOVE(self->variable->unused_ssavars, self, variable_ssavar_list);
	jive_context_free(self->variable->graph->context, self);
}

void
jive_ssavar_assign_input(jive_ssavar * self, jive_input * input)
{
	DEBUG_ASSERT(input->origin == self->origin && input->ssavar == 0);
	
	jive_ssavar_inc_use_count(self);
	if (jive_node_isinstance(input->node, &JIVE_THETA_HEAD_NODE))
		jive_region_add_used_ssavar(input->node->region->parent, self);
	else
		jive_region_add_used_ssavar(input->node->region, self);
	
	JIVE_LIST_PUSH_BACK(self->assigned_inputs, input, ssavar_input_list);
	input->ssavar = self;
	
	jive_graph_notify_ssavar_assign_input(self->variable->graph, self, input);
}

void
jive_ssavar_unassign_input(jive_ssavar * self, jive_input * input)
{
	DEBUG_ASSERT(input->ssavar == self);
	
	jive_graph_notify_ssavar_unassign_input(self->variable->graph, self, input);
	
	JIVE_LIST_REMOVE(self->assigned_inputs, input, ssavar_input_list);
	input->ssavar = 0;
	if (jive_node_isinstance(input->node, &JIVE_THETA_HEAD_NODE))
		jive_region_remove_used_ssavar(input->node->region->parent, self);
	else
		jive_region_remove_used_ssavar(input->node->region, self);
	
	jive_ssavar_dec_use_count(self);
}

void
jive_ssavar_assign_output(jive_ssavar * self, jive_output * output)
{
	DEBUG_ASSERT( (self->origin == output) && (output->ssavar == 0) );
	
	jive_ssavar_inc_use_count(self);
	self->assigned_output = output;
	output->ssavar = self;
	jive_graph_notify_ssavar_assign_output(self->variable->graph, self, output);
}

void
jive_ssavar_unassign_output(jive_ssavar * self, jive_output * output)
{
	DEBUG_ASSERT(output->ssavar == self && self->assigned_output == output);
	
	jive_graph_notify_ssavar_unassign_output(self->variable->graph, self, output);
	output->ssavar = 0;
	self->assigned_output = 0;
	jive_ssavar_dec_use_count(self);
}

void
jive_ssavar_merge(jive_ssavar * self, jive_ssavar * other)
{
	DEBUG_ASSERT(self->variable == other->variable);
	DEBUG_ASSERT(self->origin == other->origin);
	
	if (other->assigned_output) {
		jive_output * output = other->assigned_output;
		jive_ssavar_unassign_output(other, output);
		jive_ssavar_assign_output(self, output);
	}
	
	while(other->assigned_inputs.first) {
		jive_input * input = other->assigned_inputs.first;
		jive_ssavar_unassign_input(other, input);
		jive_ssavar_assign_input(self, input);
	}
}

void
jive_ssavar_divert_origin(jive_ssavar * self, jive_output * new_origin)
{
	DEBUG_ASSERT(new_origin->ssavar == 0);
	
	jive_output * old_origin = self->origin;
	self->origin = new_origin;
	
	if (self->assigned_output) {
		self->assigned_output->ssavar = 0;
		self->assigned_output = new_origin;
		self->assigned_output->ssavar = self;
	}
	
	jive_input * input;
	JIVE_LIST_ITERATE(self->assigned_inputs, input, ssavar_input_list) {
		jive_input_internal_divert_origin(input, new_origin);
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
	jive_context * context = graph->context;
	jive_variable * self = jive_context_malloc(context, sizeof(*self));
	
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
	
	DEBUG_ASSERT(self->use_count == 0);
	JIVE_LIST_REMOVE(self->graph->unused_variables, self, graph_variable_list);
	
	jive_context_free(self->graph->context, self);
}

void
jive_variable_assign_gate(jive_variable * self, struct jive_gate * gate)
{
	DEBUG_ASSERT(gate->variable == 0);
	jive_variable_inc_use_count(self);
	JIVE_LIST_PUSH_BACK(self->gates, gate, variable_gate_list);
	gate->variable = self;
	jive_graph_notify_variable_assign_gate(self->graph, self, gate);
}

void
jive_variable_unassign_gate(jive_variable * self, struct jive_gate * gate)
{
	DEBUG_ASSERT(gate->variable == self);
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
	
	jive_variable_set_resource_class(other, new_rescls);
	jive_variable_set_resource_class(self, new_rescls);
	
	DEBUG_ASSERT(self->resname == 0 || other->resname == 0 || (self->resname == other->resname));
	if (other->resname && !self->resname)
		jive_variable_set_resource_name(self, other->resname);
	if (self->resname && !other->resname)
		jive_variable_set_resource_name(other, self->resname);
	
	while (other->gates.first) {
		jive_gate * gate = other->gates.first;
		jive_variable_unassign_gate(other, gate);
		jive_variable_assign_gate(self, gate);
	}
	
	while(other->ssavars.first) {
		jive_ssavar * ssavar = other->ssavars.first;
		
		jive_variable_inc_use_count(self);
		
		DEBUG_ASSERT(ssavar->use_count);
		JIVE_LIST_REMOVE(other->ssavars, ssavar, variable_ssavar_list);
		JIVE_LIST_PUSH_BACK(self->ssavars, ssavar, variable_ssavar_list);
		
		jive_graph_notify_ssavar_variable_change(self->graph, ssavar, other, self);
		
		jive_variable_dec_use_count(other);
	}
	
	while(other->unused_ssavars.first) {
		jive_ssavar * ssavar = other->unused_ssavars.first;
		
		DEBUG_ASSERT(ssavar->use_count);
		JIVE_LIST_REMOVE(other->unused_ssavars, ssavar, variable_ssavar_list);
		JIVE_LIST_PUSH_BACK(self->unused_ssavars, ssavar, variable_ssavar_list);
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
		jive_input * input;
		JIVE_LIST_ITERATE(ssavar->assigned_inputs, input, ssavar_input_list) {
			rescls = jive_resource_class_intersection(rescls, input->required_rescls);
		}
		if (ssavar->assigned_output)
			rescls = jive_resource_class_intersection(rescls, ssavar->assigned_output->required_rescls);
	}
	jive_gate * gate;
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
jive_variable_conflicts_with(const jive_variable * self, const jive_variable * other)
{
	return false; /* WTF?!? */
}

bool
jive_variable_may_spill(const jive_variable * self)
{
	jive_gate * gate;
	JIVE_LIST_ITERATE(self->gates, gate, variable_gate_list) {
		if (!gate->may_spill)
			return false;
	}
	return true;
}
