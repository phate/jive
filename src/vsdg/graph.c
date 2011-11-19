#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region-private.h>
#include <jive/vsdg/substitution.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/variable.h>
#include <jive/util/list.h>

JIVE_DEFINE_HASH_TYPE(jive_node_normal_form_hash, struct jive_node_normal_form, struct jive_node_class *, node_class, hash_chain);

static inline void
jive_graph_init_(jive_graph * self, jive_context * context)
{
	self->context = context;
	self->variables.first = self->variables.last = 0;
	self->unused_variables.first = self->unused_variables.last = 0;
	self->top.first = self->top.last = 0;
	self->bottom.first = self->bottom.last = 0;
	self->gates.first = self->gates.last = 0;
	self->labels.first = self->labels.last = 0;
	self->resources_fully_assigned = false;
	self->normalized = true;
	
	jive_node_normal_form_hash_init(&self->node_normal_forms, context);
	
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
	jive_region_init_(self->root_region, self, 0);
	
	self->ntracker_slots = 0;
	self->tracker_slots = 0;
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
jive_graph_fini_(jive_graph * self)
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
	
	jive_context_free(self->context, self->tracker_slots);
	
	prune_regions_recursive(self->root_region);
	
	while(self->gates.first) jive_gate_destroy(self->gates.first);
	
	while(self->unused_variables.first) jive_variable_destroy(self->unused_variables.first);
	
	struct jive_node_normal_form_hash_iterator i;
	i = jive_node_normal_form_hash_begin(&self->node_normal_forms);
	while (i.entry) {
		jive_node_normal_form * normal_form = i.entry;
		jive_node_normal_form_hash_iterator_next(&i);
		
		jive_node_normal_form_fini(normal_form);
		jive_context_free(self->context, normal_form);
	}
	jive_node_normal_form_hash_fini(&self->node_normal_forms);
	
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
	jive_graph_init_(graph, context);
	return graph;
}

void
jive_graph_destroy(jive_graph * self)
{
	jive_graph_fini_(self);
	jive_context_free(self->context, self);
}

jive_tracker_slot
jive_graph_reserve_tracker_slot_slow(jive_graph * self)
{
	size_t n = self->ntracker_slots;
	
	self->ntracker_slots ++;
	self->tracker_slots = jive_context_realloc(self->context,
		self->tracker_slots,
		sizeof(self->tracker_slots[0]) * self->ntracker_slots);
	
	self->tracker_slots[n].slot.index = n;
	self->tracker_slots[n].slot.cookie = 1;
	self->tracker_slots[n].in_use = true;
	
	return self->tracker_slots[n].slot;
}

bool
jive_graph_has_active_traversers(const jive_graph * self)
{
	size_t n;
	for(n=0; n<self->ntracker_slots; n++)
		if (self->tracker_slots[n].in_use) return true;
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

jive_graph *
jive_graph_copy(jive_graph * self, jive_context * context)
{
	jive_graph * new_graph = jive_graph_create(context);
	
	jive_substitution_map * subst = jive_substitution_map_create(context);
	jive_region_copy_substitute(self->root_region, new_graph->root_region, subst, false, false);
	jive_substitution_map_destroy(subst);
	
	return new_graph;
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

jive_node_normal_form *
jive_graph_get_nodeclass_form(jive_graph * self, const jive_node_class * node_class)
{
	jive_node_normal_form * normal_form;
	normal_form = jive_node_normal_form_hash_lookup(&self->node_normal_forms, node_class);
	if (normal_form)
		return normal_form;
	
	/* note: recursion depth only depends on class hierarchy depths */
	jive_node_normal_form * parent_normal_form = NULL;
	if (node_class->parent)
		parent_normal_form = jive_graph_get_nodeclass_form(self, node_class -> parent);
	
	normal_form = node_class->get_default_normal_form(node_class, parent_normal_form, self);
	jive_node_normal_form_hash_insert(&self->node_normal_forms, normal_form);
	
	return normal_form;
}

void
jive_graph_mark_denormalized(jive_graph * self)
{
	self->normalized = false;
}

void
jive_graph_normalize(jive_graph * self)
{
	jive_traverser * trav = jive_topdown_traverser_create(self);
	
	jive_node * node;
	for(node = jive_traverser_next(trav); node; node = jive_traverser_next(trav)) {
		jive_node_normal_form * nf = jive_graph_get_nodeclass_form(self, node->class_);
		jive_node_normal_form_normalize_node(nf, node);
	}
	
	jive_traverser_destroy(trav);
	self->normalized = true;
}
