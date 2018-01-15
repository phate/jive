/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTGREATEREQ_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTGREATEREQ_H

#include <jive/types/float/fltoperation-classes.h>

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

