/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTDIFFERENCE_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTDIFFERENCE_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltbinary_operation_class JIVE_FLTDIFFERENCE_NODE_;
#define JIVE_FLTDIFFERENCE_NODE (JIVE_FLTDIFFERENCE_NODE_.base.base)

namespace jive {
namespace flt {

value_repr compute_difference(value_repr arg1, value_repr arg2);
extern const char fltdifference_name[];

typedef detail::make_binop<
	compute_difference,
	&JIVE_FLTDIFFERENCE_NODE_,
	fltdifference_name,
	jive_binary_operation_none> difference_operation;

}
}

jive::output *
jive_fltdifference(jive::output * operand1, jive::output * operand2);

#endif
