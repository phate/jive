/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGQUOTIENT_H
#define JIVE_TYPES_INTEGRAL_ARITHMETIC_ITGQUOTIENT_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itgquotient_node jive_itgquotient_node;

extern const jive_itgbinary_operation_class JIVE_ITGQUOTIENT_NODE_;
#define JIVE_ITGQUOTIENT_NODE (JIVE_ITGQUOTIENT_NODE_.base.base)

struct jive_itgquotient_node {
	jive_node base;
};

struct jive_output *
jive_itgquotient(struct jive_output * dividend, struct jive_output * divisor);

JIVE_EXPORTED_INLINE struct jive_itgquotient_node *
jive_itgquotient_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGQUOTIENT_NODE))
		return (jive_itgquotient_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itgquotient_node *
jive_itgquotient_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGQUOTIENT_NODE))
		return (const jive_itgquotient_node *) node;
	else
		return NULL;
}

#endif
