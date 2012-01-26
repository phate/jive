#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITSUM_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITSUM_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITSUM_NODE_;
#define JIVE_BITSUM_NODE (JIVE_BITSUM_NODE_.base.base)

jive_node *
jive_bitsum_create(struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

jive_output *
jive_bitsum(size_t noperands, jive_output * operands[const]);

JIVE_EXPORTED_INLINE jive_node *
jive_bitsum_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITSUM_NODE))
		return node;
	else
		return 0;
}

#endif