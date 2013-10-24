/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_COMPARISON_DBLGREATEREQ_H
#define JIVE_TYPES_DOUBLE_COMPARISON_DBLGREATEREQ_H

#include <jive/types/double/dbloperation-classes.h>

typedef struct jive_dblgreatereq_node jive_dblgreatereq_node;

extern const jive_dblcomparison_operation_class JIVE_DBLGREATEREQ_NODE_;
#define JIVE_DBLGREATEREQ_NODE (JIVE_DBLGREATEREQ_NODE_.base.base)

struct jive_dblgreatereq_node {
	jive_node base;
};

struct jive_output *
jive_dblgreatereq(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_dblgreatereq_node *
jive_dblgreatereq_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLGREATEREQ_NODE))
		return (jive_dblgreatereq_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_dblgreatereq_node *
jive_dblgreatereq_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLGREATEREQ_NODE))
		return (const jive_dblgreatereq_node *) node;
	else
		return NULL;
}

#endif
