#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITDIFFERENCE_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITDIFFERENCE_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITDIFFERENCE_NODE_;
#define JIVE_BITDIFFERENCE_NODE (JIVE_BITDIFFERENCE_NODE_.base.base)

jive_node *
jive_bitdifference_create(struct jive_region * region,
	jive_output * op1, jive_output * op2);

jive_output *
jive_bitdifference(jive_output * op1, jive_output * op2);

JIVE_EXPORTED_INLINE jive_node *
jive_bitdifference_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITDIFFERENCE_NODE))
		return node;
	else
		return 0;
}

#endif
