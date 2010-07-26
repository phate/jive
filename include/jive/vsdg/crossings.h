#ifndef JIVE_VSDG_CROSSINGS_H
#define JIVE_VSDG_CROSSINGS_H

#include <jive/context.h>

typedef struct jive_xpoint jive_xpoint;
typedef struct jive_xpoint_hash jive_xpoint_hash;
typedef struct jive_xpoint_hash jive_crossed_nodes;
typedef struct jive_xpoint_hash jive_crossed_resources;

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
	jive_xpoint ** buckets;
};

static inline jive_xpoint **
jive_xpoint_hash_bucket(const jive_xpoint_hash * hash, void * key)
{
	if (!hash->nbuckets) return 0;
	return &hash->buckets[ (size_t)key % hash->nbuckets ];
}

#endif
