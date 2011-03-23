#include <jive/regalloc/shaped-graph.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>

JIVE_DEFINE_HASH_TYPE(jive_shaped_region_hash, jive_shaped_region, jive_region *, region, hash_chain);

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
	
	jive_shaped_region_hash_init(&self->region_map, context);
	
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
	
	struct jive_shaped_region_hash_iterator region_iter;
	region_iter = jive_shaped_region_hash_begin(&self->region_map);
	while(region_iter.entry) {
		struct jive_shaped_region_hash_iterator next = region_iter;
		jive_shaped_region_hash_iterator_next(&next);
		jive_shaped_region_destroy(region_iter.entry);
		region_iter = next;
	}
	
	jive_shaped_region_hash_fini(&self->region_map);
	
	jive_context_free(context, self);
}

jive_shaped_region *
jive_shaped_graph_map_region(const jive_shaped_graph * self, const jive_region * region)
{
	return jive_shaped_region_hash_lookup(&self->region_map, region);
}
