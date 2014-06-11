/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTEQUAL_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTEQUAL_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltcomparison_operation_class JIVE_FLTEQUAL_NODE_;
#define JIVE_FLTEQUAL_NODE (JIVE_FLTEQUAL_NODE_.base.base)

namespace jive {
namespace flt {

class equal_operation final : public jive::flt_compare_operation {
};

}
}

jive::output *
jive_fltequal(jive::output * operand1, jive::output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_fltequal_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_FLTEQUAL_NODE))
		return node;
	else
		return NULL;
}

#endif
