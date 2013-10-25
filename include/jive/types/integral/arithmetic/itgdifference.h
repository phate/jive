/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGDIFFERENCE_H
#define JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGDIFFERENCE_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgdifference_node jive_itgdifference_node;

extern const jive_itgbinary_operation_class JIVE_ITGDIFFERENCE_NODE_;
#define JIVE_ITGDIFFERENCE_NODE (JIVE_ITGDIFFERENCE_NODE_.base.base)

struct jive_itgdifference_node {
	jive_node base;
};

struct jive_output *
jive_itgdifference(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itgdifference_node *
jive_itgdifference_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGDIFFERENCE_NODE))
		return (struct jive_itgdifference_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgdifference_node *
jive_itgdifference_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGDIFFERENCE_NODE))
		return (const struct jive_itgdifference_node *) node;
	else
		return NULL;
}

#endif
