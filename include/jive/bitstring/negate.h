#ifndef JIVE_BITSTRING_NEGATE_H
#define JIVE_BITSTRING_NEGATE_H

#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>
#include <jive/bitstring/type.h>

extern const jive_unary_operation_class JIVE_BITNEGATE_NODE_;
#define JIVE_BITNEGATE_NODE (JIVE_BITNEGATE_NODE_.base)

/**
	\brief Create bitnegate
	\param region Region to put node into
	\param origin Input value
	\returns Bitstring value representing negate
	
	Create new bitnegate node. Computes the two's complement
	of the input bitstring.
*/
jive_node *
jive_bitnegate_create(struct jive_region * region, jive_output * origin);

/**
	\brief Create bitnegate
	\param operand Input value
	\returns Bitstring value representing negate
	
	Convenience function to create negation of value.
*/
jive_output *
jive_bitnegate(jive_output * operand);

static inline jive_node *
jive_bitnegate_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITNEGATE_NODE) return node;
	else return 0;
}

#endif
