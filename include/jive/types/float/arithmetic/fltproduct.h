/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTPRODUCT_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTPRODUCT_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_fltbinary_operation_class JIVE_FLTPRODUCT_NODE_;
#define JIVE_FLTPRODUCT_NODE (JIVE_FLTPRODUCT_NODE_.base.base)

namespace jive {
namespace flt {

value_repr compute_product(value_repr arg1, value_repr arg2);
extern const char fltproduct_name[];

typedef detail::make_binop<
	compute_product,
	&JIVE_FLTPRODUCT_NODE_,
	fltproduct_name,
	jive_binary_operation_commutative> product_operation;

}
}

jive::output *
jive_fltproduct(jive::output * operand1, jive::output * operand2);

#endif
