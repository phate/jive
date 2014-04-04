/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_COMPARISON_ITGGREATER_H
#define JIVE_TYPES_INTEGRAL_COMPARISON_ITGGREATER_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itggreater_node jive_itggreater_node;

extern const jive_itgcomparison_operation_class JIVE_ITGGREATER_NODE_;
#define JIVE_ITGGREATER_NODE (JIVE_ITGGREATER_NODE_.base.base)

struct jive_itggreater_node : public jive_node {
};

struct jive_output *
jive_itggreater(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itggreater_node *
jive_itggreater_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGGREATER_NODE))
		return (jive_itggreater_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itggreater_node *
jive_itggreater_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGGREATER_NODE))
		return (jive_itggreater_node *) node;
	else
		return NULL;
}

#endif
