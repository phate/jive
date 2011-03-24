#include <jive/regalloc/shaped-region.h>

#include <jive/context.h>
#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/shaped-node-private.h>
#include <jive/vsdg/node.h>

static jive_cut *
jive_cut_create(jive_context * context, jive_shaped_region * shaped_region, jive_cut * before)
{
	jive_cut * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_region = shaped_region;
	JIVE_LIST_INSERT(shaped_region->cuts, before, self, region_cut_list);
	self->locations.first = self->locations.last = 0;
	
	return self;
}

JIVE_DEFINE_HASH_TYPE(jive_shaped_region_hash, jive_shaped_region, struct jive_region *, region, hash_chain);

jive_shaped_region *
jive_shaped_region_create(struct jive_shaped_graph * shaped_graph, struct jive_region * region)
{
	jive_context * context = shaped_graph->context;
	jive_shaped_region * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->region = region;
	self->cuts.first = self->cuts.last = NULL;
	
	jive_shaped_region_hash_insert(&shaped_graph->region_map, self);
	
	return self;
}

jive_cut *
jive_shaped_region_create_cut(jive_shaped_region * self)
{
	return jive_cut_create(self->shaped_graph->context, self, self->cuts.first);
}

jive_shaped_node *
jive_shaped_region_first(const jive_shaped_region * self)
{
	jive_cut * cut = self->cuts.first;
	while(cut && !cut->locations.first)
		cut = cut->region_cut_list.next;
	if (cut) return cut->locations.first;
	return 0;
}

jive_shaped_node *
jive_shaped_region_last(const jive_shaped_region * self)
{
	jive_cut * cut = self->cuts.last;
	while(cut && !cut->locations.last)
		cut = cut->region_cut_list.prev;
	if (cut) return cut->locations.last;
	return 0;
}

void
jive_shaped_region_destroy_cuts(jive_shaped_region * self)
{
	while(self->cuts.first)
		jive_cut_destroy(self->cuts.first);
}

void
jive_shaped_region_destroy(jive_shaped_region * self)
{
	jive_shaped_region_destroy_cuts(self);
	jive_shaped_region_hash_remove(&self->shaped_graph->region_map, self);
	jive_context_free(self->shaped_graph->context, self);
}

void
jive_cut_destroy(jive_cut * self)
{
	jive_context * context = self->shaped_region->shaped_graph->context;
	
	while(self->locations.first)
		jive_shaped_node_destroy(self->locations.first);
	
	JIVE_LIST_REMOVE(self->shaped_region->cuts, self, region_cut_list);
	
	jive_context_free(context, self);
}

jive_cut *
jive_cut_create_above(jive_cut * self)
{
	return jive_cut_create(self->shaped_region->shaped_graph->context, self->shaped_region, self);
}

jive_cut *
jive_cut_create_below(jive_cut * self)
{
	return jive_cut_create(self->shaped_region->shaped_graph->context, self->shaped_region, self->region_cut_list.next);
}

struct jive_shaped_node *
jive_cut_insert(jive_cut * self, jive_shaped_node * before, jive_node * node)
{
	jive_shaped_node * shaped_node = jive_shaped_node_create(self, node);
	
	JIVE_LIST_INSERT(self->locations, before, shaped_node, cut_location_list);
	
	return shaped_node;
}
