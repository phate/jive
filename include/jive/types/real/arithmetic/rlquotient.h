/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_ARITHMETIC_RLQUOTIENT_H
#define JIVE_TYPES_REAL_ARITHMETIC_RLQUOTIENT_H

#include <jive/types/real/rloperation-classes.h>

typedef struct jive_rlquotient_node jive_rlquotient_node;

extern const jive_rlbinary_operation_class JIVE_RLQUOTIENT_NODE_;
#define JIVE_RLQUOTIENT_NODE (JIVE_RLQUOTIENT_NODE_.base.base)

struct jive_rlquotient_node {
	jive_node base;
};

struct jive_output *
jive_rlquotient(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_rlquotient_node *
jive_rlquotient_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLQUOTIENT_NODE))
		return (struct jive_rlquotient_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_rlquotient_node *
jive_rlquotient_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLQUOTIENT_NODE))
		return (const struct jive_rlquotient_node *) node;
	else
		return NULL;
}

#endif
