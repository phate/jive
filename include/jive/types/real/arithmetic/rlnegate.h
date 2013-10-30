/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_REAL_ARITHMETIC_RLNEGATE_H
#define JIVE_TYPES_REAL_ARITHMETIC_RLNEGATE_H

#include <jive/types/real/rloperation-classes.h>

typedef struct jive_rlnegate_node jive_rlnegate_node;

extern const jive_rlunary_operation_class JIVE_RLNEGATE_NODE_;
#define JIVE_RLNEGATE_NODE (JIVE_RLNEGATE_NODE_.base.base)

struct jive_rlnegate_node {
	jive_node base;
};

struct jive_output *
jive_rlnegate(struct jive_output * operand);

JIVE_EXPORTED_INLINE struct jive_rlnegate_node *
jive_rlnegate_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLNEGATE_NODE))
		return (struct jive_rlnegate_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_rlnegate_node *
jive_rlnegate_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_RLNEGATE_NODE))
		return (const struct jive_rlnegate_node *) node;
	else
		return NULL;
}

#endif
