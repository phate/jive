/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_ARITHMETIC_DBLDIFFERENCE_H
#define JIVE_TYPES_DOUBLE_ARITHMETIC_DBLDIFFERENCE_H

#include <jive/types/double/dbloperation-classes.h>

typedef struct jive_dbldifference_node jive_dbldifference_node;

extern const jive_dblbinary_operation_class JIVE_DBLDIFFERENCE_NODE_;
#define JIVE_DBLDIFFERENCE_NODE (JIVE_DBLDIFFERENCE_NODE_.base.base)

struct jive_dbldifference_node : public jive_node {
};

struct jive_output *
jive_dbldifference(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_dbldifference_node *
jive_dbldifference_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLDIFFERENCE_NODE))
		return (struct jive_dbldifference_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_dbldifference_node *
jive_dbldifference_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLDIFFERENCE_NODE))
		return (const struct jive_dbldifference_node *) node;
	else
		return NULL;
}

#endif
