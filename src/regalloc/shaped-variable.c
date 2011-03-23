#include <jive/regalloc/shaped-variable.h>

#include <jive/context.h>
#include <jive/regalloc/shaped-graph.h>

JIVE_DEFINE_HASH_TYPE(jive_shaped_variable_hash, jive_shaped_variable, struct jive_variable *, variable, hash_chain);

jive_shaped_variable *
jive_shaped_variable_create(struct jive_shaped_graph * shaped_graph, struct jive_variable * variable)
{
	jive_context * context = shaped_graph->context;
	jive_shaped_variable * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->variable = variable;
	
	jive_shaped_variable_hash_insert(&shaped_graph->variable_map, self);
	
	return self;
}

void
jive_shaped_variable_destroy(jive_shaped_variable * self)
{
	jive_shaped_variable_hash_remove(&self->shaped_graph->variable_map, self);
	jive_context_free(self->shaped_graph->context, self);
}

JIVE_DEFINE_HASH_TYPE(jive_shaped_ssavar_hash, jive_shaped_ssavar, struct jive_ssavar *, ssavar, hash_chain);

jive_shaped_ssavar *
jive_shaped_ssavar_create(struct jive_shaped_graph * shaped_graph, struct jive_ssavar * ssavar)
{
	jive_context * context = shaped_graph->context;
	jive_shaped_ssavar * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->ssavar = ssavar;
	
	jive_shaped_ssavar_hash_insert(&shaped_graph->ssavar_map, self);
	
	return self;
}

void
jive_shaped_ssavar_destroy(jive_shaped_ssavar * self)
{
	jive_shaped_ssavar_hash_remove(&self->shaped_graph->ssavar_map, self);
	jive_context_free(self->shaped_graph->context, self);
}
