#ifndef JIVE_REGALLOC_SHAPED_NODE_H
#define JIVE_REGALLOC_SHAPED_NODE_H

typedef struct jive_shaped_node jive_shaped_node;

struct jive_shaped_graph;
struct jive_node;

struct jive_shaped_node {
	struct jive_shaped_graph * shaped_graph;
	
	struct jive_node * node;
	
	struct {
		jive_shaped_node * prev;
		jive_shaped_node * next;
	} hash_chain;
};

jive_shaped_node *
jive_shaped_node_create(struct jive_shaped_graph * shaped_graph, struct jive_node * node);

void
jive_shaped_node_destroy(jive_shaped_node * self);

#endif
