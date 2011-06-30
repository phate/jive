#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/region-private.h>
#include <jive/vsdg/traverser-private.h>
#include <jive/vsdg/variable.h>
#include <jive/util/list.h>

static inline void
_jive_graph_init(jive_graph * self, jive_context * context)
{
	self->context = context;
	self->variables.first = self->variables.last = 0;
	self->unused_variables.first = self->unused_variables.last = 0;
	self->top.first = self->top.last = 0;
	self->bottom.first = self->bottom.last = 0;
	self->gates.first = self->gates.last = 0;
	self->labels.first = self->labels.last = 0;
	self->resources_fully_assigned = false;
	
	jive_region_notifier_slot_init(&self->on_region_create, context);
	jive_region_notifier_slot_init(&self->on_region_destroy, context);
	jive_region_ssavar_notifier_slot_init(&self->on_region_add_used_ssavar, context);
	jive_region_ssavar_notifier_slot_init(&self->on_region_remove_used_ssavar, context);
	
	jive_node_notifier_slot_init(&self->on_node_create, context);
	jive_node_notifier_slot_init(&self->on_node_destroy, context);
	jive_node_depth_notifier_slot_init(&self->on_node_depth_change, context);
	
	jive_input_notifier_slot_init(&self->on_input_create, context);
	jive_input_change_notifier_slot_init(&self->on_input_change, context);
	jive_input_notifier_slot_init(&self->on_input_destroy, context);
	
	jive_output_notifier_slot_init(&self->on_output_create, context);
	jive_output_notifier_slot_init(&self->on_output_destroy, context);
	
	jive_variable_notifier_slot_init(&self->on_variable_create, context);
	jive_variable_notifier_slot_init(&self->on_variable_destroy, context);
	jive_variable_gate_notifier_slot_init(&self->on_variable_assign_gate, context);
	jive_variable_gate_notifier_slot_init(&self->on_variable_unassign_gate, context);
	jive_variable_resource_class_notifier_slot_init(&self->on_variable_resource_class_change, context);
	jive_variable_resource_name_notifier_slot_init(&self->on_variable_resource_name_change, context);
	
	jive_gate_notifier_slot_init(&self->on_gate_interference_add, context);
	jive_gate_notifier_slot_init(&self->on_gate_interference_remove, context);
	
	jive_ssavar_notifier_slot_init(&self->on_ssavar_create, context);
	jive_ssavar_notifier_slot_init(&self->on_ssavar_destroy, context);
	jive_ssavar_input_notifier_slot_init(&self->on_ssavar_assign_input, context);
	jive_ssavar_input_notifier_slot_init(&self->on_ssavar_unassign_input, context);
	jive_ssavar_output_notifier_slot_init(&self->on_ssavar_assign_output, context);
	jive_ssavar_output_notifier_slot_init(&self->on_ssavar_unassign_output, context);
	jive_ssavar_divert_notifier_slot_init(&self->on_ssavar_divert_origin, context);
	jive_ssavar_variable_notifier_slot_init(&self->on_ssavar_variable_change, context);
	
	self->root_region = jive_context_malloc(context, sizeof(*self->root_region));
	_jive_region_init(self->root_region, self, 0);
	
	self->ntraverser_slots = 0;
	self->traverser_slots = 0;
}

static void
prune_regions_recursive(jive_region * region)
{
	jive_region * subregion, * next;
	JIVE_LIST_ITERATE_SAFE(region->subregions, subregion, next, region_subregions_list)
		prune_regions_recursive(subregion);
	if (jive_region_empty(region)) jive_region_destroy(region);
}

