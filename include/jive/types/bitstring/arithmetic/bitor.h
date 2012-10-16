/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITOR_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITOR_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITOR_NODE_;
#define JIVE_BITOR_NODE (JIVE_BITOR_NODE_.base.base)

jive_node *
jive_bitor_create(struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

jive_output *
jive_bitor(size_t noperands, jive_output * operands[const]);

JIVE_EXPORTED_INLINE jive_node *
jive_bitor_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITOR_NODE))
		return node;
	else
		return 0;
}

#endif
