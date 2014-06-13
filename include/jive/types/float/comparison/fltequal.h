/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTEQUAL_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTEQUAL_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_node_class JIVE_FLTEQUAL_NODE;

namespace jive {
namespace flt {

bool compute_equal(value_repr arg1, value_repr arg2);
extern const char fltequal_name[];

typedef detail::make_cmpop<
	compute_equal,
	&JIVE_FLTEQUAL_NODE,
	fltequal_name,
	jive_binary_operation_commutative> equal_operation;

}
}

jive::output *
jive_fltequal(jive::output * operand1, jive::output * operand2);

#endif
