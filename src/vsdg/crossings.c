#include <jive/vsdg/crossings-private.h>
#include <jive/util/list.h>

void
jive_node_resource_interaction_destroy(jive_node_resource_interaction * self)
{
	jive_context * context = self->node->graph->context;
	
	JIVE_LIST_REMOVE(self->resource->node_interaction, self, same_resource_list);
	jive_resource_interaction_remove(&self->node->resource_interaction, self);
	
	jive_context_free(context, self);
}

jive_node_resource_interaction *
jive_node_resource_interaction_create_slow(jive_node * node, jive_resource * resource)
{
	jive_context * context = node->graph->context;
	jive_node_resource_interaction * i = jive_context_malloc(context, sizeof(*i));
	
	i->node = node;
	i->resource = resource;
	i->before_count = i->crossed_count = i->after_count = 0;
	
	jive_resource_interaction_insert(&node->resource_interaction, i);
	JIVE_LIST_PUSH_BACK(resource->node_interaction, i, same_resource_list);
	
	return i;
}
