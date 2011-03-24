#include <jive/regalloc/shaped-node.h>
#include <jive/regalloc/shaped-node-private.h>

#include <jive/context.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/xpoint-private.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

JIVE_DEFINE_HASH_TYPE(jive_shaped_node_hash, jive_shaped_node, struct jive_node *, node, hash_chain);

jive_shaped_node *
jive_shaped_node_create(jive_cut * cut, struct jive_node * node)
{
	jive_shaped_graph * shaped_graph = cut->shaped_region->shaped_graph;
	JIVE_DEBUG_ASSERT(!node->region->anchor || jive_shaped_graph_map_node(shaped_graph, node->region->anchor->node));
	jive_context * context = shaped_graph->context;
	jive_shaped_node * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->node = node;
	
	jive_shaped_node_hash_insert(&shaped_graph->node_map, self);
	
	self->cut = cut;
	
	jive_ssavar_xpoint_hash_init(&self->ssavar_xpoints, context);
	
	return self;
}

jive_shaped_node *
jive_shaped_node_prev_in_region(const jive_shaped_node * self)
{
	jive_shaped_node * tmp = self->cut_location_list.prev;
	jive_cut * cut = self->cut->region_cut_list.prev;
	
	while(cut && !tmp) {
		tmp = cut->locations.last;
		cut = cut->region_cut_list.prev;
	}
	
	return tmp;
}

jive_shaped_node *
jive_shaped_node_next_in_region(const jive_shaped_node * self)
{
	jive_shaped_node * tmp = self->cut_location_list.next;
	jive_cut * cut = self->cut->region_cut_list.next;
	
	while(cut && !tmp) {
		tmp = cut->locations.first;
		cut = cut->region_cut_list.next;
	}
	
	return tmp;
}

void
jive_shaped_node_destroy(jive_shaped_node * self)
{
	/* destroy node shapes of all anchored regions */
	size_t n;
	for(n = 0; n < self->node->ninputs; n++)
	{
		jive_input * input = self->node->inputs[n];
		if (jive_input_isinstance(input, &JIVE_CONTROL_INPUT)) {
			jive_region * region = input->origin->node->region;
			jive_shaped_region * shaped_region = jive_shaped_graph_map_region(self->shaped_graph, region);
			jive_shaped_region_destroy_cuts(shaped_region);
		}
	}
	
	/* remove from graph */
	jive_shaped_node_hash_remove(&self->shaped_graph->node_map, self);
	JIVE_LIST_REMOVE(self->cut->locations, self, cut_location_list);
	jive_ssavar_xpoint_hash_fini(&self->ssavar_xpoints);
	jive_context_free(self->shaped_graph->context, self);
}
