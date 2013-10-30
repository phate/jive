/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_COMPARISON_RLLESS_H
#define JIVE_TYPES_REAL_COMPARISON_RLLESS_H

#include <jive/types/real/rloperation-classes.h>

typedef struct jive_rlless_node jive_rlless_node;

extern const jive_rlcomparison_operation_class JIVE_RLLESS_NODE_;
#define JIVE_RLLESS_NODE (JIVE_RLLESS_NODE_.base.base)

struct jive_rlless_node {
	jive_node base;
};

struct jive_output *
jive_rlless(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_rlless_node *
jive_rlless_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLLESS_NODE))
		return (struct jive_rlless_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_rlless_node *
jive_rlless_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLLESS_NODE))
		return (const struct jive_rlless_node *) node;
	else
		return NULL;
}

#endif
