/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTDIFFERENCE_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTDIFFERENCE_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltbinary_operation_class JIVE_FLTDIFFERENCE_NODE_;
#define JIVE_FLTDIFFERENCE_NODE (JIVE_FLTDIFFERENCE_NODE_.base.base)

jive_output *
jive_fltdifference(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_fltdifference_node_cast(jive_node * node)
{
	if(jive_node_isinstance(node, &JIVE_FLTDIFFERENCE_NODE))
		return node;
	else
		return NULL;
}

#endif
