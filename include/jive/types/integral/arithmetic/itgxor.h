/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGXOR_H
#define JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGXOR_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgxor_node jive_itgxor_node;

extern const jive_itgbinary_operation_class JIVE_ITGXOR_NODE_;
#define JIVE_ITGXOR_NODE (JIVE_ITGXOR_NODE_.base.base)

struct jive_itgxor_node {
	jive_node base;
};

struct jive_output *
jive_itgxor(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itgxor_node *
jive_itgxor_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGXOR_NODE))
		return (jive_itgxor_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgxor_node *
jive_itgxor_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGXOR_NODE))
		return (const jive_itgxor_node *) node;
	else
		return NULL;
}

#endif
