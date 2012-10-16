/*
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_COMPARISON_BITULESS_H
#define JIVE_TYPES_BITSTRING_COMPARISON_BITULESS_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitcomparison_operation_class JIVE_BITULESS_NODE_;
#define JIVE_BITULESS_NODE (JIVE_BITULESS_NODE_.base.base)

jive_node *
jive_bituless_create(struct jive_region * region,
	struct jive_output * operand1, struct jive_output * operand2);

jive_output *
jive_bituless(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_bituless_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITULESS_NODE))
		return node;
	else
		return NULL;
}

#endif


