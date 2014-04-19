/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITUHIPRODUCT_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITUHIPRODUCT_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITUHIPRODUCT_NODE_;
#define JIVE_BITUHIPRODUCT_NODE (JIVE_BITUHIPRODUCT_NODE_.base.base)

namespace jive {
namespace bitstring {

class uhiproduct_operation final : public jive::bits_binary_operation {
};

}
}

jive_output *
jive_bituhiproduct(jive_output * factor1, jive_output * factor2);

JIVE_EXPORTED_INLINE jive_node *
jive_bituhiproduct_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITUHIPRODUCT_NODE))
		return node;
	else
		return 0;
}

#endif
