#ifndef JIVE_VSDG_SEQUENCE_H
#define JIVE_VSDG_SEQUENCE_H

struct jive_node;
struct jive_graph;

typedef struct jive_seq_node jive_seq_node;
typedef struct jive_seq_graph jive_seq_graph;

struct jive_seq_node {
	struct jive_node * node;
	struct {
		jive_seq_node * prev;
		jive_seq_node * next;
	} seqnode_list;
};

struct jive_seq_graph {
	struct jive_context * context;
	struct jive_graph * graph;
	struct {
		jive_seq_node * first;
		jive_seq_node * last;
	} nodes;
};

/**
	\brief Sequentialize graph
*/
jive_seq_graph *
jive_graph_sequentialize(struct jive_graph * graph);

void
jive_seq_graph_destroy(jive_seq_graph * seq);

#endif
