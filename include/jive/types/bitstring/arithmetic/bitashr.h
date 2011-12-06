#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITASHR_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITASHR_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITASHR_NODE_;
#define JIVE_BITASHR_NODE (JIVE_BITASHR_NODE_.base.base)

jive_node *
jive_bitashr_create(struct jive_region * region,
	jive_output * operand, jive_output * shift);

jive_output *
jive_bitashr(jive_output * operand, jive_output * shift);

JIVE_EXPORTED_INLINE jive_node *
jive_bitashr_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITASHR_NODE))
		return node;
	else
		return 0;
}

#endif
