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
	shaped_graph->region_map.erase(region);
}

static void
jive_shaped_graph_region_add_used_ssavar(void * closure, jive_region * region,
	jive_ssavar * ssavar)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = shaped_graph->ssavar_map.find(ssavar).ptr();
	jive_shaped_ssavar_xpoints_register_region_arc(shaped_ssavar, ssavar->origin, region);
}

static void
jive_shaped_graph_region_remove_used_ssavar(void * closure, jive_region * region,
	jive_ssavar * ssavar)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = shaped_graph->ssavar_map.find(ssavar).ptr();
	jive_shaped_ssavar_xpoints_unregister_region_arc(shaped_ssavar, ssavar->origin, region);
}

static void
jive_shaped_graph_gate_interference_add(void * closure, jive::gate * first, jive::gate * second)
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
jive_shaped_graph_gate_interference_remove(void * closure, jive::gate * first, jive::gate * second)
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
	jive_shaped_variable * svar = shaped_graph->variable_map.find(variable).ptr();
	shaped_graph->variable_map.erase(svar);
}

static void
jive_shaped_graph_variable_assign_gate(void * closure, jive_variable * variable, jive::gate * gate)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_variable_assign_gate(shaped_graph->variable_map.find(variable).ptr(), gate);
}

static void
jive_shaped_graph_variable_unassign_gate(void * closure, jive_variable * variable,
	jive::gate * gate)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_variable_unassign_gate(shaped_graph->variable_map.find(variable).ptr(), gate);
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
	shaped_graph->ssavar_map.erase(ssavar);
}

static void
jive_shaped_graph_node_destroy(void * closure, jive_node * node)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	auto i = shaped_graph->node_map.find(node);
	if (i != shaped_graph->node_map.end())
		jive_shaped_node_destroy(i.ptr());
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
jive_shaped_graph_ssavar_assign_output(void * closure, jive_ssavar * ssavar, jive::output * output)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
	jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, output->node());
	if (shaped_node)
		jive_shaped_node_add_ssavar_after(shaped_node, shaped_ssavar, ssavar->variable, 1);
}

static void
jive_shaped_graph_ssavar_unassign_output(void * closure, jive_ssavar * ssavar,
	jive::output * output)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
	jive_shaped_node * shaped_node = jive_shaped_graph_map_node(shaped_graph, output->node());
	if (shaped_node)
		jive_shaped_node_remove_ssavar_after(shaped_node, shaped_ssavar, ssavar->variable, 1);
}

static void
jive_shaped_graph_ssavar_variable_change(void * closure, jive_ssavar * ssavar,
	jive_variable * old_var, jive_variable * new_var)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	jive_shaped_ssavar * shaped_ssavar = jive_shaped_graph_map_ssavar(shaped_graph, ssavar);
	jive_shaped_ssavar_xpoints_variable_change(shaped_ssavar, old_var, new_var);
}

