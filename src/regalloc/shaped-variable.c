#include <jive/regalloc/shaped-variable.h>
#include <jive/regalloc/shaped-variable-private.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/regalloc/crossing-arc.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/regalloc/shaped-region-private.h>
#include <jive/vsdg/gate-interference-private.h>

JIVE_DEFINE_HASH_TYPE(jive_shaped_variable_hash, jive_shaped_variable, struct jive_variable *, variable, hash_chain);

jive_shaped_variable *
jive_shaped_variable_create(struct jive_shaped_graph * shaped_graph, struct jive_variable * variable)
{
	jive_context * context = shaped_graph->context;
	jive_shaped_variable * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->variable = variable;
	
	jive_shaped_variable_hash_insert(&shaped_graph->variable_map, self);
	jive_variable_interference_hash_init(&self->interference, context);
	
	jive_allowed_resource_names_hash_init(&self->allowed_names, context);
	jive_shaped_variable_internal_recompute_allowed_names(self);
	
	jive_var_assignment_tracker_add_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, self->variable->resname);
	return self;
}

void
jive_shaped_variable_resource_class_change(jive_shaped_variable * self, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls)
{
	jive_shaped_graph * shaped_graph = self->shaped_graph;
	jive_ssavar * ssavar;
	
	JIVE_LIST_ITERATE(self->variable->ssavars, ssavar, variable_ssavar_list) {
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
		jive_shaped_ssavar_xpoints_change_resource_class(shaped_ssavar, old_rescls, new_rescls);
	}
	
	if (!self->variable->resname) {
		struct jive_variable_interference_hash_iterator i;
		JIVE_HASH_ITERATE(jive_variable_interference_hash, self->interference, i) {
			jive_shaped_variable * other = i.entry->shaped_variable;
			jive_shaped_variable_sub_squeeze(other, old_rescls);
			jive_shaped_variable_add_squeeze(other, new_rescls);
		}
	}
	jive_var_assignment_tracker_remove_tracked(&self->shaped_graph->var_assignment_tracker,
		self, old_rescls, self->variable->resname);
	jive_shaped_variable_internal_recompute_allowed_names(self);
	jive_var_assignment_tracker_add_tracked(&self->shaped_graph->var_assignment_tracker,
		self, new_rescls, self->variable->resname);
}

void
jive_shaped_variable_resource_name_change(jive_shaped_variable * self, const struct jive_resource_name * old_resname, const struct jive_resource_name * new_resname)
{
	JIVE_DEBUG_ASSERT(old_resname == new_resname || self->variable->rescls->limit == 0 || jive_shaped_variable_allowed_resource_name(self, new_resname));
	
	jive_var_assignment_tracker_remove_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, old_resname);
	jive_shaped_variable_internal_recompute_allowed_names(self);
	jive_var_assignment_tracker_add_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, new_resname);
		
	struct jive_variable_interference_hash_iterator i;
	JIVE_HASH_ITERATE(jive_variable_interference_hash, self->interference, i) {
		jive_shaped_variable * other = i.entry->shaped_variable;
		jive_allowed_resource_names_remove(&other->allowed_names, new_resname);
		jive_shaped_variable_sub_squeeze(other, new_resname->resource_class);
	}
}

bool
jive_shaped_variable_allowed_resource_name(const jive_shaped_variable * self, const struct jive_resource_name * name)
{
	jive_allowed_resource_name * allowed = jive_allowed_resource_names_hash_lookup(&self->allowed_names, name);
	return !!allowed;
}

size_t
jive_shaped_variable_allowed_resource_name_count(const jive_shaped_variable * self)
{
	return self->allowed_names.nitems;
}

