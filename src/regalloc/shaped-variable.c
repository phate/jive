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
#include <jive/vsdg/region-ssavar-use-private.h>

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

size_t
jive_shaped_variable_is_active_before(const jive_shaped_variable * self, const jive_shaped_node * shaped_node)
{
	size_t count = 0;
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(self->variable->ssavars, ssavar, variable_ssavar_list) {
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar);
		count += jive_shaped_ssavar_is_active_before(shaped_ssavar, shaped_node);
	}
	
	return count;
}

size_t
jive_shaped_variable_is_crossing(const jive_shaped_variable * self, const jive_shaped_node * shaped_node)
{
	size_t count = 0;
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(self->variable->ssavars, ssavar, variable_ssavar_list) {
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar);
		count += jive_shaped_ssavar_is_crossing(shaped_ssavar, shaped_node);
	}
	
	return count;
}

size_t
jive_shaped_variable_is_active_after(const jive_shaped_variable * self, const jive_shaped_node * shaped_node)
{
	size_t count = 0;
	jive_ssavar * ssavar;
	JIVE_LIST_ITERATE(self->variable->ssavars, ssavar, variable_ssavar_list) {
		jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(self->shaped_graph, ssavar);
		count += jive_shaped_ssavar_is_active_after(shaped_ssavar, shaped_node);
	}
	
	return count;
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
	self->boundary_region_depth = (size_t)-1;
	
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
jive_shaped_ssavar_set_boundary_region_depth(jive_shaped_ssavar * self, size_t depth)
{
	if (self->boundary_region_depth == depth)
		return;
		
	/* if node is shaped, boundary region is ignored */
	jive_shaped_node * origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, self->ssavar->origin->node);
	if (origin_shaped_node) {
		self->boundary_region_depth = depth;
		return;
	}
	
	jive_shaped_ssavar_xpoints_unregister_arcs(self);
	self->boundary_region_depth = depth;
	jive_shaped_ssavar_xpoints_register_arcs(self);
}

size_t
jive_shaped_ssavar_is_active_before(const jive_shaped_ssavar * self, const jive_shaped_node * shaped_node)
{
	jive_xpoint * xpoint = jive_node_xpoint_hash_lookup(&self->node_xpoints, shaped_node);
	if (!xpoint) return 0;
	else return xpoint->before_count;
}

size_t
jive_shaped_ssavar_is_crossing(const jive_shaped_ssavar * self, const jive_shaped_node * shaped_node)
{
	jive_xpoint * xpoint = jive_node_xpoint_hash_lookup(&self->node_xpoints, shaped_node);
	if (!xpoint) return 0;
	else return xpoint->cross_count;
}

size_t
jive_shaped_ssavar_is_active_after(const jive_shaped_ssavar * self, const jive_shaped_node * shaped_node)
{
	jive_xpoint * xpoint = jive_node_xpoint_hash_lookup(&self->node_xpoints, shaped_node);
	if (!xpoint) return 0;
	else return xpoint->after_count;
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
	
	if (input_shaped_node && (origin_shaped_node || self->boundary_region_depth <= input_shaped_node->node->region->depth))
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
	
	if (input_shaped_node && (origin_shaped_node || self->boundary_region_depth <= input_shaped_node->node->region->depth))
		jive_shaped_node_remove_ssavar_before(input_shaped_node, self, variable, 1);
	
	if (origin_shaped_node && input_shaped_node)
		jive_shaped_node_remove_ssavar_after(origin_shaped_node, self, variable, 1);
}

void
jive_shaped_ssavar_xpoints_register_region_arc(jive_shaped_ssavar * self, jive_output * output, jive_region * region)
{
	jive_variable * variable = self->ssavar->variable;
		
	jive_shaped_node * origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, output->node);
	jive_shaped_node * input_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, jive_region_get_bottom_node(region));
	
	jive_crossing_arc_iterator i;
	jive_crossing_arc_iterator_init(&i, origin_shaped_node, input_shaped_node, self);
	
	while(i.region) {
		if (i.node)
			jive_shaped_node_add_ssavar_crossed(i.node, self, variable, 1);
		else
			jive_shaped_region_add_active_top(i.region, self, 1);
		jive_crossing_arc_iterator_next(&i);
	}
	
	if (input_shaped_node)
		jive_shaped_node_add_ssavar_before(input_shaped_node, self, variable, 1);
	
	if (origin_shaped_node && input_shaped_node)
		jive_shaped_node_add_ssavar_after(origin_shaped_node, self, variable, 1);
}

void
jive_shaped_ssavar_xpoints_unregister_region_arc(jive_shaped_ssavar * self, jive_output * output, jive_region * region)
{
	jive_variable * variable = self->ssavar->variable;
		
	jive_shaped_node * origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, output->node);
	jive_shaped_node * input_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, jive_region_get_bottom_node(region));
	
	jive_crossing_arc_iterator i;
	jive_crossing_arc_iterator_init(&i, origin_shaped_node, input_shaped_node, self);
	
	while(i.region) {
		if (i.node)
			jive_shaped_node_remove_ssavar_crossed(i.node, self, variable, 1);
		else
			jive_shaped_region_remove_active_top(i.region, self, 1);
		jive_crossing_arc_iterator_next(&i);
	}
	
	if (input_shaped_node)
		jive_shaped_node_remove_ssavar_before(input_shaped_node, self, variable, 1);
	
	if (origin_shaped_node && input_shaped_node)
		jive_shaped_node_remove_ssavar_after(origin_shaped_node, self, variable, 1);
}

