/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTQUOTIENT_HPP
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTQUOTIENT_HPP

#include <jive/types/float/fltoperation-classes.hpp>

namespace jive {
namespace flt {

extern const char fltquotient_name[];

typedef detail::make_binop<
	std::divides<value_repr>,
	fltquotient_name,
	jive::binary_op::flags::none> div_op;

}
}

jive::output *
jive_fltquotient(jive::output * operand1, jive::output * operand2);

#endif
