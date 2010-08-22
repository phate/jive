#include <jive/vsdg/graph-private.h>
#include <jive/vsdg/region-private.h>
#include <jive/vsdg/traverser-private.h>
#include <jive/util/list.h>

static inline void
jive_graph_valueres_tracker_init(jive_graph_valueres_tracker * self, jive_context * context)
{
	self->context = context;
	self->assigned.first = self->assigned.last = 0;
	self->trivial.first = self->trivial.last = 0;
	self->max_pressure = self->space = 0;
	self->pressured = 0;
}

static inline void
jive_graph_valueres_tracker_fini(jive_graph_valueres_tracker * self)
{
	jive_context_free(self->context, self->pressured);
}

static inline void
_jive_graph_init(jive_graph * self, jive_context * context)
{
	self->context = context;
	self->resources.first = self->resources.last = 0;
	self->unused_resources.first = self->unused_resources.last = 0;
	self->top.first = self->top.last = 0;
	self->bottom.first = self->bottom.last = 0;
	self->gates.first = self->gates.last = 0;
	self->resources_fully_assigned = false;
	
	self->root_region = jive_context_malloc(context, sizeof(*self->root_region));
	_jive_region_init(self->root_region, self, 0);
	
	self->ntraverser_slots = 0;
	self->traverser_slots = 0;
	
	jive_node_notifier_slot_init(&self->on_node_create, context);
	jive_node_notifier_slot_init(&self->on_node_destroy, context);
	jive_node_notifier_slot_init(&self->on_node_place, context);
	jive_node_notifier_slot_init(&self->on_node_remove_place, context);
	
	jive_input_notifier_slot_init(&self->on_input_create, context);
	jive_input_change_notifier_slot_init(&self->on_input_change, context);
	jive_input_notifier_slot_init(&self->on_input_destroy, context);
	
	jive_output_notifier_slot_init(&self->on_output_create, context);
	jive_output_notifier_slot_init(&self->on_output_destroy, context);
	
	jive_graph_valueres_tracker_init(&self->valueres, context);
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
	while(self->bottom.first) {
		jive_graph_prune(self);
		jive_node * node;
		JIVE_LIST_ITERATE(self->bottom, node, graph_bottom_list)
			node->reserved = 0;
	}
	
	jive_context_free(self->context, self->traverser_slots);
	
	prune_regions_recursive(self->root_region);
	
	while(self->gates.first) jive_gate_destroy(self->gates.first);
	
	while(self->unused_resources.first) jive_resource_destroy(self->unused_resources.first);
	
	jive_graph_valueres_tracker_fini(&self->valueres);
	
	jive_node_notifier_slot_fini(&self->on_node_create);
	jive_node_notifier_slot_fini(&self->on_node_destroy);
	jive_node_notifier_slot_fini(&self->on_node_place);
	jive_node_notifier_slot_fini(&self->on_node_remove_place);
	
	jive_input_notifier_slot_fini(&self->on_input_create);
	jive_input_change_notifier_slot_fini(&self->on_input_change);
	jive_input_notifier_slot_fini(&self->on_input_destroy);
	
	jive_output_notifier_slot_fini(&self->on_output_create);
	jive_output_notifier_slot_fini(&self->on_output_destroy);
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
		
		if (!node->reserved) jive_node_destroy(node);
		
		node = next;
	}
}
