/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGMODULO_H
#define JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGMODULO_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgmodulo_node jive_itgmodulo_node;

extern const jive_itgbinary_operation_class JIVE_ITGMODULO_NODE_;
#define JIVE_ITGMODULO_NODE (JIVE_ITGMODULO_NODE_.base.base)

struct jive_itgmodulo_node : public jive_node {
};

struct jive_output *
jive_itgmodulo(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itgmodulo_node *
jive_itgmodulo_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGMODULO_NODE))
		return (jive_itgmodulo_node *) node;
	else
		return 0;
}

JIVE_EXPORTED_INLINE const struct jive_itgmodulo_node *
jive_itgmodulo_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGMODULO_NODE))
		return (const jive_itgmodulo_node *) node;
	else
		return 0;
}

#endif
