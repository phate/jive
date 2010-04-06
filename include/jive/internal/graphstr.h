#ifndef JIVE_INTERNAL_GRAPHSTR_H
#define JIVE_INTERNAL_GRAPHSTR_H

#include <stdbool.h>
#include <jive/graph.h>

/* currently, maximum of 2 concurrent traversals */
#define CONCURRENT_TRAVERSALS 2

typedef struct _jive_node_head jive_node_head;
typedef struct _jive_node_traversal_state jive_node_traversal_state;
typedef struct _jive_edge_container jive_edge_container;

struct _jive_edgelist_anchor {
	jive_edge_container * prev, * next;
};

struct _jive_edgelist {
	jive_edge_container * first, * last;
};

struct jive_graph {
	jive_context * context;
	
	jive_node *top;
	jive_node *bottom;
	
	/** \brief Edges ready to be recycled */
	jive_edgelist unused;
	/** \brief Edges "coming from nowhere" (pointing to dangling nodes at top of graph) */
	jive_edgelist null_origin;
	/** \brief Edges "going to nowhere" (pointing to dangling nodes at bottom of graph) */
	jive_edgelist null_target;
	
	/* subgraph ? */
	
	jive_graph_traverser * cached_traversers;
	jive_graph_traverser * first_traverser,  * last_traverser;
	unsigned int traversal_cookies[CONCURRENT_TRAVERSALS];
	unsigned int allocated_traversers;
};

struct _jive_graph_traverser {
	jive_graph * graph;
	struct {
		jive_node * first, * last;
	} cut;
	short index, direction;
	unsigned int cookie;
	
	jive_graph_traverser * prev, * next;
};

#define JIVE_ITERATE_ACTIVE_TRAVERSERS(graph, trav) \
	for(trav=graph->first_traverser; unlikely(trav!=0); trav=trav->next)

struct _jive_node_traversal_state {
	jive_node * prev, * next;
	unsigned int cookie;
};

struct _jive_node_head {
	jive_edgelist input_edges;
	jive_edgelist output_edges;
	
	struct {jive_operand * first, * last;} inputs;
	struct {jive_value * first, * last;} outputs;
	
	struct _jive_node_passthrough_info * gates;
	
	jive_node_traversal_state traversal[CONCURRENT_TRAVERSALS];
	
	unsigned short reservation;
};

struct _jive_edge_container {
	jive_edge base;
	jive_edgelist_anchor input_list;
	jive_edgelist_anchor output_list;
};

static inline jive_node_head *
head_of_node(jive_node * node)
{
	return ((jive_node_head *)node)-1;
}

static inline jive_node_traversal_state *
traversal_state(jive_graph_traverser * traverser, jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	return &head->traversal[traverser->index];
}

static inline bool
node_behind_cut(jive_graph_traverser * traverser, jive_node * node)
{
	return traversal_state(traverser, node)->cookie == traverser->cookie;
}

static inline void
mark_node_behind_cut(jive_graph_traverser * traverser, jive_node * node)
{
	traversal_state(traverser, node)->cookie = traverser->cookie;
}

static inline bool
node_in_cut(jive_graph_traverser * traverser, jive_node * node)
{
	return traversal_state(traverser, node)->cookie == (traverser->cookie-1);
}

static inline void
mark_node_in_cut(jive_graph_traverser * traverser, jive_node * node)
{
	traversal_state(traverser, node)->cookie = traverser->cookie-1;
}

static inline bool
node_before_cut(jive_graph_traverser * traverser, jive_node * node)
{
	return traversal_state(traverser, node)->cookie < (traverser->cookie-1);
}

static inline void
mark_node_before_cut(jive_graph_traverser * traverser, jive_node * node)
{
	traversal_state(traverser, node)->cookie = traverser->cookie-2;
}

/* connect value to operand of node; internal use only */
void
jive_operand_value_connect(
	jive_node * value_node,
	jive_value * value,
	jive_node * operand_node,
	jive_operand * operand);

#endif
