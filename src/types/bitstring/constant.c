/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/constant.h>

namespace jive {
namespace base {
// explicit instantiation
template class domain_const_op<
	bittype, bitvalue_repr, format_value, type_of_value
>;
}
}
