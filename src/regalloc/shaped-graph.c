/*
 * Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/shaped-graph.h>

#include <jive/common.h>
#include <jive/regalloc/assignment-tracker-private.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/regalloc/shaped-variable-private.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/variable.h>

JIVE_DEFINE_HASH_TYPE(jive_shaped_region_hash, jive_shaped_region, const jive_region *, region, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_shaped_variable_hash, jive_shaped_variable, const struct jive_variable *, variable, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_shaped_ssavar_hash, jive_shaped_ssavar, const struct jive_ssavar *, ssavar, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_shaped_node_hash, jive_shaped_node, const struct jive_node *, node, hash_chain);

static void
jive_shaped_graph_region_create(void * closure, jive_region * region)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_region_create(shaped_graph, region);
}

static void
jive_shaped_graph_region_destroy(void * closure, jive_region * region)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_region * shaped_region = jive_shaped_region_hash_lookup(&shaped_graph->region_map, region);
	jive_shaped_region_destroy(shaped_region);
}

static void
jive_shaped_graph_region_add_used_ssavar(void * closure, jive_region * region, jive_ssavar * ssavar)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_ssavar_hash_lookup(&shaped_graph->ssavar_map, ssavar);
	
	jive_shaped_ssavar_xpoints_register_region_arc(shaped_ssavar, ssavar->origin, region);
}

static void
jive_shaped_graph_region_remove_used_ssavar(void * closure, jive_region * region, jive_ssavar * ssavar)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_ssavar_hash_lookup(&shaped_graph->ssavar_map, ssavar);
	
	jive_shaped_ssavar_xpoints_unregister_region_arc(shaped_ssavar, ssavar->origin, region);
}

static void
jive_shaped_graph_gate_interference_add(void * closure, jive_gate * first, jive_gate * second)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	if (first->variable && second->variable) {
		jive_variable_interference_add(
			jive_shaped_graph_map_variable(shaped_graph, first->variable),
			jive_shaped_graph_map_variable(shaped_graph, second->variable)
		);
	}
}

static void
jive_shaped_graph_gate_interference_remove(void * closure, jive_gate * first, jive_gate * second)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	if (first->variable && second->variable) {
		jive_variable_interference_remove(
			jive_shaped_graph_map_variable(shaped_graph, first->variable),
			jive_shaped_graph_map_variable(shaped_graph, second->variable)
		);
	}
}

static void
jive_shaped_graph_variable_create(void * closure, jive_variable * variable)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_variable_create(shaped_graph, variable);
}

static void
jive_shaped_graph_variable_destroy(void * closure, jive_variable * variable)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_variable * shaped_variable = jive_shaped_variable_hash_lookup(&shaped_graph->variable_map, variable);
	jive_shaped_variable_destroy(shaped_variable);
}

static void
jive_shaped_graph_variable_assign_gate(void * closure, jive_variable * variable, jive_gate * gate)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_variable * shaped_variable = jive_shaped_variable_hash_lookup(&shaped_graph->variable_map, variable);
	jive_shaped_variable_assign_gate(shaped_variable, gate);
}

static void
jive_shaped_graph_variable_unassign_gate(void * closure, jive_variable * variable, jive_gate * gate)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_variable * shaped_variable = jive_shaped_variable_hash_lookup(&shaped_graph->variable_map, variable);
	jive_shaped_variable_unassign_gate(shaped_variable, gate);
}

static void
jive_shaped_graph_variable_resource_class_change(void * closure, jive_variable * variable,
	const jive_resource_class * old_rescls, const jive_resource_class * new_rescls)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_variable * shaped_variable = jive_shaped_graph_map_variable(shaped_graph, variable);
	if (shaped_variable)
		jive_shaped_variable_resource_class_change(shaped_variable, old_rescls, new_rescls);
}

static void
jive_shaped_graph_variable_resource_name_change(void * closure, jive_variable * variable,
	const jive_resource_name * old_resname, const jive_resource_name * new_resname)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_variable * shaped_variable = jive_shaped_graph_map_variable(shaped_graph, variable);
	if (shaped_variable)
		jive_shaped_variable_resource_name_change(shaped_variable, old_resname, new_resname);
}

static void
jive_shaped_graph_ssavar_create(void * closure, jive_ssavar * ssavar)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar_create(shaped_graph, ssavar);
}

static void
jive_shaped_graph_ssavar_destroy(void * closure, jive_ssavar * ssavar)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_ssavar_hash_lookup(&shaped_graph->ssavar_map, ssavar);
	jive_shaped_ssavar_destroy(shaped_ssavar);
}

static void
jive_shaped_graph_node_destroy(void * closure, jive_node * node)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_node * shaped_node = jive_shaped_node_hash_lookup(&shaped_graph->node_map, node);
	if (shaped_node) jive_shaped_node_destroy(shaped_node);
}

static void
jive_shaped_graph_ssavar_assign_input(void * closure, jive_ssavar * ssavar, jive::input * input)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
	jive_shaped_ssavar_xpoints_register_arc(shaped_ssavar, input, input->origin());
}

static void
jive_shaped_graph_ssavar_unassign_input(void * closure, jive_ssavar * ssavar, jive::input * input)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
	jive_shaped_ssavar_xpoints_unregister_arc(shaped_ssavar, input, input->origin());
}

static void
jive_shaped_graph_ssavar_assign_output(void * closure, jive_ssavar * ssavar, jive_output * output)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
	jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, output->node());
	if (shaped_node)
		jive_shaped_node_add_ssavar_after(shaped_node, shaped_ssavar, ssavar->variable, 1);
}

static void
jive_shaped_graph_ssavar_unassign_output(void * closure, jive_ssavar * ssavar, jive_output * output)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
	jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, output->node());
	if (shaped_node)
		jive_shaped_node_remove_ssavar_after(shaped_node, shaped_ssavar, ssavar->variable, 1);
}

static void
jive_shaped_graph_ssavar_variable_change(void * closure, jive_ssavar * ssavar, jive_variable * old_var, jive_variable * new_var)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
	jive_shaped_ssavar_xpoints_variable_change(shaped_ssavar, old_var, new_var);
}

static void
jive_shaped_graph_ssavar_divert(void * closure, jive_ssavar * ssavar, jive_output * old_origin, jive_output * new_origin)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
	jive_shaped_ssavar_notify_divert_origin(shaped_ssavar, old_origin, new_origin);
}

static void
jive_shaped_graph_add_region_recursive(jive_shaped_graph * self, jive_region * region)
{
	jive_shaped_graph_region_create(self, region);
	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list)
		jive_shaped_graph_add_region_recursive(self, subregion);
}

jive_shaped_graph *
jive_shaped_graph_create(jive_graph * graph)
{
	jive_context * context = graph->context;
	jive_shaped_graph * self = jive_context_malloc(context, sizeof(*self));
	
	self->graph = graph;
	self->context = context;
	
	size_t n;
	for(n = 0; n < sizeof(self->callbacks)/sizeof(self->callbacks[0]); n++)
		self->callbacks[n] = NULL;
	
	n = 0;
	self->callbacks[n++] = jive_region_notifier_slot_connect(&graph->on_region_create, jive_shaped_graph_region_create, self);
	self->callbacks[n++] = jive_region_notifier_slot_connect(&graph->on_region_destroy, jive_shaped_graph_region_destroy, self);
	self->callbacks[n++] = jive_region_ssavar_notifier_slot_connect(&graph->on_region_add_used_ssavar, jive_shaped_graph_region_add_used_ssavar, self);
	self->callbacks[n++] = jive_region_ssavar_notifier_slot_connect(&graph->on_region_remove_used_ssavar, jive_shaped_graph_region_remove_used_ssavar, self);
	self->callbacks[n++] = jive_gate_notifier_slot_connect(&graph->on_gate_interference_add, jive_shaped_graph_gate_interference_add, self);
	self->callbacks[n++] = jive_gate_notifier_slot_connect(&graph->on_gate_interference_remove, jive_shaped_graph_gate_interference_remove, self);
	self->callbacks[n++] = jive_variable_notifier_slot_connect(&graph->on_variable_create, jive_shaped_graph_variable_create, self);
	self->callbacks[n++] = jive_variable_notifier_slot_connect(&graph->on_variable_destroy, jive_shaped_graph_variable_destroy, self);
	self->callbacks[n++] = jive_variable_gate_notifier_slot_connect(&graph->on_variable_assign_gate, jive_shaped_graph_variable_assign_gate, self);
	self->callbacks[n++] = jive_variable_gate_notifier_slot_connect(&graph->on_variable_unassign_gate, jive_shaped_graph_variable_unassign_gate, self);
	self->callbacks[n++] = jive_variable_resource_class_notifier_slot_connect(&graph->on_variable_resource_class_change, jive_shaped_graph_variable_resource_class_change, self);
	self->callbacks[n++] = jive_variable_resource_name_notifier_slot_connect(&graph->on_variable_resource_name_change, jive_shaped_graph_variable_resource_name_change, self);
	self->callbacks[n++] = jive_ssavar_notifier_slot_connect(&graph->on_ssavar_create, jive_shaped_graph_ssavar_create, self);
	self->callbacks[n++] = jive_ssavar_notifier_slot_connect(&graph->on_ssavar_destroy, jive_shaped_graph_ssavar_destroy, self);
	self->callbacks[n++] = jive_node_notifier_slot_connect(&graph->on_node_destroy, jive_shaped_graph_node_destroy, self);
	self->callbacks[n++] = jive_ssavar_input_notifier_slot_connect(&graph->on_ssavar_assign_input, jive_shaped_graph_ssavar_assign_input, self);
	self->callbacks[n++] = jive_ssavar_input_notifier_slot_connect(&graph->on_ssavar_unassign_input, jive_shaped_graph_ssavar_unassign_input, self);
	self->callbacks[n++] = jive_ssavar_output_notifier_slot_connect(&graph->on_ssavar_assign_output, jive_shaped_graph_ssavar_assign_output, self);
	self->callbacks[n++] = jive_ssavar_output_notifier_slot_connect(&graph->on_ssavar_unassign_output, jive_shaped_graph_ssavar_unassign_output, self);
	self->callbacks[n++] = jive_ssavar_variable_notifier_slot_connect(&graph->on_ssavar_variable_change, jive_shaped_graph_ssavar_variable_change, self);
	self->callbacks[n++] = jive_ssavar_divert_notifier_slot_connect(&graph->on_ssavar_divert_origin, jive_shaped_graph_ssavar_divert, self);
	
	JIVE_DEBUG_ASSERT(n <= sizeof(self->callbacks)/sizeof(self->callbacks[0]));
	
	jive_shaped_region_hash_init(&self->region_map, context);
	jive_shaped_variable_hash_init(&self->variable_map, context);
	jive_shaped_ssavar_hash_init(&self->ssavar_map, context);
	jive_shaped_node_hash_init(&self->node_map, context);
	jive_var_assignment_tracker_init(&self->var_assignment_tracker, context);
	
	jive_node_notifier_slot_init(&self->on_shaped_node_create, context);
	jive_node_notifier_slot_init(&self->on_shaped_node_destroy, context);
	jive_shaped_region_ssavar_notifier_slot_init(&self->on_shaped_region_ssavar_add, context);
	jive_shaped_region_ssavar_notifier_slot_init(&self->on_shaped_region_ssavar_remove, context);
	
	jive_variable * variable;
	JIVE_LIST_ITERATE(graph->variables, variable, graph_variable_list) {
		jive_shaped_variable * shaped_variable = jive_shaped_variable_create(self, variable);
		jive_ssavar * ssavar;
		JIVE_LIST_ITERATE(variable->ssavars, ssavar, variable_ssavar_list) {
			jive_shaped_ssavar_create(self, ssavar);
		}
		jive_gate * gate;
		JIVE_LIST_ITERATE(variable->gates, gate, variable_gate_list) {
			jive_shaped_variable_initial_assign_gate(shaped_variable, gate);
		}
	}
	
	jive_shaped_graph_add_region_recursive(self, graph->root_region);
	
	return self;
}

void
jive_shaped_graph_destroy(jive_shaped_graph * self)
{
	jive_context * context = self->context;
	size_t n;
	for(n = 0; n < sizeof(self->callbacks)/sizeof(self->callbacks[0]); n++) {
		if (self->callbacks[n])
			jive_notifier_disconnect(self->callbacks[n]);
	}
	
	jive_shaped_region_destroy_cuts(jive_shaped_graph_map_region(self, self->graph->root_region));
	
	JIVE_DEBUG_ASSERT(self->node_map.nitems == 0);
	
	struct jive_shaped_region_hash_iterator region_iter;
	region_iter = jive_shaped_region_hash_begin(&self->region_map);
	while(region_iter.entry) {
		struct jive_shaped_region_hash_iterator next = region_iter;
		jive_shaped_region_hash_iterator_next(&next);
		jive_shaped_region_destroy(region_iter.entry);
		region_iter = next;
	}
	
	struct jive_shaped_variable_hash_iterator variable_iter;
	variable_iter = jive_shaped_variable_hash_begin(&self->variable_map);
	while(variable_iter.entry) {
		struct jive_shaped_variable_hash_iterator next = variable_iter;
		jive_shaped_variable_hash_iterator_next(&next);
		jive_shaped_variable_destroy(variable_iter.entry);
		variable_iter = next;
	}
	
	struct jive_shaped_ssavar_hash_iterator ssavar_iter;
	ssavar_iter = jive_shaped_ssavar_hash_begin(&self->ssavar_map);
	while(ssavar_iter.entry) {
		struct jive_shaped_ssavar_hash_iterator next = ssavar_iter;
		jive_shaped_ssavar_hash_iterator_next(&next);
		jive_shaped_ssavar_destroy(ssavar_iter.entry);
		ssavar_iter = next;
	}
	
	jive_shaped_node_hash_fini(&self->node_map);
	jive_shaped_ssavar_hash_fini(&self->ssavar_map);
	jive_shaped_variable_hash_fini(&self->variable_map);
	jive_shaped_region_hash_fini(&self->region_map);
	jive_var_assignment_tracker_fini(&self->var_assignment_tracker);
	
	jive_node_notifier_slot_fini(&self->on_shaped_node_create);
	jive_node_notifier_slot_fini(&self->on_shaped_node_destroy);
	jive_shaped_region_ssavar_notifier_slot_fini(&self->on_shaped_region_ssavar_add);
	jive_shaped_region_ssavar_notifier_slot_fini(&self->on_shaped_region_ssavar_remove);
	
	jive_context_free(context, self);
}

jive_shaped_region *
jive_shaped_graph_map_region(const jive_shaped_graph * self, const jive_region * region)
{
	return jive_shaped_region_hash_lookup(&self->region_map, region);
}

jive_shaped_variable *
jive_shaped_graph_map_variable(const jive_shaped_graph * self, const jive_variable * variable)
{
	return jive_shaped_variable_hash_lookup(&self->variable_map, variable);
}

jive_shaped_ssavar *
jive_shaped_graph_map_ssavar(const jive_shaped_graph * self, const jive_ssavar * ssavar)
{
	return jive_shaped_ssavar_hash_lookup(&self->ssavar_map, ssavar);
}

jive_shaped_node *
jive_shaped_graph_map_node(const jive_shaped_graph * self, const jive_node * node)
{
	return jive_shaped_node_hash_lookup(&self->node_map, node);
}
