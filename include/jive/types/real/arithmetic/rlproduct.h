/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_ARITHMETIC_RLPRODUCT_H
#define JIVE_TYPES_REAL_ARITHMETIC_RLPRODUCT_H

#include <jive/types/real/rloperation-classes.h>

typedef struct jive_rlproduct_node jive_rlproduct_node;

extern const jive_rlbinary_operation_class JIVE_RLPRODUCT_NODE_;
#define JIVE_RLPRODUCT_NODE (JIVE_RLPRODUCT_NODE_.base.base)

struct jive_rlproduct_node {
	jive_node base;
};

struct jive_output *
jive_rlproduct(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_rlproduct_node *
jive_rlproduct_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLPRODUCT_NODE))
		return (struct jive_rlproduct_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_rlproduct_node *
jive_rlproduct_node_const_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLPRODUCT_NODE))
		return (const struct jive_rlproduct_node *) node;
	else
		return NULL;
}

#endif
