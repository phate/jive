/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGOR_H
#define JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGOR_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgor_node jive_itgor_node;

extern const jive_itgbinary_operation_class JIVE_ITGOR_NODE_;
#define JIVE_ITGOR_NODE (JIVE_ITGOR_NODE_.base.base)

struct jive_itgor_node : public jive_node {
};

struct jive_output *
jive_itgor(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itgor_node *
jive_itgor_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGOR_NODE))
		return (jive_itgor_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgor_node *
jive_itgor_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGOR_NODE))
		return (const jive_itgor_node *) node;
	else
		return NULL;
}

#endif
