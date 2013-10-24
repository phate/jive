/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_ARITHMETIC_DBLNEGATE_H
#define JIVE_TYPES_DOUBLE_ARITHMETIC_DBLNEGATE_H

#include <jive/types/double/dbloperation-classes.h>

typedef struct jive_dblnegate_node jive_dblnegate_node;

extern const jive_dblunary_operation_class JIVE_DBLNEGATE_NODE_;
#define JIVE_DBLNEGATE_NODE (JIVE_DBLNEGATE_NODE_.base.base)

struct jive_dblnegate_node {
	jive_node base;
};

struct jive_output *
jive_dblnegate(struct jive_output * operand);

JIVE_EXPORTED_INLINE struct jive_dblnegate_node *
jive_dblnegate_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLNEGATE_NODE))
		return (struct jive_dblnegate_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_dblnegate_node *
jive_dblnegate_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLNEGATE_NODE))
		return (const struct jive_dblnegate_node *) node;
	else
		return NULL;
}

#endif
