/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTNEGATE_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTNEGATE_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltunary_operation_class JIVE_FLTNEGATE_NODE_;
#define JIVE_FLTNEGATE_NODE (JIVE_FLTNEGATE_NODE_.base.base)

namespace jive {
namespace flt {

class negate_operation final : public jive::flt_unary_operation {
};

}
}

jive_output *
jive_fltnegate(struct jive_output * operand);

JIVE_EXPORTED_INLINE jive_node *
jive_fltnegate_node_cast(jive_node * node)
{
	if(jive_node_isinstance(node, &JIVE_FLTNEGATE_NODE))
		return node;
	else
		return NULL;
}

#endif
