/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTGREATEREQ_HPP
#define JIVE_TYPES_FLOAT_COMPARISON_FLTGREATEREQ_HPP

#include <jive/types/float/fltoperation-classes.hpp>

namespace jive {
namespace flt {

extern const char fltgreatereq_name[];

typedef detail::make_cmpop<
	std::greater_equal<value_repr>,
	fltgreatereq_name,
	jive::binary_op::flags::none> ge_op;

}
}

jive::output *
jive_fltgreatereq(jive::output * operand1, jive::output * operand2);

#endif

