/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTSUM_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTSUM_H

#include <jive/types/float/fltoperation-classes.h>

namespace jive {
namespace flt {

extern const char fltsum_name[];

typedef detail::make_binop<
	std::plus<value_repr>,
	fltsum_name,
	jive_binary_operation_commutative> add_op;

}
}

jive::output *
jive_fltsum(jive::output * operand1, jive::output * operand2);

#endif
