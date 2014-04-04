/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_DOUBLE_COMPARISON_DBLLESSEQ_H
#define JIVE_TYPES_DOUBLE_COMPARISON_DBLLESSEQ_H

#include <jive/types/double/dbloperation-classes.h>

typedef struct jive_dbllesseq_node jive_dbllesseq_node;

extern const jive_dblcomparison_operation_class JIVE_DBLLESSEQ_NODE_;
#define JIVE_DBLLESSEQ_NODE (JIVE_DBLLESSEQ_NODE_.base.base)

struct jive_dbllesseq_node : public jive_node {
};

struct jive_output *
jive_dbllesseq(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_dbllesseq_node *
jive_dbllesseq_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLLESSEQ_NODE))
		return (jive_dbllesseq_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_dbllesseq_node *
jive_dbllesseq_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_DBLLESSEQ_NODE))
		return (const jive_dbllesseq_node *) node;
	else
		return NULL;
}

#endif
