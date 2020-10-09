/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTPRODUCT_HPP
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTPRODUCT_HPP

#include <jive/types/float/fltoperation-classes.hpp>

namespace jive {
namespace flt {

extern const char fltproduct_name[];

typedef detail::make_binop<
	std::multiplies<value_repr>,
	fltproduct_name,
	jive::binary_op::flags::commutative> mul_op;

}
}

jive::output *
jive_fltproduct(jive::output * operand1, jive::output * operand2);

#endif
