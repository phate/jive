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
jive_graph_valueres_tracker_add(jive_graph_valueres_tracker * self, jive_value_resource * resource)
{
	if (resource->cpureg) {
		JIVE_LIST_PUSH_BACK(self->assigned, resource, graph_valueres_list);
	} else if (resource->allowed_registers.nitems > resource->squeeze) {
		JIVE_LIST_PUSH_BACK(self->trivial, resource, graph_valueres_list);
	} else {
		size_t index = resource->squeeze - resource->allowed_registers.nitems;
		if (index >= self->space) {
			self->pressured = jive_context_realloc(self->context, self->pressured, (index + 1) * sizeof(self->pressured[0]));
			size_t n;
			for(n=self->space; n<=index ; n++)
				self->pressured[index].first = self->pressured[index].last = 0;
			self->space = index + 1;
		}
		JIVE_LIST_PUSH_BACK(self->pressured[index], resource, graph_valueres_list);
		if (index >= self->max_pressure) self->max_pressure = index + 1;
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
		size_t index = resource->squeeze - resource->allowed_registers.nitems;
		if (index >= self->space) {
			self->pressured = jive_context_realloc(self->context, self->pressured, (index + 1) * sizeof(self->pressured[0]));
			size_t n;
			for(n=self->space; n<=index ; n++)
				self->pressured[index].first = self->pressured[index].last = 0;
			self->space = index + 1;
		}
		JIVE_LIST_REMOVE(self->pressured[index], resource, graph_valueres_list);
		while(self->max_pressure && !self->pressured[self->max_pressure-1].first)
			self->max_pressure --;
	}
}

#endif