static void
_jive_graph_fini(jive_graph * self)
{
	while(self->labels.first) {
		jive_label_internal * label = self->labels.first;
		label->base.class_->fini(&label->base);
		jive_context_free(self->context, label);
	}
	
	while(self->bottom.first) {
		jive_graph_prune(self);
		jive_node * node;
		JIVE_LIST_ITERATE(self->bottom, node, graph_bottom_list)
			node->reserved = 0;
	}
	
	jive_context_free(self->context, self->traverser_slots);
	
	prune_regions_recursive(self->root_region);
	
	while(self->gates.first) jive_gate_destroy(self->gates.first);
	
	while(self->unused_variables.first) jive_variable_destroy(self->unused_variables.first);
	
	jive_region_notifier_slot_fini(&self->on_region_create);
	jive_region_notifier_slot_fini(&self->on_region_destroy);
	jive_region_ssavar_notifier_slot_fini(&self->on_region_add_used_ssavar);
	jive_region_ssavar_notifier_slot_fini(&self->on_region_remove_used_ssavar);
	
	jive_node_notifier_slot_fini(&self->on_node_create);
	jive_node_notifier_slot_fini(&self->on_node_destroy);
	jive_node_depth_notifier_slot_fini(&self->on_node_depth_change);
	
	jive_input_notifier_slot_fini(&self->on_input_create);
	jive_input_change_notifier_slot_fini(&self->on_input_change);
	jive_input_notifier_slot_fini(&self->on_input_destroy);
	
	jive_output_notifier_slot_fini(&self->on_output_create);
	jive_output_notifier_slot_fini(&self->on_output_destroy);
	
	jive_variable_notifier_slot_fini(&self->on_variable_create);
	jive_variable_notifier_slot_fini(&self->on_variable_destroy);
	jive_variable_gate_notifier_slot_fini(&self->on_variable_assign_gate);
	jive_variable_gate_notifier_slot_fini(&self->on_variable_unassign_gate);
	jive_variable_resource_class_notifier_slot_fini(&self->on_variable_resource_class_change);
	jive_variable_resource_name_notifier_slot_fini(&self->on_variable_resource_name_change);
	
	jive_gate_notifier_slot_fini(&self->on_gate_interference_add);
	jive_gate_notifier_slot_fini(&self->on_gate_interference_remove);
	
	jive_ssavar_notifier_slot_fini(&self->on_ssavar_create);
	jive_ssavar_notifier_slot_fini(&self->on_ssavar_destroy);
	jive_ssavar_input_notifier_slot_fini(&self->on_ssavar_assign_input);
	jive_ssavar_input_notifier_slot_fini(&self->on_ssavar_unassign_input);
	jive_ssavar_output_notifier_slot_fini(&self->on_ssavar_assign_output);
	jive_ssavar_output_notifier_slot_fini(&self->on_ssavar_unassign_output);
	jive_ssavar_divert_notifier_slot_fini(&self->on_ssavar_divert_origin);
	jive_ssavar_variable_notifier_slot_fini(&self->on_ssavar_variable_change);
}

jive_graph *
jive_graph_create(jive_context * context)
{
	jive_graph * graph;
	graph = jive_context_malloc(context, sizeof(*graph));
	_jive_graph_init(graph, context);
	return graph;
}

void
jive_graph_destroy(jive_graph * self)
{
	_jive_graph_fini(self);
	jive_context_free(self->context, self);
}

bool
jive_graph_has_active_traversers(const jive_graph * self)
{
	size_t n;
	for(n=0; n<self->ntraverser_slots; n++)
		if (self->traverser_slots[n].traversal_state) return true;
	return false;
}

void
jive_graph_prune(jive_graph * self)
{
	jive_node * node;
	node = self->bottom.first;
	while(node) {
		jive_node * next = node->graph_bottom_list.next;
		
		if (!node->reserved) {
			jive_node_destroy(node);
			if (!next) next = self->bottom.first;
		}
		
		node = next;
	}
	
	jive_region * subregion, * next;
	JIVE_LIST_ITERATE_SAFE(self->root_region->subregions, subregion, next, region_subregions_list)
		prune_regions_recursive(subregion);
}

void
jive_graph_push_outward(jive_graph * self)
{
	jive_traverser * trav = jive_topdown_traverser_create(self);
	
	for(;;) {
		jive_node * node = jive_traverser_next(trav);
		if (!node)
			break;
		
		while (jive_node_can_move_outward(node))
			jive_node_move_outward(node);
	}
	
	jive_traverser_destroy(trav);
}

void
jive_graph_pull_inward(jive_graph * self)
{
	jive_traverser * trav = jive_bottomup_traverser_create(self);
	
	for(;;) {
		jive_node * node = jive_traverser_next(trav);
		if (!node)
			break;
		jive_region * region;
		do {
			region = node->region;
			jive_node_move_inward(node);
		} while (region != node->region);
	}
	
	jive_traverser_destroy(trav);
}

