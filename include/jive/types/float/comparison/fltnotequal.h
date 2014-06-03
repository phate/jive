/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTNOTEQUAL_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTNOTEQUAL_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltcomparison_operation_class JIVE_FLTNOTEQUAL_NODE_;
#define JIVE_FLTNOTEQUAL_NODE (JIVE_FLTNOTEQUAL_NODE_.base.base)

namespace jive {
namespace flt {

bool compute_notequal(value_repr arg1, value_repr arg2);
extern const char fltnotequal_name[];

typedef detail::make_cmpop<
	compute_notequal,
	&JIVE_FLTNOTEQUAL_NODE_,
	fltnotequal_name,
	jive_binary_operation_commutative> notequal_operation;

}
}

jive::output *
jive_fltnotequal(jive::output * operand1, jive::output * operand2);

#endif
