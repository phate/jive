#include <jive/vsdg/crossings-private.h>
#include <jive/util/list.h>

void
jive_node_resource_interaction_destroy(jive_node_resource_interaction * self)
{
	jive_context * context = self->node->graph->context;
	size_t hash_by_resource = ((size_t) self->resource) & (self->node->resource_interaction.nbuckets);
	
	JIVE_LIST_REMOVE(self->resource->node_interaction, self, same_resource_list);
	JIVE_LIST_REMOVE(self->node->resource_interaction.buckets[hash_by_resource], self, same_node_list);
	
	self->node->resource_interaction.nitems --;
	
	jive_context_free(context, self);
}

static void
rehash(jive_node * node)
{
	size_t new_nbuckets = node->resource_interaction.nitems * 2 + 1;
	size_t n;
	
	jive_node_resource_interaction_hash_bucket * new_buckets;
	new_buckets = jive_context_malloc(node->graph->context, new_nbuckets * sizeof(*new_buckets));
	
	for(n=0; n<new_nbuckets; n++) new_buckets[n].first = new_buckets[n].last = 0;
	
	for(n=0; n<node->resource_interaction.nbuckets; n++) {
		while(node->resource_interaction.buckets[n].first) {
			jive_node_resource_interaction * i = node->resource_interaction.buckets[n].first;
			JIVE_LIST_REMOVE(node->resource_interaction.buckets[n], i, same_node_list);
			
			size_t hash = ((size_t)i->resource) % new_nbuckets;
			JIVE_LIST_PUSH_BACK(new_buckets[hash], i, same_node_list);
		}
	}
	
	jive_context_free(node->graph->context, node->resource_interaction.buckets);
	node->resource_interaction.buckets = new_buckets;
	node->resource_interaction.nbuckets = new_nbuckets;
}

jive_node_resource_interaction *
jive_node_resource_interaction_create_slow(jive_node * node, jive_resource * resource)
{
	jive_context * context = node->graph->context;
	jive_node_resource_interaction * i = jive_context_malloc(context, sizeof(*i));
	
	i->node = node;
	i->resource = resource;
	i->before_count = i->crossed_count = i->after_count = 0;
	
	node->resource_interaction.nitems ++;
	if (node->resource_interaction.nitems > node->resource_interaction.nbuckets)
		rehash(node);
	
	size_t hash = ((size_t)resource) % node->resource_interaction.nbuckets;
	
	JIVE_LIST_PUSH_BACK(resource->node_interaction, i, same_resource_list);
	JIVE_LIST_PUSH_BACK(node->resource_interaction.buckets[hash], i, same_node_list);
	
	return i;
}
