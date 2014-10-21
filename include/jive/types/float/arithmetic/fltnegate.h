/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_ARITHMETIC_FLTNEGATE_H
#define JIVE_TYPES_FLOAT_ARITHMETIC_FLTNEGATE_H

#include <jive/types/float/fltoperation-classes.h>

namespace jive {
namespace flt {

struct compute_negation {
	inline value_repr operator()(value_repr v) const noexcept { return -v; }
};

extern const char fltnegate_name[];

typedef detail::make_unop<
	compute_negation,
	fltnegate_name> neg_op;


}
}

jive::output *
jive_fltnegate(jive::output * arg);

#endif
