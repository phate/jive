/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTPRODUCT_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTPRODUCT_H

#include <jive/types/float/fltoperation-classes.h>

namespace jive {
namespace flt {

extern const char fltproduct_name[];

typedef detail::make_binop<
	std::multiplies<value_repr>,
	fltproduct_name,
	jive_binary_operation_commutative> mul_op;

}
}

jive::oport *
jive_fltproduct(jive::oport * operand1, jive::oport * operand2);

#endif
