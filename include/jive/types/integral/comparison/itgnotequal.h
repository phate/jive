/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_COMPARISON_ITGNOTEQUAL_H
#define JIVE_TYPES_INTEGRAL_COMPARISON_ITGNOTEQUAL_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgnotequal_node jive_itgnotequal_node;

extern const jive_itgcomparison_operation_class JIVE_ITGNOTEQUAL_NODE_;
#define JIVE_ITGNOTEQUAL_NODE (JIVE_ITGNOTEQUAL_NODE_.base.base)

struct jive_itgnotequal_node : public jive_node {
};

struct jive_output *
jive_itgnotequal(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itgnotequal_node *
jive_itgnotequal_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGNOTEQUAL_NODE))
		return (jive_itgnotequal_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgnotequal_node *
jive_itgnotequal_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGNOTEQUAL_NODE))
		return (const jive_itgnotequal_node *) node;
	else
		return NULL;
}

#endif
