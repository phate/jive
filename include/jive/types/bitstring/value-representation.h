/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_VALUE_REPRESENTATION_H
#define JIVE_TYPES_BITSTRING_VALUE_REPRESENTATION_H

#include <cstring>
#include <vector>

namespace jive {
namespace bits {

// Value representation used for compile-time evaluation of bitstring
// expressions.
//
// FIXME: We currently use a "vector of chars" where each character is either
// '0' or '1'. This representation is subject to change.
typedef std::vector<char> value_repr;

static inline value_repr
value_repr_from_string(const char * s)
{
	return value_repr(s, s + strlen(s));
}

}
}

#endif
