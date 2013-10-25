/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGAND_H
#define JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGAND_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgand_node jive_itgand_node;

extern const jive_itgbinary_operation_class JIVE_ITGAND_NODE_;
#define JIVE_ITGAND_NODE (JIVE_ITGAND_NODE_.base.base)

struct jive_itgand_node {
	jive_node base;
};

struct jive_output *
jive_itgand(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itgand_node *
jive_itgand_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGAND_NODE))
		return (struct jive_itgand_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgand_node *
jive_itgand_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGAND_NODE))
		return (const struct jive_itgand_node *) node;
	else
		return NULL;
}

#endif
