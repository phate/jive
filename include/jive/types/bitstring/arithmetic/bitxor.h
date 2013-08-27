/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITXOR_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITXOR_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITXOR_NODE_;
#define JIVE_BITXOR_NODE (JIVE_BITXOR_NODE_.base.base)

jive_output *
jive_bitxor(size_t noperands, jive_output * const * operands);

JIVE_EXPORTED_INLINE jive_node *
jive_bitxor_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITXOR_NODE))
		return node;
	else
		return 0;
}

#endif
