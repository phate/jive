/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_ARITHMETIC_DBLQUOTIENT_H
#define JIVE_TYPES_DOUBLE_ARITHMETIC_DBLQUOTIENT_H

#include <jive/types/double/dbloperation-classes.h>

typedef struct jive_dblquotient_node jive_dblquotient_node;

extern const jive_dblbinary_operation_class JIVE_DBLQUOTIENT_NODE_;
#define JIVE_DBLQUOTIENT_NODE (JIVE_DBLQUOTIENT_NODE_.base.base)

struct jive_dblquotient_node {
	jive_node base;
};

struct jive_output *
jive_dblquotient(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_dblquotient_node *
jive_dblquotient_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLQUOTIENT_NODE))
		return (jive_dblquotient_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_dblquotient_node *
jive_dblquotient_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLQUOTIENT_NODE))
		return (const jive_dblquotient_node *) node;
	else
		return NULL;
}

#endif
