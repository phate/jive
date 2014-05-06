/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTPRODUCT_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTPRODUCT_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltbinary_operation_class JIVE_FLTPRODUCT_NODE_;
#define JIVE_FLTPRODUCT_NODE (JIVE_FLTPRODUCT_NODE_.base.base)

namespace jive {
namespace flt {

class product_operation final : public jive::flt_binary_operation {
};

}
}

jive_output *
jive_fltproduct(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_fltproduct_node_cast(jive_node * node)
{
	if(jive_node_isinstance(node, &JIVE_FLTPRODUCT_NODE))
		return node;
	else
		return NULL;
}

#endif
