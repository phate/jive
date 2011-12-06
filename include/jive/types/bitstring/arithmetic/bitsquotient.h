#ifndef JIVE_TYPES_BITSTRING_ARITHMETIC_BITSQUOTIENT_H
#define JIVE_TYPES_BITSTRING_ARITHMETIC_BITSQUOTIENT_H

#include <jive/types/bitstring/bitoperation-classes.h>

extern const jive_bitbinary_operation_class JIVE_BITSQUOTIENT_NODE_;
#define JIVE_BITSQUOTIENT_NODE (JIVE_BITSQUOTIENT_NODE_.base.base)

jive_node *
jive_bitsquotient_create(struct jive_region * region,
	jive_output * dividend, jive_output * divisor);

jive_output *
jive_bitsquotient(jive_output * dividend, jive_output * divisor);

JIVE_EXPORTED_INLINE jive_node *
jive_bitsquotient_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_BITSQUOTIENT_NODE))
		return node;
	else
		return 0;
}

#endif
