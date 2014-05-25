/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_VALUE_REPRESENTATION_H
#define JIVE_TYPES_BITSTRING_VALUE_REPRESENTATION_H

#include <vector>

namespace jive {
namespace bitstring {

// Value representation used for compile-time evaluation of bitstring
// expressions.
//
// FIXME: We currently use a "vector of chars" where each character is either
// '0' or '1'. This representation is subject to change.
typedef std::vector<char> value_repr;

}
}

#endif