void
jive_shaped_ssavar_xpoints_register_arcs(jive_shaped_ssavar * self)
{
	jive_ssavar * ssavar = self->ssavar;
	jive_variable * variable = ssavar->variable;
	jive_input * input;
	JIVE_LIST_ITERATE(ssavar->assigned_inputs, input, ssavar_input_list) {
		jive_shaped_ssavar_xpoints_register_arc(self, input, input->origin);
	}
	if (ssavar->assigned_output) {
		jive_shaped_node * origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, ssavar->assigned_output->node);
		if (origin_shaped_node)
			jive_shaped_node_add_ssavar_after(origin_shaped_node, self, variable, 1);
	}
	
	struct jive_ssavar_region_hash_iterator i;
	JIVE_HASH_ITERATE(jive_ssavar_region_hash, ssavar->assigned_regions, i) {
		jive_region * region = i.entry->region;
		jive_shaped_ssavar_xpoints_register_region_arc(self, self->ssavar->origin, region);
	}
}

void
jive_shaped_ssavar_xpoints_unregister_arcs(jive_shaped_ssavar * self)
{
	jive_ssavar * ssavar = self->ssavar;
	jive_variable * variable = ssavar->variable;
	jive_input * input;
	JIVE_LIST_ITERATE(ssavar->assigned_inputs, input, ssavar_input_list) {
		jive_shaped_ssavar_xpoints_unregister_arc(self, input, input->origin);
	}
	if (ssavar->assigned_output) {
		jive_shaped_node * origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, ssavar->assigned_output->node);
		if (origin_shaped_node)
			jive_shaped_node_remove_ssavar_after(origin_shaped_node, self, variable, 1);
	}
	
	struct jive_ssavar_region_hash_iterator i;
	JIVE_HASH_ITERATE(jive_ssavar_region_hash, ssavar->assigned_regions, i) {
		jive_region * region = i.entry->region;
		jive_shaped_ssavar_xpoints_unregister_region_arc(self, self->ssavar->origin, region);
	}
}

void
jive_shaped_ssavar_xpoints_variable_change(jive_shaped_ssavar * self, jive_variable * old_variable, jive_variable * new_variable)
{
	jive_shaped_graph * shaped_graph = self->shaped_graph;
	jive_context * context = shaped_graph->context;
	jive_shaped_variable * old_shaped_var = jive_shaped_graph_map_variable(self->shaped_graph, old_variable);
	jive_shaped_variable * new_shaped_var = jive_shaped_graph_map_variable(self->shaped_graph, new_variable);
	const jive_resource_class * old_rescls = jive_variable_get_resource_class(old_variable);
	const jive_resource_class * new_rescls = jive_variable_get_resource_class(new_variable);
	
	struct jive_node_xpoint_hash_iterator i;
	JIVE_HASH_ITERATE(jive_node_xpoint_hash, self->node_xpoints, i) {
		jive_xpoint * xpoint= i.entry;
		
		jive_shaped_node * shaped_node = xpoint->shaped_node;
		
		struct jive_ssavar_xpoint_hash_iterator j;
		JIVE_HASH_ITERATE(jive_ssavar_xpoint_hash, shaped_node->ssavar_xpoints, j) {
			jive_xpoint * other_xpoint = j.entry;
			if (other_xpoint == xpoint) continue;
			jive_shaped_variable * other_shaped_var;
			other_shaped_var = jive_shaped_graph_map_variable(shaped_graph, other_xpoint->shaped_ssavar->ssavar->variable);
			
			if (xpoint->before_count && other_xpoint->before_count) {
				jive_variable_interference_remove(old_shaped_var, other_shaped_var);
				jive_variable_interference_add(new_shaped_var, other_shaped_var);
				
			}
			if (xpoint->after_count && other_xpoint->after_count) {
				jive_variable_interference_remove(old_shaped_var, other_shaped_var);
				jive_variable_interference_add(new_shaped_var, other_shaped_var);
			}
		}
		if (old_rescls != new_rescls) {
			if (xpoint->before_count)
				jive_resource_class_count_change(&shaped_node->use_count_before, context, old_rescls, new_rescls);
			if (xpoint->after_count)
				jive_resource_class_count_change(&shaped_node->use_count_after, context, old_rescls, new_rescls);
		}
	}
}

void
jive_shaped_ssavar_notify_divert_origin(jive_shaped_ssavar * self, jive_output * old_origin, jive_output * new_origin)
{
	jive_shaped_node * old_origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, old_origin->node);
	jive_shaped_node * new_origin_shaped_node = jive_shaped_graph_map_node(self->shaped_graph, new_origin->node);
	
	jive_variable * variable = self->ssavar->variable;
	jive_input * input;
	struct jive_ssavar_region_hash_iterator i;
	
	if (old_origin_shaped_node && self->ssavar->assigned_output)
		jive_shaped_node_remove_ssavar_after(old_origin_shaped_node, self, variable, 1);
	JIVE_LIST_ITERATE(self->ssavar->assigned_inputs, input, ssavar_input_list) {
		jive_shaped_ssavar_xpoints_unregister_arc(self, input, old_origin);
	}
	JIVE_HASH_ITERATE(jive_ssavar_region_hash, self->ssavar->assigned_regions, i) {
		jive_region * region = i.entry->region;
		jive_shaped_ssavar_xpoints_unregister_region_arc(self, old_origin, region);
	}
	
	JIVE_LIST_ITERATE(self->ssavar->assigned_inputs, input, ssavar_input_list) {
		jive_shaped_ssavar_xpoints_register_arc(self, input, new_origin);
	}
	JIVE_HASH_ITERATE(jive_ssavar_region_hash, self->ssavar->assigned_regions, i) {
		jive_region * region = i.entry->region;
		jive_shaped_ssavar_xpoints_register_region_arc(self, new_origin, region);
	}
	if (new_origin_shaped_node && self->ssavar->assigned_output)
		jive_shaped_node_add_ssavar_after(new_origin_shaped_node, self, variable, 1);
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
