/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTSUM_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTSUM_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltbinary_operation_class JIVE_FLTSUM_NODE_;
#define JIVE_FLTSUM_NODE (JIVE_FLTSUM_NODE_.base.base)

namespace jive {
namespace flt {

value_repr compute_sum(value_repr arg1, value_repr arg2);
extern const char fltsum_name[];

typedef detail::make_binop<
	compute_sum,
	&JIVE_FLTSUM_NODE_,
	fltsum_name,
	jive_binary_operation_commutative> sum_operation;

}
}

jive::output *
jive_fltsum(jive::output * operand1, jive::output * operand2);

#endif
