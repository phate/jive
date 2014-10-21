/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_COMPARISON_FLTLESSEQ_H
#define JIVE_TYPES_FLOAT_COMPARISON_FLTLESSEQ_H

#include <jive/types/float/fltoperation-classes.h>

namespace jive {
namespace flt {

extern const char fltlesseq_name[];

typedef detail::make_cmpop<
	std::less_equal<value_repr>,
	fltlesseq_name,
	jive_binary_operation_none> le_op;

}
}

jive::output *
jive_fltlesseq(jive::output * operand1, jive::output * operand2);

#endif
