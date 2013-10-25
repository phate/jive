/*
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_INTEGRAL_COMPARISON_ITGLESSEQ_H
#define JIVE_TYPES_INTEGRAL_COMPARISON_ITGLESSEQ_H

#include <jive/types/integral/itgoperation-classes.h>

typedef struct jive_itglesseq_node jive_itglesseq_node;

extern const jive_itgcomparison_operation_class JIVE_ITGLESSEQ_NODE_;
#define JIVE_ITGLESSEQ_NODE (JIVE_ITGLESSEQ_NODE_.base.base)

struct jive_itglesseq_node {
	jive_node base;
};

struct jive_output *
jive_itglesseq(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE struct jive_itglesseq_node *
jive_itglesseq_node_cast(struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGLESSEQ_NODE))
		return (jive_itglesseq_node *) node;
	else
		return NULL;
}

JIVE_EXPORTED_INLINE const struct jive_itglesseq_node *
jive_itglesseq_node_const_cast(const struct jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_ITGLESSEQ_NODE))
		return (const jive_itglesseq_node *) node;
	else
		return NULL;
}

#endif
