/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_ARITHMETIC_DBLPRODUCT_H
#define JIVE_TYPES_DOUBLE_ARITHMETIC_DBLPRODUCT_H

#include <jive/types/double/dbloperation-classes.h>

typedef struct jive_dblproduct_node jive_dblproduct_node;

extern const jive_dblbinary_operation_class JIVE_DBLPRODUCT_NODE_;
#define JIVE_DBLPRODUCT_NODE (JIVE_DBLPRODUCT_NODE_.base.base)

struct jive_dblproduct_node {
	jive_node base;
};

struct jive_output *
jive_dblproduct(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_dblproduct_node *
jive_dblproduct_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLPRODUCT_NODE))
		return (jive_dblproduct_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_dblproduct_node *
jive_dblproduct_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLPRODUCT_NODE))
		return (const jive_dblproduct_node *) node;
	else
		return NULL;
}

#endif
