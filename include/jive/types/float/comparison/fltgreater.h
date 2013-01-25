/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTGREATER_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTGREATER_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltcomparison_operation_class JIVE_FLTGREATER_NODE_;
#define JIVE_FLTGREATER_NODE (JIVE_FLTGREATER_NODE_.base.base)

jive_output *
jive_fltgreater(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_fltgreater_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_FLTGREATER_NODE))
		return node;
	else
		return NULL;
}

#endif

