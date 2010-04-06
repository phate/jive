#ifndef JIVE_GRAPH_H
#define JIVE_GRAPH_H

#include <stdbool.h>
#include <jive/types.h>
#include <jive/context.h>

/**
	\defgroup jive_graph Graphs
	Graphs
	@{
*/

/** \brief Graph structure */
typedef struct jive_graph jive_graph;

/**
	\brief Create graph
	\param context Context to create graph in
	\return Newly allocated graph
*/
jive_graph *
jive_graph_create(jive_context * context);

/**
	\brief Duplicate graph structure
	\param context Context to create new graph in
	\param original Graph to duplicate
	\return Newly allocated graph
*/
jive_graph *
jive_graph_copy(jive_context * context, const jive_graph * original);

/** \brief Allocate memory in the context of this graph */
void *
jive_malloc(jive_graph * graph, size_t count);

/** \brief Allocate memory in the context of this graph and copy string */
const char *
jive_strdup(jive_graph * graph, const char * src);

/** \brief Terminate operation on the graph with a fatal error */
void
jive_graph_fatal_error(jive_graph * graph, const char * message);

/** \brief Retrieve endian setting of this graph */
jive_endian
jive_graph_endian(const jive_graph * graph);

/** \brief Remove dangling nodes from bottom of graph, recursively */
void
jive_graph_prune(jive_graph * graph);

/**
	\defgroup jive_nodes Nodes
	Nodes
	@{
*/

typedef struct _jive_node_class jive_node_class;

bool
jive_node_is_instance(const jive_node * node, const jive_node_class * type);

/* mark inputs as changed, invalidate internal state */
void
jive_node_invalidate(jive_node * node);

/* force recomputing internal state after invalidation */
void
jive_node_revalidate(jive_node * node);

void
jive_node_reserve(jive_node * node);

void
jive_node_unreserve(jive_node * node);

void
jive_node_prune_recursive(jive_node * node);

void
jive_node_add_operand(jive_node * node, jive_operand * input);

/**
	\defgroup jive_values Node output values
	Node output values
	@{
*/

typedef struct _jive_value_class jive_value_class;
extern const jive_value_class JIVE_VALUE_BASE;

void
jive_value_init(jive_value * output, jive_node * node);

/** \brief Replace all instances of "old_value" with "new_value" */
void
jive_value_replace(jive_value * old_value, jive_value * new_value);

bool
jive_value_is_instance(const jive_value * value, const jive_value_class * type);

/* auxiliary state */

jive_cpureg_class_t
jive_value_get_cpureg_class(const jive_value * value);

void
jive_value_set_cpureg_class(jive_value * value, jive_cpureg_class_t regcls);

jive_cpureg_t
jive_value_get_cpureg(const jive_value * value);

void
jive_value_set_cpureg(jive_value * value, jive_cpureg_t reg);

jive_passthrough *
jive_value_get_passthrough(const jive_value * value);

jive_regalloc_regstate
jive_value_get_regalloc_regstate(const jive_value * value);

void
jive_value_set_regalloc_regstate(jive_value * value, jive_regalloc_regstate state);

bool
jive_value_get_mayspill(const jive_value * value);

void
jive_value_set_mayspill(jive_value * value, bool may_spill);

/* iterating outputs */

jive_value *
jive_node_iterate_values(jive_node * node);

/**	@}	*/

/**
	\defgroup jive_operands Operands
	Operands
	@{
*/

typedef struct _jive_operand_class jive_operand_class;
typedef struct _jive_operand_list jive_operand_list;

extern const jive_operand_class JIVE_INPUT_BASE;

void
jive_operand_init(jive_operand * input, jive_value * inout);

/* auxiliary state */

jive_cpureg_class_t
jive_operand_get_cpureg_class(const jive_operand * input);

void
jive_operand_set_cpureg_class(jive_operand * input, jive_cpureg_class_t regcls);

jive_cpureg_class_t
jive_value_get_cpureg_class_shared(const jive_value * value);

void
jive_value_set_cpureg_class_shared(jive_value * value, jive_cpureg_class_t regcls);

jive_passthrough *
jive_operand_get_passthrough(const jive_operand * input);

/* iterating input ports */

jive_operand *
jive_node_iterate_operands(jive_node * node);

/** @} */

/** @} */

/**
	\defgroup jive_edges Edges
	Edges
	@{
*/

typedef struct jive_edge jive_edge;
typedef jive_edge * jive_input_edge_iterator;
typedef jive_edge * jive_output_edge_iterator;

typedef struct _jive_edgelist_anchor jive_edgelist_anchor;
typedef struct _jive_edgelist jive_edgelist;

typedef struct _jive_graph_traverser jive_graph_traverser;

jive_edge *
jive_state_edge_create(jive_node * origin, jive_node * target);

bool
jive_edge_is_state_edge(const jive_edge * edge);

void
jive_edge_divert_origin(jive_edge * edge, jive_value * new_value);

void
jive_state_edge_remove(jive_edge * edge);

jive_input_edge_iterator
jive_node_iterate_inputs(jive_node * node);

jive_input_edge_iterator
jive_input_edge_iterator_next(jive_input_edge_iterator iterator);

jive_output_edge_iterator
jive_node_iterate_outputs(jive_node * node);

jive_output_edge_iterator
jive_output_edge_iterator_next(jive_output_edge_iterator iterator);

#define JIVE_ITERATE_INPUTS(iterator, node) \
	for(iterator = jive_node_iterate_inputs(node); iterator; iterator = jive_input_edge_iterator_next(iterator))

#define JIVE_ITERATE_OUTPUTS(iterator, node) \
	for(iterator = jive_node_iterate_outputs(node); iterator; iterator = jive_output_edge_iterator_next(iterator))

jive_output_edge_iterator
jive_graph_iterate_top(jive_graph * graph);

#define JIVE_ITERATE_TOP(iterator, graph) \
	for(iterator = jive_graph_iterate_top(graph); iterator; iterator = jive_output_edge_iterator_next(iterator))

jive_input_edge_iterator
jive_graph_iterate_bottom(jive_graph * graph);

#define JIVE_ITERATE_BOTTOM(iterator, graph) \
	for(iterator = jive_graph_iterate_bottom(graph); iterator; iterator = jive_input_edge_iterator_next(iterator))

/**	@}	*/

/**
	\defgroup jive_traverser Traversers
	Traversers
	@{
*/

jive_graph_traverser *
jive_graph_traverse_topdown(jive_graph * graph);

jive_graph_traverser *
jive_graph_traverse_bottomup(jive_graph * graph);

void
jive_graph_traverse_finish(jive_graph_traverser * traverser);

jive_node *
jive_graph_traverse_next(jive_graph_traverser * traverser);

/**	@}	*/

/**	@}	*/


/* public data structures */

/** \brief Node */
struct jive_node {
	const jive_node_class * type;
	jive_graph * graph;
	size_t depth_from_root;
};

/** \brief Edge */
struct jive_edge {
	struct {
		jive_node * node;
		jive_value * port;
	} origin;
	struct {
		jive_node * node;
		jive_operand * port;
	} target;
};

#endif
