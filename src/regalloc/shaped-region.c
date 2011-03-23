#include <jive/regalloc/shaped-region.h>

#include <jive/context.h>
#include <jive/regalloc/shaped-graph.h>

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

void
jive_shaped_region_destroy(jive_shaped_region * self)
{
	jive_shaped_region_hash_remove(&self->shaped_graph->region_map, self);
	jive_context_free(self->shaped_graph->context, self);
}


