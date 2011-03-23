#ifndef JIVE_REGALLOC_SHAPED_REGION_H
#define JIVE_REGALLOC_SHAPED_REGION_H

typedef struct jive_shaped_region jive_shaped_region;
typedef struct jive_cut jive_cut;

struct jive_shaped_graph;
struct jive_region;

struct jive_shaped_region {
	struct jive_shaped_graph * shaped_graph;
	
	struct jive_region * region;
	
	struct {
		jive_shaped_region * prev;
		jive_shaped_region * next;
	} hash_chain;
	
	struct {
		jive_cut * first;
		jive_cut * last;
	} cuts;
};

jive_shaped_region *
jive_shaped_region_create(struct jive_shaped_graph * shaped_graph, struct jive_region * region);

void
jive_shaped_region_destroy(jive_shaped_region * self);

#endif
