#ifndef JIVE_ARCH_LOADSTORE_H
#define JIVE_ARCH_LOADSTORE_H

#include <jive/common.h>
#include <jive/vsdg/node.h>

struct jive_context;
struct jive_type;

extern const jive_node_class JIVE_LOAD_NODE;
extern const jive_node_class JIVE_STORE_NODE;

typedef struct jive_load_node_attrs jive_load_node_attrs;
typedef struct jive_store_node_attrs jive_store_node_attrs;

typedef struct jive_load_node jive_load_node;
typedef struct jive_store_node jive_store_node;

struct jive_load_node_attrs {
	jive_node_attrs base;
	struct jive_value_type * datatype; /* note: dynamically allocated */
};

struct jive_store_node_attrs {
	jive_node_attrs base;
	struct jive_value_type * datatype; /* note: dynamically allocated */
};

struct jive_load_node {
	jive_node base;
	jive_load_node_attrs attrs;
};

struct jive_store_node {
	jive_node base;
	jive_store_node_attrs attrs;
};

struct jive_node *
jive_load_node_create(struct jive_region * region,
	struct jive_output * address,
	const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_output *
jive_load_create(struct jive_output * address,
	const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

JIVE_EXPORTED_INLINE jive_load_node *
jive_load_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_LOAD_NODE)
		return (jive_load_node *) node;
	else
		return NULL;
}

struct jive_node *
jive_store_node_create(struct jive_region * region,
	struct jive_output * address,
	const struct jive_value_type * datatype, struct jive_output * value,
	size_t nstates, struct jive_output * const states[]);

JIVE_EXPORTED_INLINE jive_store_node *
jive_store_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_STORE_NODE)
		return (jive_store_node *) node;
	else
		return NULL;
}

#endif
