/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTGREATEREQ_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTGREATEREQ_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_node_class JIVE_FLTGREATEREQ_NODE;

namespace jive {
namespace flt {

bool compute_greatereq(value_repr arg1, value_repr arg2);
extern const char fltgreatereq_name[];

typedef detail::make_cmpop<
	compute_greatereq,
	&JIVE_FLTGREATEREQ_NODE,
	fltgreatereq_name,
	jive_binary_operation_none> greatereq_operation;

}
}

jive::output *
jive_fltgreatereq(jive::output * operand1, jive::output * operand2);

#endif

