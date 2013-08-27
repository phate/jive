/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITNOT_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITNOT_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitunary_operation_class JIVE_BITNOT_NODE_;
#define JIVE_BITNOT_NODE (JIVE_BITNOT_NODE_.base.base)

/**
	\brief Create bitnot
	\param operand Input value
	\returns Bitstring value representing not
	
	Convenience function to create negation of value.
*/
jive_output *
jive_bitnot(jive_output * operand);

JIVE_EXPORTED_INLINE jive_node *
jive_bitnot_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITNOT_NODE))
		return node;
	else
		return 0;
}

#endif