void
jive_shaped_variable_initial_assign_gate(jive_shaped_variable * self, jive_gate * gate)
{
	/* during initial build of shaped_graph, other_shape might be NULL */
	
	struct jive_gate_interference_hash_iterator i;
	JIVE_HASH_ITERATE(jive_gate_interference_hash, gate->interference, i) {
		jive_gate * other_gate = i.entry->gate;
		jive_variable * other = other_gate->variable;
		if (!other) continue;
		jive_shaped_variable * other_shape = jive_shaped_graph_map_variable(self->shaped_graph, other);
		if (other_shape)
			jive_variable_interference_add(self, other_shape);
	}
}

void
jive_shaped_variable_assign_gate(jive_shaped_variable * self, jive_gate * gate)
{
	struct jive_gate_interference_hash_iterator i;
	JIVE_HASH_ITERATE(jive_gate_interference_hash, gate->interference, i) {
		jive_gate * other_gate = i.entry->gate;
		jive_variable * other = other_gate->variable;
		if (!other) continue;
		jive_shaped_variable * other_shape = jive_shaped_graph_map_variable(self->shaped_graph, other);
		jive_variable_interference_add(self, other_shape);
	}
}

void
jive_shaped_variable_unassign_gate(jive_shaped_variable * self, jive_gate * gate)
{
	struct jive_gate_interference_hash_iterator i;
	JIVE_HASH_ITERATE(jive_gate_interference_hash, gate->interference, i) {
		jive_gate * other_gate = i.entry->gate;
		jive_variable * other = other_gate->variable;
		if (!other) continue;
		jive_shaped_variable * other_shape = jive_shaped_graph_map_variable(self->shaped_graph, other);
		jive_variable_interference_remove(self, other_shape);
	}
}

void
jive_shaped_variable_destroy(jive_shaped_variable * self)
{
	jive_gate * gate;
	JIVE_LIST_ITERATE(self->variable->gates, gate, variable_gate_list) {
		struct jive_gate_interference_hash_iterator i;
		JIVE_HASH_ITERATE(jive_gate_interference_hash, gate->interference, i) {
			jive_gate * other_gate = i.entry->gate;
			jive_variable * other = other_gate->variable;
			if (!other) continue;
			jive_shaped_variable * other_shape = jive_shaped_graph_map_variable(self->shaped_graph, other);
			if (other_shape)
				jive_variable_interference_remove(self, other_shape);
		}
	}
	jive_var_assignment_tracker_remove_tracked(&self->shaped_graph->var_assignment_tracker,
		self, self->variable->rescls, self->variable->resname);
	JIVE_DEBUG_ASSERT(self->interference.nitems == 0);
	jive_variable_interference_hash_fini(&self->interference);
	jive_allowed_resource_names_clear(&self->allowed_names);
	jive_allowed_resource_names_hash_fini(&self->allowed_names);
	jive_shaped_variable_hash_remove(&self->shaped_graph->variable_map, self);
	jive_context_free(self->shaped_graph->context, self);
}

jive_variable_interference *
jive_variable_interference_create(jive_shaped_variable * first, jive_shaped_variable * second)
{
	jive_variable_interference * i = jive_context_malloc(first->shaped_graph->context, sizeof(*i));
	i->first.shaped_variable = first;
	i->first.whole = i;
	i->second.shaped_variable = second;
	i->second.whole = i;
	i->count = 0;
	
	jive_variable_interference_hash_insert(&first->interference, &i->second);
	jive_variable_interference_hash_insert(&second->interference, &i->first);
	
	return i;
}

void
jive_variable_interference_destroy(jive_variable_interference * self)
{
	jive_variable_interference_hash_remove(&self->first.shaped_variable->interference, &self->second);
	jive_variable_interference_hash_remove(&self->second.shaped_variable->interference, &self->first);
	jive_context_free(self->first.shaped_variable->shaped_graph->context, self);
}


JIVE_DEFINE_HASH_TYPE(jive_shaped_ssavar_hash, jive_shaped_ssavar, struct jive_ssavar *, ssavar, hash_chain);

