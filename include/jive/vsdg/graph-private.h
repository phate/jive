#ifndef JIVE_VSDG_GRAPH_PRIVATE_H
#define JIVE_VSDG_GRAPH_PRIVATE_H

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/valuetype.h>

static inline void
jive_graph_notify_node_create(jive_graph * graph, jive_node * node)
{
	jive_node_notifier_slot_call(&graph->on_node_create, node);
}

static inline void
jive_graph_notify_node_destroy(jive_graph * graph, jive_node * node)
{
	jive_node_notifier_slot_call(&graph->on_node_create, node);
}

static inline void
jive_graph_notify_node_place(jive_graph * graph, jive_node * node)
{
	jive_node_notifier_slot_call(&graph->on_node_place, node);
}

static inline void
jive_graph_notify_node_remove_place(jive_graph * graph, jive_node * node)
{
	jive_node_notifier_slot_call(&graph->on_node_remove_place, node);
}

static inline void
jive_graph_notify_input_create(jive_graph * graph, jive_input * input)
{
	jive_input_notifier_slot_call(&graph->on_input_create, input);
}

static inline void
jive_graph_notify_input_change(jive_graph * graph, jive_input * input, jive_output * old_origin, jive_output * new_origin)
{
	jive_input_change_notifier_slot_call(&graph->on_input_change, input, old_origin, new_origin);
}

static inline void
jive_graph_notify_input_destroy(jive_graph * graph, jive_input * input)
{
	jive_input_notifier_slot_call(&graph->on_input_destroy, input);
}

static inline void
jive_graph_notify_output_create(jive_graph * graph, jive_output * output)
{
	jive_output_notifier_slot_call(&graph->on_output_create, output);
}

static inline void
jive_graph_notify_output_destroy(jive_graph * graph, jive_output * output)
{
	jive_output_notifier_slot_call(&graph->on_output_destroy, output);
}

static inline void
jive_graph_notify_variable_create(jive_graph * graph, struct jive_variable * variable)
{
	jive_variable_notifier_slot_call(&graph->on_variable_create, variable);
}

static inline void
jive_graph_notify_variable_destroy(jive_graph * graph, struct jive_variable * variable)
{
	jive_variable_notifier_slot_call(&graph->on_variable_destroy, variable);
}

static inline void
jive_graph_notify_variable_assign_gate(jive_graph * graph, struct jive_variable * variable, struct jive_gate * gate)
{
	jive_variable_gate_notifier_slot_call(&graph->on_variable_assign_gate, variable, gate);
}

static inline void
jive_graph_notify_variable_unassign_gate(jive_graph * graph, struct jive_variable * variable, struct jive_gate * gate)
{
	jive_variable_gate_notifier_slot_call(&graph->on_variable_unassign_gate, variable, gate);
}

static inline void
jive_graph_notify_variable_resource_class_change(jive_graph * graph, struct jive_variable * variable, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls)
{
	jive_variable_resource_class_notifier_slot_call(&graph->on_variable_resource_class_change, variable, old_rescls, new_rescls);
}

static inline void
jive_graph_notify_variable_resource_name_change(jive_graph * graph, struct jive_variable * variable, const struct jive_resource_name * old_rescls, const struct jive_resource_name * new_rescls)
{
	jive_variable_resource_name_notifier_slot_call(&graph->on_variable_resource_name_change, variable, old_rescls, new_rescls);
}

static inline void
jive_graph_notify_ssavar_create(jive_graph * graph, struct jive_ssavar * ssavar)
{
	jive_ssavar_notifier_slot_call(&graph->on_ssavar_create, ssavar);
}

static inline void
jive_graph_notify_ssavar_destroy(jive_graph * graph, struct jive_ssavar * ssavar)
{
	jive_ssavar_notifier_slot_call(&graph->on_ssavar_destroy, ssavar);
}

static inline void
jive_graph_notify_ssavar_assign_input(jive_graph * graph, struct jive_ssavar * ssavar, struct jive_input * input)
{
	jive_ssavar_input_notifier_slot_call(&graph->on_ssavar_assign_input, ssavar, input);
}

