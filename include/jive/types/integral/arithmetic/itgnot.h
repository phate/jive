/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGNOT_H
#define JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGNOT_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgnot_node jive_itgnot_node;

extern const jive_itgunary_operation_class JIVE_ITGNOT_NODE_;
#define JIVE_ITGNOT_NODE (JIVE_ITGNOT_NODE_.base.base)

struct jive_itgnot_node : public jive_node {
};

struct jive_output *
jive_itgnot(struct jive_output * operand);

JIVE_EXPORTED_INLINE struct jive_itgnot_node *
jive_itgnot_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGNOT_NODE))
		return (jive_itgnot_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgnot_node *
jive_itgnot_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGNOT_NODE))
		return (const jive_itgnot_node *) node;
	else
		return NULL;
}

#endif
