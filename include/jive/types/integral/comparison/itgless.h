/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_COMPARISON_ITGLESS_H
#define JIVE_TYPES_INTEGRAL_COMPARISON_ITGLESS_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgless_node jive_itgless_node;

extern const jive_itgcomparison_operation_class JIVE_ITGLESS_NODE_;
#define JIVE_ITGLESS_NODE (JIVE_ITGLESS_NODE_.base.base)

struct jive_itgless_node : public jive_node {
};

struct jive_output *
jive_itgless(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itgless_node *
jive_itgless_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGLESS_NODE))
		return (jive_itgless_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgless_node *
jive_itgless_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGLESS_NODE))
		return (const jive_itgless_node *) node;
	else
		return NULL;
}

#endif
