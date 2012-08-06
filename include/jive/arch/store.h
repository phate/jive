#ifndef JIVE_ARCH_STORE_H
#define JIVE_ARCH_STORE_H

#include <jive/vsdg/node.h>

extern const jive_node_class JIVE_STORE_NODE;

typedef struct jive_store_node_attrs jive_store_node_attrs;
typedef struct jive_store_node jive_store_node;

struct jive_store_node_attrs {
	jive_node_attrs base;
	struct jive_value_type * datatype; /* note: dynamically allocated */
};

struct jive_store_node {
	jive_node base;
	jive_store_node_attrs attrs;
};

struct jive_node *
jive_store_by_address_node_create(struct jive_region * region,
	struct jive_output * address,
	const struct jive_value_type * datatype, struct jive_output * value,
	size_t nstates, struct jive_output * const states[]);

struct jive_output * const *
jive_store_by_address_create(struct jive_output * address,
	const struct jive_value_type * datatype, struct jive_output * value,
	size_t nstates, struct jive_output * const states[]);

struct jive_node *
jive_store_by_bitstring_node_create(struct jive_region * region,
	struct jive_output * address, size_t nbits,
	const struct jive_value_type * datatype, struct jive_output * value,
	size_t nstates, struct jive_output * const states[]);

struct jive_output * const *
jive_store_by_bitstring_create(struct jive_output * address, size_t nbits,
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

JIVE_EXPORTED_INLINE const struct jive_value_type *
jive_store_node_get_datatype(const jive_store_node * node)
{
	return node->attrs.datatype;
}

#endif
