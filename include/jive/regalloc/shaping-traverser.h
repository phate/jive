#ifndef JIVE_REGALLOC_SHAPE_H
#define JIVE_REGALLOC_SHAPE_H

struct jive_graph;
struct jive_region;
struct jive_notifier;
struct jive_traversal_nodestate;

typedef struct jive_shaping_traverser jive_shaping_traverser;
typedef struct jive_shaping_region_traverser jive_shaping_region_traverser;
typedef struct jive_shaping_traverser_bucket jive_shaping_traverser_bucket;

jive_shaping_region_traverser *
jive_shaping_region_traverser_create(struct jive_graph * graph);

void
jive_shaping_region_traverser_destroy(jive_shaping_region_traverser * self);

struct jive_traverser *
jive_shaping_region_traverser_enter_region(jive_shaping_region_traverser * self, struct jive_region * region);

#endif
