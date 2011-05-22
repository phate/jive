#ifndef JIVE_VSDG_SEQUENCE_H
#define JIVE_VSDG_SEQUENCE_H

#include <jive/util/hash.h>

struct jive_graph;
struct jive_node;
struct jive_region;

typedef struct jive_seq_graph jive_seq_graph;
typedef struct jive_seq_node jive_seq_node;
typedef struct jive_seq_region jive_seq_region;

struct jive_seq_node {
	struct jive_node * node;
	struct {
		jive_seq_node * prev;
		jive_seq_node * next;
	} seqnode_list;
	struct {
		jive_seq_node * prev;
		jive_seq_node * next;
	} hash_chain;
};

struct jive_seq_region {
	struct jive_region * region;
	struct {
		jive_seq_region * prev;
		jive_seq_region * next;
	} seqregion_list;
	struct {
		jive_seq_region * prev;
		jive_seq_region * next;
	} hash_chain;
	
	jive_seq_node * first_node;
	jive_seq_node * last_node;
};

typedef struct jive_seq_node_hash jive_seq_node_hash;
typedef struct jive_seq_region_hash jive_seq_region_hash;
JIVE_DECLARE_HASH_TYPE(jive_seq_node_hash, jive_seq_node, struct jive_node *, node, hash_chain);
JIVE_DECLARE_HASH_TYPE(jive_seq_region_hash, jive_seq_region, struct jive_region *, region, hash_chain);

struct jive_seq_graph {
	struct jive_context * context;
	struct jive_graph * graph;
	struct {
		jive_seq_node * first;
		jive_seq_node * last;
	} nodes;
	struct {
		jive_seq_region * first;
		jive_seq_region * last;
	} regions;
	
	jive_seq_node_hash node_map;
	jive_seq_region_hash region_map;
};

/**
	\brief Sequentialize graph
*/
jive_seq_graph *
jive_graph_sequentialize(struct jive_graph * graph);

void
jive_seq_graph_destroy(jive_seq_graph * seq);

jive_seq_node *
jive_seq_graph_map_node(const jive_seq_graph * seq, struct jive_node * node);

jive_seq_region *
jive_seq_graph_map_region(const jive_seq_graph * seq, struct jive_region * region);

#endif
