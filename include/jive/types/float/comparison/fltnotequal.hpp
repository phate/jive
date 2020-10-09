/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTNOTEQUAL_HPP
#define JIVE_TYPES_FLOAT_COMPARISON_FLTNOTEQUAL_HPP

#include <jive/types/float/fltoperation-classes.hpp>

namespace jive {
namespace flt {

extern const char fltnotequal_name[];

typedef detail::make_cmpop<
	std::not_equal_to<value_repr>,
	fltnotequal_name,
	jive::binary_op::flags::commutative> ne_op;

}
}

jive::output *
jive_fltnotequal(jive::output * operand1, jive::output * operand2);

#endif
