#ifndef JIVE_REGALLOC_SHAPED_GRAPH_H
#define JIVE_REGALLOC_SHAPED_GRAPH_H

#include <jive/util/hash.h>

typedef struct jive_shaped_graph jive_shaped_graph;

struct jive_graph;
struct jive_context;

struct jive_shaped_graph {
	struct jive_graph * graph;
	struct jive_context * context;
	
	struct jive_notifier * callbacks[21];
};

jive_shaped_graph *
jive_shaped_graph_create(struct jive_graph * graph);

void
jive_shaped_graph_destroy(jive_shaped_graph * self);

#endif
