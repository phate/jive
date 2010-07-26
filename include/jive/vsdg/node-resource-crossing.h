#ifndef JIVE_VSDG_NODE_RESOURCE_CROSSING_H
#define JIVE_VSDG_NODE_RESOURCE_CROSSING_H

#include <jive/context.h>

typedef struct jive_xpoint jive_xpoint;
typedef struct jive_crossed_nodes jive_crossed_nodes;
typedef struct jive_crossed_resources jive_crossed_resources;

struct jive_resource;
struct jive_node;

struct jive_xpoint {
	struct jive_node * node;
	struct jive_resource * resource;
	size_t count;
	struct {
		jive_xpoint * prev;
		jive_xpoint * next;
	} by_node;
	struct {
		jive_xpoint * prev;
		jive_xpoint * next;
	} by_resource;
};

struct jive_xpoint_hash {
	size_t nbuckets, nitems;
	jive_xpoint ** buckets);
};

static inline void
jive_xpoint_hash_init(jive_xpoint_hash * hash)
{
	hash->nbuckets = hash->nitems = 0;
	hash->buckets = 0;
}

static inline jive_xpoint **
jive_xpoint_hash_bucket(const jive_xpoint_hash * hash, void * key)
{
	if (!hash->nbuckets) return 0;
	return &hash->buckets[ (size_t)key % hash->nbuckets ];
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
	for(n=0; n<hash->nbuckets; n++) buckets[n] = 0;
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
	jive_xpoint ** node_resource_xpoints_bucket, ** resource_node_xpoints_bucket;
	
	node_resource_xpoints_bucket = jive_xpoint_hash_bucket(&node->resource_xpoints, resource);
	resource_node_xpoints_bucket = jive_xpoint_hash_bucket(&resource->node_xpoints, node);
	
	if (node_resource_xpoints_bucket) {
		jive_xpoint * xpoint = *node_resource_xpoints_bucket;
		while(xpoint) {
			if (xpoint->resource == resource) {
				size_t orig = xpoint->count;
				xpoint->count = orig + count;
				return orig;
			}
			xpoint = xpoint->by_resource.next;
		}
	}
	
	by_node.nitems ++;
	if (jive_xpoint_hash_resize(&resource->node_xpoints, context, true))
		resource_node_xpoints_bucket = jive_xpoint_hash_bucket(&resource->node_xpoints, node);
	
	by_resource.nitems ++;
	if (jive_xpoint_hash_resize(&node->resource_xpoints, context, false))
		node_resource_xpoints_bucket = jive_xpoint_hash_bucket(&node->resource_xpoints, resource);
	
	jive_context * context = node->graph->context;
	
	xpoint = jive_context_malloc(context, sizeof(*xpoint));
	xpoint->node = node;
	xpoint->resource = resource;
	xpoint->count = count;
	xpoint->by_node.next = *resource_node_xpoints_bucket;
	xpoint->by_res.next = *node_resource_xpoints_bucket;
	
	if (*resource_node_xpoints_bucket) (*resource_node_xpoints_bucket)->by_node.prev = xpoint;
	if (*node_resource_xpoints_bucket) (*node_resource_xpoints_bucket)->by_res.prev = xpoint;
	*resource_node_xpoints_bucket = xpoint;
	*node_resource_xpoints_bucket = xpoint;
	
	return 0;
}

static inline size_t
jive_node_resource_crossing_del(
	jive_node * node,
	jive_resource * resource,
	size_t count)
{
	jive_xpoint ** node_resource_xpoints_bucket, ** resource_node_xpoints_bucket;
	
	node_resource_xpoints_bucket = jive_xpoint_hash_bucket(&node->resource_xpoints, resource);
	resource_node_xpoints_bucket = jive_xpoint_hash_bucket(&resource->node_xpoints, node);
	
	DEBUG_ASSERT(node_resource_xpoints_bucket);
	DEBUG_ASSERT(resource_node_xpoints_bucket);
	
	jive_xpoint * xpoint = *node_resource_xpoints_bucket;
	while(xpoint) {
		if (xpoint->resource == resource) break;
		xpoint = xpoint->by_resource.next;
	}
	
	DEBUG_ASSERT(xpoint);
	DEBUG_ASSERT(xpoint->count >= count);
	xpoint->count -= count;
	count = xpoint->count;
	
	if (count) return count;
	
	node->resource_xpoints.nitems --;
	resource->node_xpoints.nitems --;
	
	if (xpoint->by_node.prev) xpoint->by_node.prev->by_node.next = xpoint->by_node.next;
	else *resource_node_xpoints_bucket = xpoint->by_node.next;
	if (xpoint->by_node.next) xpoint->by_node.next->by_node.prev = xpoint->by_node.prev;
	
	if (xpoint->by_res.prev) xpoint->by_res.prev->by_res.next = xpoint->by_res.next;
	else *node_resource_xpoints_bucket = xpoint->by_res.next;
	if (xpoint->by_res.next) xpoint->by_res.next->by_res.prev = xpoint->by_res.prev;
	
	jive_context * context = node->graph->context;
	
	jive_context_free(context, xpoint);
	
	return 0;
}

#endif
