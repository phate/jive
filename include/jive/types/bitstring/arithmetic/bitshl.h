#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITSHL_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITSHL_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITSHL_NODE_;
#define JIVE_BITSHL_NODE (JIVE_BITSHL_NODE_.base.base)

jive_node *
jive_bitshl_create(struct jive_region * region,
	jive_output * operand, jive_output * shift);

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
