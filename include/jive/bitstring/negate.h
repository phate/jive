#ifndef JIVE_BITSTRING_NEGATE_H
#define JIVE_BITSTRING_NEGATE_H

#include <jive/vsdg/node.h>
#include <jive/bitstring/type.h>

extern const jive_node_class JIVE_BITNEGATE_NODE;

typedef struct jive_node jive_bitnegate_node;
typedef struct jive_node_attrs jive_bitnegate_node_attrs;

struct jive_bitnegate_node {
	jive_node base;
	jive_bitnegate_node_attrs attrs;
};

/**
	\brief Create two's complement node
	\param region Region to put node into
	\param origin Input value
	\returns Bitstring value representing two's complement of input
	
	Create new two's complement node.
*/
jive_bitnegate_node *
jive_bitnegate_node_create(struct jive_region * region, jive_output * origin);

/**
	\brief Create bitnegate
	\param region Region to put node into
	\param origin Input value
	\returns Bitstring value representing two's complement of input
	
	Compute two's complement.
*/
jive_bitstring *
jive_bitnegate(jive_bitstring * operand);

static inline jive_bitnegate_node *
jive_bitnegate_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITNEGATE_NODE) return (jive_bitnegate_node *) node;
	else return 0;
}

#endif
