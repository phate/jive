/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_GRAPH_PRIVATE_H
#define JIVE_VSDG_GRAPH_PRIVATE_H

#include <jive/common.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/basetype.h>
#include <jive/vsdg/valuetype.h>

struct jive_tracker_slot_reservation {
	jive_tracker_slot slot;
	bool in_use;
};

struct jive_tracker_nodestate_list {
	jive_tracker_nodestate * first;
	jive_tracker_nodestate * last;
};

struct jive_tracker_depth_state {
	size_t top_occupied;
	size_t bottom_occupied;
	size_t count;
	size_t space;
	std::vector<jive_tracker_nodestate_list> nodestates_per_depth;
	struct jive_context * context;
	
	struct {
		jive_tracker_depth_state * prev;
		jive_tracker_depth_state * next;
	} graph_cached_tracker_states;
};

jive_tracker_slot
jive_graph_reserve_tracker_slot_slow(jive_graph * self);

static inline jive_tracker_slot
jive_graph_reserve_tracker_slot(jive_graph * self)
{
	size_t n;
	for (n = 0; n < self->tracker_slots.size(); n++) {
		if (!self->tracker_slots[n].in_use) {
			/* in theory, overflow might be possible, causing
			a cookie to be reused... just catch this case
			even if it is never going to happen in real life */
			if (self->tracker_slots[n].slot.cookie == (size_t) -1)
				continue;
			self->tracker_slots[n].slot.cookie ++;
			self->tracker_slots[n].in_use = true;
			return self->tracker_slots[n].slot;
		}
	}
	
	return jive_graph_reserve_tracker_slot_slow(self);
}

static inline void
jive_graph_return_tracker_slot(jive_graph * self, jive_tracker_slot slot)
{
	JIVE_DEBUG_ASSERT(self->tracker_slots[slot.index].in_use);
	self->tracker_slots[slot.index].in_use = false;
}

static inline jive_tracker_depth_state *
jive_graph_reserve_tracker_depth_state(jive_graph * self)
{
	jive_tracker_depth_state * state = new jive_tracker_depth_state;
	state->count = 0;
	state->space = 0;
	state->context = self->context;
	return state;
}

static inline void
jive_graph_return_tracker_depth_state(jive_graph * self, jive_tracker_depth_state * state)
{
	delete state;
}

static inline void
jive_graph_notify_node_create(jive_graph * graph, jive_node * node)
{
	jive_node_notifier_slot_call(&graph->on_node_create, node);
}

static inline void
jive_graph_notify_node_destroy(jive_graph * graph, jive_node * node)
{
	jive_node_notifier_slot_call(&graph->on_node_destroy, node);
}

static inline void
jive_graph_notify_input_create(jive_graph * graph, jive::input * input)
{
	jive_input_notifier_slot_call(&graph->on_input_create, input);
}

static inline void
jive_graph_notify_input_change(jive_graph * graph, jive::input * input, jive::output * old_origin,
	jive::output * new_origin)
{
	jive_input_change_notifier_slot_call(&graph->on_input_change, input, old_origin, new_origin);
}

static inline void
jive_graph_notify_input_destroy(jive_graph * graph, jive::input * input)
{
	jive_input_notifier_slot_call(&graph->on_input_destroy, input);
}

static inline void
jive_graph_notify_output_create(jive_graph * graph, jive::output * output)
{
	jive_output_notifier_slot_call(&graph->on_output_create, output);
}

static inline void
jive_graph_notify_output_destroy(jive_graph * graph, jive::output * output)
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
jive_graph_notify_variable_assign_gate(jive_graph * graph, struct jive_variable * variable,
	jive::gate * gate)
{
	jive_variable_gate_notifier_slot_call(&graph->on_variable_assign_gate, variable, gate);
}

static inline void
jive_graph_notify_variable_unassign_gate(jive_graph * graph, struct jive_variable * variable,
	jive::gate * gate)
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
jive_graph_notify_ssavar_assign_input(jive_graph * graph, struct jive_ssavar * ssavar,
	jive::input * input)
{
	jive_ssavar_input_notifier_slot_call(&graph->on_ssavar_assign_input, ssavar, input);
}

static inline void
jive_graph_notify_ssavar_unassign_input(jive_graph * graph, struct jive_ssavar * ssavar,
	jive::input * input)
{
	jive_ssavar_input_notifier_slot_call(&graph->on_ssavar_unassign_input, ssavar, input);
}

static inline void
jive_graph_notify_ssavar_assign_output(jive_graph * graph, struct jive_ssavar * ssavar,
	jive::output * output)
{
	jive_ssavar_output_notifier_slot_call(&graph->on_ssavar_assign_output, ssavar, output);
}

static inline void
jive_graph_notify_ssavar_unassign_output(jive_graph * graph, struct jive_ssavar * ssavar,
	jive::output * output)
{
	jive_ssavar_output_notifier_slot_call(&graph->on_ssavar_unassign_output, ssavar, output);
}

static inline void
jive_graph_notify_ssavar_divert_origin(jive_graph * graph, struct jive_ssavar * ssavar,
	jive::output * old_origin, jive::output * new_origin)
{
	jive_ssavar_divert_notifier_slot_call(&graph->on_ssavar_divert_origin, ssavar, old_origin,
		new_origin);
}

static inline void
jive_graph_notify_ssavar_variable_change(jive_graph * graph, struct jive_ssavar * ssavar,
	struct jive_variable * old_variable, struct jive_variable * new_variable)
{
	jive_ssavar_variable_notifier_slot_call(&graph->on_ssavar_variable_change, ssavar, old_variable,
		new_variable);
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

#endif
