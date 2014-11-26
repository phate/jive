/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTNOTEQUAL_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTNOTEQUAL_H

#include <jive/types/float/fltoperation-classes.h>

namespace jive {
namespace flt {

extern const char fltnotequal_name[];

typedef detail::make_cmpop<
	std::not_equal_to<value_repr>,
	fltnotequal_name,
	jive_binary_operation_commutative> ne_op;

}
}

jive::output *
jive_fltnotequal(jive::output * operand1, jive::output * operand2);

#endif
