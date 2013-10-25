/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGSUM_H
#define JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGSUM_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgsum_node jive_itgsum_node;

extern const jive_itgbinary_operation_class JIVE_ITGSUM_NODE_;
#define JIVE_ITGSUM_NODE (JIVE_ITGSUM_NODE_.base.base)

struct jive_itgsum_node {
	jive_node base;
};

struct jive_output *
jive_itgsum(struct jive_output * operand1, struct jive_output * summand2);

JIVE_EXPORTED_INLINE struct jive_itgsum_node *
jive_itgsum_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGSUM_NODE))
		return (struct jive_itgsum_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgsum_node *
jive_itgsum_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGSUM_NODE))
		return (const struct jive_itgsum_node *) node;
	else
		return NULL;
}

#endif
