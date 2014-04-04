/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_COMPARISON_RLNOTEQUAL_H
#define JIVE_TYPES_REAL_COMPARISON_RLNOTEQUAL_H

#include <jive/types/real/rloperation-classes.h>

typedef struct jive_rlnotequal_node jive_rlnotequal_node;

extern const jive_rlcomparison_operation_class JIVE_RLNOTEQUAL_NODE_;
#define JIVE_RLNOTEQUAL_NODE (JIVE_RLNOTEQUAL_NODE_.base.base)

struct jive_rlnotequal_node : public jive_node {
};

struct jive_output *
jive_rlnotequal(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_rlnotequal_node *
jive_rlnotequal_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLNOTEQUAL_NODE))
		return (struct jive_rlnotequal_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_rlnotequal_node *
jive_rlnotequal_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLNOTEQUAL_NODE))
		return (const struct jive_rlnotequal_node *) node;
	else
		return NULL;
}

#endif
