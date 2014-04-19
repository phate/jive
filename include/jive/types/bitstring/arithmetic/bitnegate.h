/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITNEGATE_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITNEGATE_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitunary_operation_class JIVE_BITNEGATE_NODE_;
#define JIVE_BITNEGATE_NODE (JIVE_BITNEGATE_NODE_.base.base)

namespace jive {
namespace bitstring {

class negate_operation final : public jive::bits_unary_operation {
};

}
}


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
