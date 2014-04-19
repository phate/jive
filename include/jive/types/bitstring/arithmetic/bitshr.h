/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITSHR_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITSHR_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITSHR_NODE_;
#define JIVE_BITSHR_NODE (JIVE_BITSHR_NODE_.base.base)

namespace jive {
namespace bitstring {

class shr_operation final : public jive::bits_binary_operation {
};

}
}

jive_output *
jive_bitshr(jive_output * operand, jive_output * shift);

JIVE_EXPORTED_INLINE jive_node *
jive_bitshr_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITSHR_NODE))
		return node;
	else
		return 0;
}

#endif
