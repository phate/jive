/*
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTLESSEQ_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTLESSEQ_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltcomparison_operation_class JIVE_FLTLESSEQ_NODE_;
#define JIVE_FLTLESSEQ_NODE (JIVE_FLTLESSEQ_NODE_.base.base)

namespace jive {
namespace flt {

class lesseq_operation final : public jive::flt_compare_operation {
};

}
}

struct jive_output *
jive_fltlesseq(struct jive_output * operand1, struct jive_output * operand2);

JIVE_EXPORTED_INLINE jive_node *
jive_fltlesseq_node_cast(jive_node * node)
{
	if (jive_node_isinstance(node, &JIVE_FLTLESSEQ_NODE))
		return node;
	else
		return NULL;
}

#endif
