#ifndef JIVE_ARCH_LOAD_H
#define JIVE_ARCH_LOAD_H

#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_LOAD_NODE;

typedef struct jive_load_node_attrs jive_load_node_attrs;
typedef struct jive_load_node jive_load_node;

struct jive_load_node_attrs {
	jive_node_attrs base;
	struct jive_value_type * datatype; /* note: dynamically allocated */
};

struct jive_load_node {
	jive_node base;
	jive_load_node_attrs attrs;
};

struct jive_node *
jive_load_by_address_node_create(struct jive_region * region,
	struct jive_output * address,
	const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_output *
jive_load_by_address_create(struct jive_output * address,
	const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_node *
jive_load_by_bitstring_node_create(struct jive_region * region,
	struct jive_output * address, size_t nbits,
	const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

struct jive_output *
jive_load_by_bitstring_create(struct jive_output * address,
	size_t nbits, const struct jive_value_type * datatype,
	size_t nstates, struct jive_output * const states[]);

JIVE_EXPORTED_INLINE jive_load_node *
jive_load_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_LOAD_NODE)
		return (jive_load_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_value_type *
jive_load_node_get_datatype(const jive_load_node * node)
{
	return node->attrs.datatype;
}

#endif
