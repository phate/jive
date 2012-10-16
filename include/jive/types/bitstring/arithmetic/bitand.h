/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITAND_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITAND_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITAND_NODE_;
#define JIVE_BITAND_NODE (JIVE_BITAND_NODE_.base.base)

jive_node *
jive_bitand_create(struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

jive_output *
jive_bitand(size_t noperands, jive_output * operands[const]);

JIVE_EXPORTED_INLINE jive_node *
jive_bitand_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITAND_NODE))
		return node;
	else
		return 0;
}

#endif
