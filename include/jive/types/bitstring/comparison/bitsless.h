#ifndef JIVE_TYPES_BITSTRING_COMPARISON_BITSLESS_H
#define JIVE_TYPES_BITSTRING_COMPARISON_BITSLESS_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitcomparison_operation_class JIVE_BITSLESS_NODE_;
#define JIVE_BITSLESS_NODE (JIVE_BITSLESS_NODE_.base.base)

jive_node *
jive_bitsless_create(struct jive_region * region,
	struct jive_output * operand1, struct jive_output * operand2);

jive_output *
jive_bitsless(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_bitsless_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITSLESS_NODE))
		return node;
	else
		return NULL;
}

#endif