static inline void
jive_graph_notify_ssavar_unassign_input(jive_graph * graph, struct jive_ssavar * ssavar, struct jive_input * input)
{
	jive_ssavar_input_notifier_slot_call(&graph->on_ssavar_unassign_input, ssavar, input);
}

static inline void
jive_graph_notify_ssavar_assign_output(jive_graph * graph, struct jive_ssavar * ssavar, struct jive_output * output)
{
	jive_ssavar_output_notifier_slot_call(&graph->on_ssavar_assign_output, ssavar, output);
}

static inline void
jive_graph_notify_ssavar_unassign_output(jive_graph * graph, struct jive_ssavar * ssavar, struct jive_output * output)
{
	jive_ssavar_output_notifier_slot_call(&graph->on_ssavar_unassign_output, ssavar, output);
}

static inline void
jive_graph_notify_ssavar_divert_origin(jive_graph * graph, struct jive_ssavar * ssavar, struct jive_output * old_origin, struct jive_output * new_origin)
{
	jive_ssavar_divert_notifier_slot_call(&graph->on_ssavar_divert_origin, ssavar, old_origin, new_origin);
}

static inline void
jive_graph_notify_ssavar_variable_change(jive_graph * graph, struct jive_ssavar * ssavar, struct jive_variable * old_variable, struct jive_variable * new_variable)
{
	jive_ssavar_variable_notifier_slot_call(&graph->on_ssavar_variable_change, ssavar, old_variable, new_variable);
}

static inline void
jive_graph_notify_region_create(jive_graph * graph, struct jive_region * region)
{
	jive_region_notifier_slot_call(&graph->on_region_create, region);
}

static inline void
jive_graph_notify_region_destroy(jive_graph * graph, struct jive_region * region)
{
	jive_region_notifier_slot_call(&graph->on_region_create, region);
}

static inline void
jive_graph_notify_region_add_used_ssavar(jive_graph * graph, struct jive_region * region, struct jive_ssavar * ssavar)
{
	jive_region_ssavar_notifier_slot_call(&graph->on_region_add_used_ssavar, region, ssavar);
}

static inline void
jive_graph_notify_region_remove_used_ssavar(jive_graph * graph, struct jive_region * region, struct jive_ssavar * ssavar)
{
	jive_region_ssavar_notifier_slot_call(&graph->on_region_remove_used_ssavar, region, ssavar);
}

#if 0
static inline void
jive_graph_valueres_tracker_add(jive_graph_valueres_tracker * self, jive_value_resource * resource)
{
	if (resource->cpureg) {
		JIVE_LIST_PUSH_BACK(self->assigned, resource, graph_valueres_list);
	} else if (resource->allowed_registers.nitems > resource->squeeze) {
		JIVE_LIST_PUSH_BACK(self->trivial, resource, graph_valueres_list);
	} else {
		size_t pressure = resource->squeeze - resource->allowed_registers.nitems;
		if (pressure >= self->space) {
			self->pressured = jive_context_realloc(self->context, self->pressured, (pressure + 1) * sizeof(self->pressured[0]));
			size_t n;
			for(n=self->space; n<=pressure ; n++)
				self->pressured[n].first = self->pressured[n].last = 0;
			self->space = pressure + 1;
		}
		JIVE_LIST_PUSH_BACK(self->pressured[pressure], resource, graph_valueres_list);
		if (pressure >= self->max_pressure) self->max_pressure = pressure + 1;
	}
}

static inline void
jive_graph_valueres_tracker_remove(jive_graph_valueres_tracker * self, jive_value_resource * resource)
{
	if (resource->cpureg) {
		JIVE_LIST_REMOVE(self->assigned, resource, graph_valueres_list);
	} else if (resource->allowed_registers.nitems > resource->squeeze) {
		JIVE_LIST_REMOVE(self->trivial, resource, graph_valueres_list);
	} else {
		size_t pressure = resource->squeeze - resource->allowed_registers.nitems;
		JIVE_LIST_REMOVE(self->pressured[pressure], resource, graph_valueres_list);
		while(self->max_pressure && !self->pressured[self->max_pressure-1].first)
			self->max_pressure --;
	}
}
#endif

#endif
