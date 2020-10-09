/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTGREATER_HPP
#define JIVE_TYPES_FLOAT_COMPARISON_FLTGREATER_HPP

#include <jive/types/float/fltoperation-classes.hpp>

namespace jive {
namespace flt {

extern const char fltgreater_name[];

typedef detail::make_cmpop<
	std::greater<value_repr>,
	fltgreater_name,
	jive::binary_op::flags::none> gt_op;

}
}

jive::output *
jive_fltgreater(jive::output * operand1, jive::output * operand2);

#endif

