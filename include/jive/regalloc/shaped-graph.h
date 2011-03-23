#ifndef JIVE_REGALLOC_SHAPED_GRAPH_H
#define JIVE_REGALLOC_SHAPED_GRAPH_H

#include <jive/regalloc/shaped-region.h>

#include <jive/util/hash.h>

typedef struct jive_shaped_graph jive_shaped_graph;

JIVE_DECLARE_HASH_TYPE(jive_shaped_region_hash, jive_shaped_region, struct jive_region *, region, hash_chain);
typedef struct jive_shaped_region_hash jive_shaped_region_hash;

struct jive_graph;
struct jive_context;

struct jive_shaped_graph {
	struct jive_graph * graph;
	struct jive_context * context;
	
	struct jive_notifier * callbacks[21];
	
	jive_shaped_region_hash region_map;
};

jive_shaped_graph *
jive_shaped_graph_create(struct jive_graph * graph);

void
jive_shaped_graph_destroy(jive_shaped_graph * self);

struct jive_shaped_region *
jive_shaped_graph_map_region(const jive_shaped_graph * self, const struct jive_region * region);

#endif
