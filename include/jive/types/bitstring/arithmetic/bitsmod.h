#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITSMOD_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITSMOD_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITSMOD_NODE_;
#define JIVE_BITSMOD_NODE (JIVE_BITSMOD_NODE_.base.base)

jive_node *
jive_bitsmod_create(struct jive_region * region,
	jive_output * operand1, jive_output * operand2);

jive_output *
jive_bitsmod(jive_output * operand1, jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_bitsmod_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITSMOD_NODE))
		return node;
	else
		return 0;
}

#endif

