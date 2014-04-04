/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_ARITHMETIC_RLDIFFERENCE_H
#define JIVE_TYPES_REAL_ARITHMETIC_RLDIFFERENCE_H

#include <jive/types/real/rloperation-classes.h>

typedef struct jive_rldifference_node jive_rldifference_node;

extern const jive_rlbinary_operation_class JIVE_RLDIFFERENCE_NODE_;
#define JIVE_RLDIFFERENCE_NODE (JIVE_RLDIFFERENCE_NODE_.base.base)

struct jive_rldifference_node : public jive_node {
};

struct jive_output *
jive_rldifference(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_rldifference_node *
jive_rldifference_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLDIFFERENCE_NODE))
		return (struct jive_rldifference_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_rldifference_node *
jive_rldifference_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLDIFFERENCE_NODE))
		return (struct jive_rldifference_node *) node;
	else
		return NULL;
}

#endif
