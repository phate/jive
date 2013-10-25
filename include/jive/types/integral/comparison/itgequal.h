/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_COMPARISON_ITGEQUAL_H
#define JIVE_TYPES_INTEGRAL_COMPARISON_ITGEQUAL_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgequal_node jive_itgequal_node;

extern const jive_itgcomparison_operation_class JIVE_ITGEQUAL_NODE_;
#define JIVE_ITGEQUAL_NODE (JIVE_ITGEQUAL_NODE_.base.base)

struct jive_itgequal_node {
	jive_node base;
};

struct jive_output *
jive_itgequal(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itgequal_node *
jive_itgequal_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGEQUAL_NODE))
		return (jive_itgequal_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgequal_node *
jive_itgequal_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGEQUAL_NODE))
		return (const jive_itgequal_node *) node;
	else
		return NULL;
}

#endif
