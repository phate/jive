#ifndef JIVE_VSDG_CROSSINGS_PRIVATE_H
#define JIVE_VSDG_CROSSINGS_PRIVATE_H

#include <jive/context.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/graph.h>
#include <jive/debug-private.h>

static inline void
jive_xpoint_hash_init(jive_xpoint_hash * hash)
{
	hash->nbuckets = hash->nitems = 0;
	hash->buckets = 0;
}

static inline bool
jive_xpoint_hash_resize(jive_xpoint_hash * hash, jive_context * context, bool indexed_by_node)
{
	if (hash->nbuckets >= hash->nitems) return false;
	jive_xpoint ** old_buckets = hash->buckets;
	size_t old_nbuckets = hash->nbuckets;
	
	size_t new_size = hash->nitems * 2 + 1, n;
	hash->buckets = jive_context_malloc(context, new_size * sizeof(jive_xpoint *));
	hash->nbuckets = new_size;
	for(n=0; n<hash->nbuckets; n++) hash->buckets[n] = 0;
	for(n=0; n<old_nbuckets; n++) {
		jive_xpoint * xpoint = old_buckets[n];
		while(xpoint) {
			if (indexed_by_node) {
				old_buckets[n] = xpoint->by_node.next;
				jive_xpoint ** bucket = jive_xpoint_hash_bucket(hash, xpoint->node);
				xpoint->by_node.prev = * bucket;
				xpoint->by_node.next = 0;
				*bucket = xpoint;
			} else {
				old_buckets[n] = xpoint->by_resource.next;
				jive_xpoint ** bucket = jive_xpoint_hash_bucket(hash, xpoint->resource);
				xpoint->by_resource.prev = * bucket;
				xpoint->by_resource.next = 0;
				*bucket = xpoint;
			}
			
			xpoint = old_buckets[n];
		}
	}
	
	jive_context_free(context, old_buckets);
	
	return true;
}

static inline size_t
jive_node_resource_crossing_add(
	jive_node * node,
	jive_resource * resource,
	size_t count)
{
	jive_xpoint ** node_resouce_crossings_bucket, ** resource_node_crossings_bucket;
	
	node_resouce_crossings_bucket = jive_xpoint_hash_bucket(&node->resource_crossings, resource);
	resource_node_crossings_bucket = jive_xpoint_hash_bucket(&resource->node_crossings, node);
	
	if (node_resouce_crossings_bucket) {
		jive_xpoint * xpoint = *node_resouce_crossings_bucket;
		while(xpoint) {
			if (xpoint->resource == resource) {
				size_t orig = xpoint->count;
				xpoint->count = orig + count;
				return orig;
			}
			xpoint = xpoint->by_resource.next;
		}
	}
	
	jive_context * context = node->graph->context;
	
	resource->node_crossings.nitems ++;
	if (jive_xpoint_hash_resize(&resource->node_crossings, context, true))
		resource_node_crossings_bucket = jive_xpoint_hash_bucket(&resource->node_crossings, node);
	
	node->resource_crossings.nitems ++;
	if (jive_xpoint_hash_resize(&node->resource_crossings, context, false))
		node_resouce_crossings_bucket = jive_xpoint_hash_bucket(&node->resource_crossings, resource);
	
	jive_xpoint * xpoint = jive_context_malloc(context, sizeof(*xpoint));
	xpoint->node = node;
	xpoint->resource = resource;
	xpoint->count = count;
	xpoint->by_node.next = *resource_node_crossings_bucket;
	xpoint->by_resource.next = *node_resouce_crossings_bucket;
	
	if (*resource_node_crossings_bucket) (*resource_node_crossings_bucket)->by_node.prev = xpoint;
	if (*node_resouce_crossings_bucket) (*node_resouce_crossings_bucket)->by_resource.prev = xpoint;
	*resource_node_crossings_bucket = xpoint;
	*node_resouce_crossings_bucket = xpoint;
	
	return 0;
}

static inline size_t
jive_node_resource_crossing_del(
	jive_node * node,
	jive_resource * resource,
	size_t count)
{
	jive_xpoint ** node_resource_crossings_bucket, ** resource_node_crossings_bucket;
	
	node_resource_crossings_bucket = jive_xpoint_hash_bucket(&node->resource_crossings, resource);
	resource_node_crossings_bucket = jive_xpoint_hash_bucket(&resource->node_crossings, node);
	
	DEBUG_ASSERT(node_resource_crossings_bucket);
	DEBUG_ASSERT(resource_node_crossings_bucket);
	
	jive_xpoint * xpoint = *node_resource_crossings_bucket;
	while(xpoint) {
		if (xpoint->resource == resource) break;
		xpoint = xpoint->by_resource.next;
	}
	
	DEBUG_ASSERT(xpoint);
	DEBUG_ASSERT(xpoint->count >= count);
	xpoint->count -= count;
	count = xpoint->count;
	
	if (count) return count;
	
	node->resource_crossings.nitems --;
	resource->node_crossings.nitems --;
	
	if (xpoint->by_node.prev) xpoint->by_node.prev->by_node.next = xpoint->by_node.next;
	else *resource_node_crossings_bucket = xpoint->by_node.next;
	if (xpoint->by_node.next) xpoint->by_node.next->by_node.prev = xpoint->by_node.prev;
	
	if (xpoint->by_resource.prev) xpoint->by_resource.prev->by_resource.next = xpoint->by_resource.next;
	else *node_resource_crossings_bucket = xpoint->by_resource.next;
	if (xpoint->by_resource.next) xpoint->by_resource.next->by_resource.prev = xpoint->by_resource.prev;
	
	jive_context * context = node->graph->context;
	
	jive_context_free(context, xpoint);
	
	return 0;
}

#endif
