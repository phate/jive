/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITSHL_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITSHL_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITSHL_NODE_;
#define JIVE_BITSHL_NODE (JIVE_BITSHL_NODE_.base.base)

namespace jive {
namespace bitstring {

class shl_operation final : public jive::bits_binary_operation {
};

}
}

jive_output *
jive_bitshl(jive_output * operand, jive_output * shift);

JIVE_EXPORTED_INLINE jive_node *
jive_bitshl_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITSHL_NODE))
		return node;
	else
		return 0;
}

#endif
