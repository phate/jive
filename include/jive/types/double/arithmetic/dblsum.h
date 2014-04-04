/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_ARITHMETIC_DBLSUM_H
#define JIVE_TYPES_DOUBLE_ARITHMETIC_DBLSUM_H

#include <jive/types/double/dbloperation-classes.h>

typedef struct jive_dblsum_node jive_dblsum_node;

extern const jive_dblbinary_operation_class JIVE_DBLSUM_NODE_;
#define JIVE_DBLSUM_NODE (JIVE_DBLSUM_NODE_.base.base)

struct jive_dblsum_node : public jive_node {
};

struct jive_output *
jive_dblsum(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_dblsum_node *
jive_dblsum_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLSUM_NODE))
		return (struct jive_dblsum_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_dblsum_node *
jive_dblsum_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLSUM_NODE))
		return (struct jive_dblsum_node *) node;
	else
		return NULL;
}

#endif
