/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_VALUE_REPRESENTATION_HPP
#define JIVE_TYPES_FLOAT_VALUE_REPRESENTATION_HPP

#include <vector>

namespace jive {
namespace flt {

// Value representation used for compile-time evaluation of bitstring
// expressions.
//
// FIXME: Uses machine float, should probably have own internal format.
typedef float value_repr;

}
}

#endif