jive_shaped_ssavar *
jive_shaped_ssavar_create(struct jive_shaped_graph * shaped_graph, struct jive_ssavar * ssavar)
{
	jive_context * context = shaped_graph->context;
	jive_shaped_ssavar * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->ssavar = ssavar;
	self->hovering = false;
	
	jive_shaped_ssavar_hash_insert(&shaped_graph->ssavar_map, self);
	
	jive_node_xpoint_hash_init(&self->node_xpoints, context);
	jive_region_tpoint_hash_init(&self->region_tpoints, context);
	
	return self;
}

size_t
jive_shaped_variable_interferes_with(const jive_shaped_variable * self, const jive_shaped_variable * other)
{
	jive_variable_interference_part * part = jive_variable_interference_hash_lookup(&self->interference, other);
	if (part)
		return part->whole->count;
	else
		return 0;
}

bool
jive_shaped_variable_can_merge(const jive_shaped_variable * self, const jive_variable * other)
{
	if (!other)
		return true;
	
	jive_shaped_variable * other_shape;
	other_shape = jive_shaped_graph_map_variable(self->shaped_graph, other);
	if (other_shape && jive_variable_interference_hash_lookup(&self->interference, other_shape))
		return false;
	
	const jive_resource_class * new_rescls;
	new_rescls = jive_resource_class_intersection(self->variable->rescls, other->rescls);
	if (!new_rescls)
		return false;
	
	const jive_resource_class * overflow;
	overflow = jive_shaped_variable_check_change_resource_class(self, new_rescls);
	if (overflow)
		return false;
	
	if (other_shape) {
		overflow = jive_shaped_variable_check_change_resource_class(other_shape, new_rescls);
		if (overflow)
			return false;
	}
	
	if (self->variable->resname && other->resname && self->variable->resname != other->resname)
		return false;
	
	return true;
}

const jive_resource_class *
jive_shaped_variable_check_change_resource_class(const jive_shaped_variable * self, const jive_resource_class * new_rescls)
{
	const jive_resource_class * old_rescls = self->variable->rescls;
	if (old_rescls == new_rescls)
		return NULL;
	
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(self->variable->ssavars, ssavar, variable_ssavar_list) {
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar);
		const jive_resource_class * overflow;
		overflow = jive_shaped_ssavar_check_change_resource_class(shaped_ssavar, old_rescls, new_rescls);
		if (overflow)
			return overflow;
	}
	
	jive_context * context = self->shaped_graph->context;
	
	jive_resource_class_count use_count;
	jive_resource_class_count_init(&use_count);
	
	jive_gate * gate;
	JIVE_LIST_ITERATE(self->variable->gates, gate, variable_gate_list) {
		jive_input * input;
		JIVE_LIST_ITERATE(gate->inputs, input, gate_inputs_list) {
			jive_node_get_use_count_input(input->node, &use_count, context);
			const jive_resource_class * overflow;
			overflow = jive_resource_class_count_check_change(&use_count, old_rescls, new_rescls);
			if (overflow) {
				jive_resource_class_count_fini(&use_count, context);
				return overflow;
			}
		}
		jive_output * output;
		JIVE_LIST_ITERATE(gate->outputs, output, gate_outputs_list) {
			jive_node_get_use_count_output(output->node, &use_count, context);
			const jive_resource_class * overflow;
			overflow = jive_resource_class_count_check_change(&use_count, old_rescls, new_rescls);
			if (overflow) {
				jive_resource_class_count_fini(&use_count, context);
				return overflow;
			}
		}
	}
	
	jive_resource_class_count_fini(&use_count, context);
	
	return 0;
}


void
jive_shaped_ssavar_destroy(jive_shaped_ssavar * self)
{
	JIVE_DEBUG_ASSERT(self->node_xpoints.nitems == 0);
	JIVE_DEBUG_ASSERT(self->region_tpoints.nitems == 0);
	jive_node_xpoint_hash_fini(&self->node_xpoints);
	jive_region_tpoint_hash_fini(&self->region_tpoints);
	jive_shaped_ssavar_hash_remove(&self->shaped_graph->ssavar_map, self);
	jive_context_free(self->shaped_graph->context, self);
}