static void
jive_shaped_graph_ssavar_divert(void * closure, jive_ssavar * ssavar, jive::output * old_origin,
	jive::output * new_origin)
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
	jive_shaped_graph * self = new jive_shaped_graph;
	
	self->graph = graph;
	
	size_t n;
	for(n = 0; n < sizeof(self->callbacks)/sizeof(self->callbacks[0]); n++)
		self->callbacks[n] = NULL;
	
	n = 0;
	self->callbacks[n++] = jive_region_notifier_slot_connect(&graph->on_region_create,
		jive_shaped_graph_region_create, self);
	self->callbacks[n++] = jive_region_notifier_slot_connect(&graph->on_region_destroy,
		jive_shaped_graph_region_destroy, self);
	self->callbacks[n++] = jive_region_ssavar_notifier_slot_connect(&graph->on_region_add_used_ssavar,
		jive_shaped_graph_region_add_used_ssavar, self);
	self->callbacks[n++] = jive_region_ssavar_notifier_slot_connect(
		&graph->on_region_remove_used_ssavar, jive_shaped_graph_region_remove_used_ssavar, self);
	self->callbacks[n++] = jive_gate_notifier_slot_connect(&graph->on_gate_interference_add,
		jive_shaped_graph_gate_interference_add, self);
	self->callbacks[n++] = jive_gate_notifier_slot_connect(&graph->on_gate_interference_remove,
		jive_shaped_graph_gate_interference_remove, self);
	self->callbacks[n++] = jive_variable_notifier_slot_connect(&graph->on_variable_create,
		jive_shaped_graph_variable_create, self);
	self->callbacks[n++] = jive_variable_notifier_slot_connect(&graph->on_variable_destroy,
		jive_shaped_graph_variable_destroy, self);
	self->callbacks[n++] = jive_variable_gate_notifier_slot_connect(&graph->on_variable_assign_gate,
		jive_shaped_graph_variable_assign_gate, self);
	self->callbacks[n++] = jive_variable_gate_notifier_slot_connect(&graph->on_variable_unassign_gate,
		jive_shaped_graph_variable_unassign_gate, self);
	self->callbacks[n++] = jive_variable_resource_class_notifier_slot_connect(
		&graph->on_variable_resource_class_change, jive_shaped_graph_variable_resource_class_change,
		self);
	self->callbacks[n++] = jive_variable_resource_name_notifier_slot_connect(
		&graph->on_variable_resource_name_change, jive_shaped_graph_variable_resource_name_change, self);
	self->callbacks[n++] = jive_ssavar_notifier_slot_connect(&graph->on_ssavar_create,
		jive_shaped_graph_ssavar_create, self);
	self->callbacks[n++] = jive_ssavar_notifier_slot_connect(&graph->on_ssavar_destroy,
		jive_shaped_graph_ssavar_destroy, self);
	self->callbacks[n++] = jive_node_notifier_slot_connect(&graph->on_node_destroy,
		jive_shaped_graph_node_destroy, self);
	self->callbacks[n++] = jive_ssavar_input_notifier_slot_connect(&graph->on_ssavar_assign_input,
		jive_shaped_graph_ssavar_assign_input, self);
	self->callbacks[n++] = jive_ssavar_input_notifier_slot_connect(&graph->on_ssavar_unassign_input,
		jive_shaped_graph_ssavar_unassign_input, self);
	self->callbacks[n++] = jive_ssavar_output_notifier_slot_connect(&graph->on_ssavar_assign_output,
		jive_shaped_graph_ssavar_assign_output, self);
	self->callbacks[n++] = jive_ssavar_output_notifier_slot_connect(&graph->on_ssavar_unassign_output,
		jive_shaped_graph_ssavar_unassign_output, self);
	self->callbacks[n++] = jive_ssavar_variable_notifier_slot_connect(
		&graph->on_ssavar_variable_change, jive_shaped_graph_ssavar_variable_change, self);
	self->callbacks[n++] = jive_ssavar_divert_notifier_slot_connect(
		&graph->on_ssavar_divert_origin, jive_shaped_graph_ssavar_divert, self);
	
	JIVE_DEBUG_ASSERT(n <= sizeof(self->callbacks)/sizeof(self->callbacks[0]));
	
	jive_var_assignment_tracker_init(&self->var_assignment_tracker);
	
	jive_node_notifier_slot_init(&self->on_shaped_node_create);
	jive_node_notifier_slot_init(&self->on_shaped_node_destroy);
	jive_shaped_region_ssavar_notifier_slot_init(&self->on_shaped_region_ssavar_add);
	jive_shaped_region_ssavar_notifier_slot_init(&self->on_shaped_region_ssavar_remove);
	
	jive_variable * variable;
	JIVE_LIST_ITERATE(graph->variables, variable, graph_variable_list) {
		jive_shaped_variable * shaped_variable = jive_shaped_variable_create(self, variable);
		jive_ssavar * ssavar;
		JIVE_LIST_ITERATE(variable->ssavars, ssavar, variable_ssavar_list) {
			jive_shaped_ssavar_create(self, ssavar);
		}
		jive::gate * gate;
		JIVE_LIST_ITERATE(variable->gates, gate, variable_gate_list) {
			jive_shaped_variable_initial_assign_gate(shaped_variable, gate);
		}
	}
	
	jive_shaped_graph_add_region_recursive(self, graph->root_region);
	
	return self;
}

jive_shaped_graph::~jive_shaped_graph()
{
	for (size_t n = 0; n < sizeof(callbacks)/sizeof(callbacks[0]); n++) {
		if (callbacks[n])
			jive_notifier_disconnect(callbacks[n]);
	}
	
	jive_shaped_region_destroy_cuts(jive_shaped_graph_map_region(this, graph->root_region));
	
	JIVE_DEBUG_ASSERT(node_map.size() == 0);

	jive_node_notifier_slot_fini(&on_shaped_node_create);
	jive_node_notifier_slot_fini(&on_shaped_node_destroy);
	jive_shaped_region_ssavar_notifier_slot_fini(&on_shaped_region_ssavar_add);
	jive_shaped_region_ssavar_notifier_slot_fini(&on_shaped_region_ssavar_remove);
}

void
jive_shaped_graph_destroy(jive_shaped_graph * self)
{
	delete self;
}
