#ifndef JIVE_NODECLASS_H
#define JIVE_NODECLASS_H

#include <alloca.h>

#include <jive/graph.h>

typedef enum {
	jive_node_class_associative=1,
	jive_node_class_commutative=2
} jive_node_class_flags;

struct _jive_node_class {
	const jive_node_class * parent;
	const char * name;
	size_t size;
	jive_node_class_flags flags;
	
	/** \brief Give textual representation of node (for debugging) */
	char * (*repr)(const void * self);
	/** \brief Test for equivalence with another node */
	bool (*equiv)(const void * self, const void * other);
	/** \brief Invalidate any computed state depending on inputs (i.e. value range) */
	void (*invalidate_inputs)(void * self);
	/** \brief Recompute any auxiliary state of outputs */
	void (*revalidate_outputs)(void * self);
};

struct _jive_value_class {
	jive_value_class * parent;
	const char * name;
	size_t size;
	
	char * (*repr)(const void * self);
};

typedef struct jive_value_extra jive_value_extra;

#define JIVE_VALUE_COMMON_FIELDS \
	const jive_value_class * type; \
	jive_node * node; \
	jive_value * next; \
	jive_value_extra * extra;

struct jive_value {
	JIVE_VALUE_COMMON_FIELDS
};

struct jive_value_extra {
	jive_passthrough * passthrough;
	/* the assigned cpu register */
	jive_cpureg_class_t cpureg_cls;
	/* the register class required by the defining instruction */
	jive_cpureg_t cpureg;
	/* register class satisfying requirements of defining
	and using instructions (constructed incrementally) */
	jive_cpureg_class_t cpureg_cls_shared;
	jive_regalloc_regstate ra_state;
	bool may_spill;
};

static const jive_value_extra jive_value_extra_default = {
	0, 0, 0, 0, 0, true
};

static inline jive_value_extra *
jive_value_get_extra(jive_value * value)
{
	if (!value->extra) {
		value->extra = jive_malloc(value->node->graph, sizeof(jive_value_extra));
		*(value->extra) = jive_value_extra_default;
	}
	return value->extra;
}

static inline const jive_value_extra *
jive_value_get_extra_ro(const jive_value * value)
{
	if (value->extra) return value->extra;
	else return &jive_value_extra_default;
}

struct _jive_operand_class {
	jive_operand_class * parent;
	const char * name;
	size_t size;
	
	char * (*repr)(const void * self);
};

typedef struct _jive_operand_extra jive_operand_extra;

#define JIVE_OPERAND_COMMON_FIELDS(value_type) \
	const jive_operand_class * type; \
	value_type * value; \
	jive_operand * next; \
	jive_operand_extra * extra;

struct jive_operand {
	JIVE_OPERAND_COMMON_FIELDS(jive_value)
};

struct _jive_operand_extra {
	jive_passthrough * passthrough;
	jive_cpureg_class_t cpureg_cls;
};

static const jive_operand_extra jive_operand_extra_default = {
	0, 0
};

static inline jive_operand_extra *
jive_input_get_extra(jive_operand * input)
{
	if (!input->extra) {
		input->extra = jive_malloc(input->value->node->graph, sizeof(jive_operand_extra));
		*(input->extra) = jive_operand_extra_default;
	}
	return input->extra;
}

static inline const jive_operand_extra *
jive_input_get_extra_ro(const jive_operand * input)
{
	if (input->extra) return input->extra;
	else return &jive_operand_extra_default;
}

/**
	\brief Create input ports
*/
jive_operand_list *
jive_operand_list_create(jive_graph *graph, size_t ninputs, jive_value * const outputs[]);

/**
	\brief Allocate node for graph with inputs
*/
jive_node *
jive_node_create(jive_graph *graph, const jive_node_class * type, size_t ninputs, jive_operand_list * inputs);

/**
	\ingroup jive_nodes
	\brief Add output port to node
*/
void
jive_node_add_value(jive_node * node, jive_value * value);

/**
	\brief Allocate node for graph with inputs
*/
jive_node *
jive_node_alloc(jive_graph * graph, const jive_node_class * type, size_t ninputs, jive_value * const inputs[]);

/**
	\brief Expand inputs of specified class
*/
#define JIVE_EXPAND_INPUTS(port_struct, ninputs, inputs, NODE_CLASS, get_ninputs, get_input) \
do { \
	size_t n = 0; \
	while(n<ninputs) { \
		jive_node * subnode = ((jive_value *)inputs[n])->node; \
		if (subnode->type == &NODE_CLASS) { \
			size_t count = get_ninputs(subnode); \
			size_t total = ninputs-1 + count, k; \
			port_struct ** tmp = alloca(sizeof(port_struct *)*total); \
			 \
			size_t p = 0; \
			for(k=0; k<n; k++) tmp[p++] = inputs[k]; \
			for(k=0; k<count; k++) \
				tmp[p++] = (port_struct *) get_input(subnode, k); \
			for(k=n+1; k<ninputs; k++) tmp[p++] = inputs[k]; \
			 \
			inputs = tmp; \
			ninputs = total; \
		} else n++; \
	} \
} while(0)

#endif
