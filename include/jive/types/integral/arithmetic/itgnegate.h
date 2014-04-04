/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGNEGATE_H
#define JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGNEGATE_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgnegate_node jive_itgnegate_node;

extern const jive_itgunary_operation_class JIVE_ITGNEGATE_NODE_;
#define JIVE_ITGNEGATE_NODE (JIVE_ITGNEGATE_NODE_.base.base)

struct jive_itgnegate_node : public jive_node {
};

struct jive_output *
jive_itgnegate(struct jive_output * operand);

JIVE_EXPORTED_INLINE struct jive_itgnegate_node *
jive_itgnegate_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGNEGATE_NODE))
		return (jive_itgnegate_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgnegate_node *
jive_itgnegate_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGNEGATE_NODE))
		return (const jive_itgnegate_node *) node;
	else
		return NULL;
}

#endif
