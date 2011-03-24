#include <jive/regalloc/shaped-node.h>

#include <jive/context.h>
#include <jive/regalloc/shaped-graph.h>

JIVE_DEFINE_HASH_TYPE(jive_shaped_node_hash, jive_shaped_node, struct jive_node *, node, hash_chain);

jive_shaped_node *
jive_shaped_node_create(struct jive_shaped_graph * shaped_graph, struct jive_node * node)
{
	jive_context * context = shaped_graph->context;
	jive_shaped_node * self = jive_context_malloc(context, sizeof(*self));
	
	self->shaped_graph = shaped_graph;
	self->node = node;
	
	jive_shaped_node_hash_insert(&shaped_graph->node_map, self);
	
	return self;
}

void
jive_shaped_node_destroy(jive_shaped_node * self)
{
	jive_shaped_node_hash_remove(&self->shaped_graph->node_map, self);
	jive_context_free(self->shaped_graph->context, self);
}
