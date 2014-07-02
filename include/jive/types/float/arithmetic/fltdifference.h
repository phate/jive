/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTDIFFERENCE_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTDIFFERENCE_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_node_class JIVE_FLTDIFFERENCE_NODE;

namespace jive {
namespace flt {

extern const char fltdifference_name[];

typedef detail::make_binop<
	std::minus<value_repr>,
	&JIVE_FLTDIFFERENCE_NODE,
	fltdifference_name,
	jive_binary_operation_none> sub_op;

}
}

jive::output *
jive_fltdifference(jive::output * operand1, jive::output * operand2);

#endif
