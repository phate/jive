/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITNEGATE_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITNEGATE_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitunary_operation_class JIVE_BITNEGATE_NODE_;
#define JIVE_BITNEGATE_NODE (JIVE_BITNEGATE_NODE_.base.base)

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


JIVE_EXPORTED_INLINE jive_node *
jive_bitnegate_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITNEGATE_NODE))
		return node;
	else
		return 0;
}

#endif