void
jive_shaped_ssavar_xpoints_register_arc(jive_shaped_ssavar * self, jive_input * input, jive_output * output)
{
	jive_variable * variable = self->ssavar->variable;
	
	jive_shaped_node * origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, output->node);
	jive_shaped_node * input_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, input->node);
	
	jive_crossing_arc_iterator i;
	jive_crossing_arc_iterator_init(&i, origin_shaped_node, input_shaped_node, self);
	
	while(i.region) {
		if (i.node)
			jive_shaped_node_add_ssavar_crossed(i.node, self, variable, 1);
		else
			jive_shaped_region_add_active_top(i.region, self, 1);
		jive_crossing_arc_iterator_next(&i);
	}
	
	if (input_shaped_node && (origin_shaped_node || self->hovering))
		jive_shaped_node_add_ssavar_before(input_shaped_node, self, variable, 1);
	
	if (origin_shaped_node && input_shaped_node)
		jive_shaped_node_add_ssavar_after(origin_shaped_node, self, variable, 1);
}

void
jive_shaped_ssavar_xpoints_unregister_arc(jive_shaped_ssavar * self, jive_input * input, jive_output * output)
{
	jive_variable * variable = self->ssavar->variable;
	
	jive_shaped_node * origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, output->node);
	jive_shaped_node * input_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, input->node);
	
	jive_crossing_arc_iterator i;
	jive_crossing_arc_iterator_init(&i, origin_shaped_node, input_shaped_node, self);
	
	while(i.region) {
		if (i.node)
			jive_shaped_node_remove_ssavar_crossed(i.node, self, variable, 1);
		else
			jive_shaped_region_remove_active_top(i.region, self, 1);
		jive_crossing_arc_iterator_next(&i);
	}
	
	if (input_shaped_node && (origin_shaped_node || self->hovering))
		jive_shaped_node_remove_ssavar_before(input_shaped_node, self, variable, 1);
	
	if (origin_shaped_node && input_shaped_node)
		jive_shaped_node_remove_ssavar_after(origin_shaped_node, self, variable, 1);
}

void
jive_shaped_ssavar_xpoints_change_resource_class(jive_shaped_ssavar * self, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls)
{
	jive_context * context = self->shaped_graph->context;
	
	struct jive_node_xpoint_hash_iterator i;
	JIVE_HASH_ITERATE(jive_node_xpoint_hash, self->node_xpoints, i) {
		jive_xpoint * xpoint = i.entry;
		jive_shaped_node * shaped_node = xpoint->shaped_node;
		if (xpoint->before_count)
			jive_resource_class_count_change(&shaped_node->use_count_before, context, old_rescls, new_rescls);
		if (xpoint->after_count)
			jive_resource_class_count_change(&shaped_node->use_count_after, context, old_rescls, new_rescls);
	}
}

const jive_resource_class *
jive_shaped_ssavar_check_change_resource_class(const jive_shaped_ssavar * self, const struct jive_resource_class * old_rescls, const struct jive_resource_class * new_rescls)
{
	const jive_resource_class * overflow;
	struct jive_node_xpoint_hash_iterator i;
	JIVE_HASH_ITERATE(jive_node_xpoint_hash, self->node_xpoints, i) {
		jive_xpoint * xpoint = i.entry;
		jive_shaped_node * shaped_node = xpoint->shaped_node;
		if (xpoint->before_count) {
			overflow = jive_resource_class_count_check_change(&shaped_node->use_count_before, old_rescls, new_rescls);
			if (overflow)
				return overflow;
		}
		if (xpoint->after_count) {
			overflow = jive_resource_class_count_check_change(&shaped_node->use_count_after, old_rescls, new_rescls);
			if (overflow)
				return overflow;
		}
	}
	
	return 0;
}
