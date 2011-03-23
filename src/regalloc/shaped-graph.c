#include <jive/regalloc/shaped-graph.h>

#include <jive/common.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/variable.h>

JIVE_DEFINE_HASH_TYPE(jive_shaped_region_hash, jive_shaped_region, jive_region *, region, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_shaped_variable_hash, jive_shaped_variable, struct jive_variable *, variable, hash_chain);
JIVE_DEFINE_HASH_TYPE(jive_shaped_ssavar_hash, jive_shaped_ssavar, struct jive_ssavar *, ssavar, hash_chain);

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
	self->callbacks[n++] = jive_variable_notifier_slot_connect(&graph->on_variable_create, jive_shaped_graph_variable_create, self);
	self->callbacks[n++] = jive_variable_notifier_slot_connect(&graph->on_variable_destroy, jive_shaped_graph_variable_destroy, self);
	self->callbacks[n++] = jive_ssavar_notifier_slot_connect(&graph->on_ssavar_create, jive_shaped_graph_ssavar_create, self);
	self->callbacks[n++] = jive_ssavar_notifier_slot_connect(&graph->on_ssavar_destroy, jive_shaped_graph_ssavar_destroy, self);
	
	JIVE_DEBUG_ASSERT(n <= sizeof(self->callbacks)/sizeof(self->callbacks[0]));
	
	jive_shaped_region_hash_init(&self->region_map, context);
	jive_shaped_variable_hash_init(&self->variable_map, context);
	jive_shaped_ssavar_hash_init(&self->ssavar_map, context);
	
	jive_variable * variable;
	JIVE_LIST_ITERATE(graph->variables, variable, graph_variable_list) {
		jive_shaped_variable_create(self, variable);
		jive_ssavar * ssavar;
		JIVE_LIST_ITERATE(variable->ssavars, ssavar, variable_ssavar_list) {
			jive_shaped_ssavar_create(self, ssavar);
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
	
	jive_shaped_ssavar_hash_fini(&self->ssavar_map);
	jive_shaped_variable_hash_fini(&self->variable_map);
	jive_shaped_region_hash_fini(&self->region_map);
	
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
