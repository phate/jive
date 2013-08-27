/*
 * Copyright 2011 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITPRODUCT_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITPRODUCT_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITPRODUCT_NODE_;
#define JIVE_BITPRODUCT_NODE (JIVE_BITPRODUCT_NODE_.base.base)

jive_output *
jive_bitmultiply(size_t noperands, jive_output * const * operands);

JIVE_EXPORTED_INLINE jive_node *
jive_bitproduct_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITPRODUCT_NODE))
		return node;
	else
		return 0;
}

#endif
