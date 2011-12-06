#ifndef JIVE_TYPES_BITSTRING_CONCAT_H
#define JIVE_TYPES_BITSTRING_CONCAT_H

#include <jive/types/bitstring/type.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

extern const jive_binary_operation_class JIVE_BITCONCAT_NODE_;
#define JIVE_BITCONCAT_NODE (JIVE_BITCONCAT_NODE_.base)

jive_node *
jive_bitconcat_create(
	struct jive_region * region,
	size_t noperands, struct jive_output * operands[const]);

jive_output *
jive_bitconcat(size_t noperands, jive_output * operands[const]);

JIVE_EXPORTED_INLINE jive_node *
jive_bitconcat_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_BITCONCAT_NODE) return node;
	else return 0;
}

#endif
