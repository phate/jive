/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_COMPARISON_DBLNOTEQUAL_H
#define JIVE_TYPES_DOUBLE_COMPARISON_DBLNOTEQUAL_H

#include <jive/types/double/dbloperation-classes.h>

typedef struct jive_dblnotequal_node jive_dblnotequal_node;

extern const jive_dblcomparison_operation_class JIVE_DBLNOTEQUAL_NODE_;
#define JIVE_DBLNOTEQUAL_NODE (JIVE_DBLNOTEQUAL_NODE_.base.base)

struct jive_dblnotequal_node {
	jive_node base;
};

struct jive_output *
jive_dblnotequal(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_dblnotequal_node *
jive_dblnotequal_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLNOTEQUAL_NODE))
		return (jive_dblnotequal_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_dblnotequal_node *
jive_dblnotequal_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLNOTEQUAL_NODE))
		return (const jive_dblnotequal_node *) node;
	else
		return NULL;
}

#endif
