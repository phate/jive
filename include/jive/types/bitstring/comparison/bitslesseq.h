#ifndef JIVE_TYPES_BITSTRING_COMPARISON_BITSLESSEQ_H
#define JIVE_TYPES_BITSTRING_COMPARISON_BITSLESSEQ_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitcomparison_operation_class JIVE_BITSLESSEQ_NODE_;
#define JIVE_BITSLESSEQ_NODE (JIVE_BITSLESSEQ_NODE_.base.base)

jive_node *
jive_bitslesseq_create(struct jive_region * region,
	struct jive_output * operand1, struct jive_output * operand2);

jive_output *
jive_bitslesseq(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_bitslesseq_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITSLESSEQ_NODE))
		return node;
	else
		return NULL;
}

#endif


