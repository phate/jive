/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTEQUAL_HPP
#define JIVE_TYPES_FLOAT_COMPARISON_FLTEQUAL_HPP

#include <jive/types/float/fltoperation-classes.hpp>

namespace jive {
namespace flt {

extern const char fltequal_name[];

typedef detail::make_cmpop<
	std::equal_to<value_repr>,
	fltequal_name,
	jive::binary_op::flags::commutative> eq_op;

}
}

jive::output *
jive_fltequal(jive::output * operand1, jive::output * operand2);

#endif
