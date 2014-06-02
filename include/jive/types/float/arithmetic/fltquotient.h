/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTQUOTIENT_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTQUOTIENT_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltbinary_operation_class JIVE_FLTQUOTIENT_NODE_;
#define JIVE_FLTQUOTIENT_NODE (JIVE_FLTQUOTIENT_NODE_.base.base)

namespace jive {
namespace flt {

value_repr compute_quotient(value_repr arg1, value_repr arg2);
extern const char fltquotient_name[];

typedef detail::make_binop<
	compute_quotient,
	&JIVE_FLTQUOTIENT_NODE_,
	fltquotient_name,
	jive_binary_operation_none> quotient_operation;

}
}

jive::output *
jive_fltquotient(jive::output * operand1, jive::output * operand2);

#endif
