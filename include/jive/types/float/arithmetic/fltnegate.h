/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTNEGATE_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTNEGATE_H

#include <jive/types/float/fltoperation-classes.h>

extern const jive_node_class JIVE_FLTNEGATE_NODE;

namespace jive {
namespace flt {

value_repr compute_negation(value_repr arg);
extern const char fltnegate_name[];

typedef detail::make_unop<
	compute_negation,
	&JIVE_FLTNEGATE_NODE,
	fltnegate_name> negate_operation;


}
}

jive::output *
jive_fltnegate(jive::output * arg);

#endif
