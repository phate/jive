/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_VALUE_REPRESENTATION_H
#define JIVE_TYPES_BITSTRING_VALUE_REPRESENTATION_H

#include <cstdint>
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

static inline value_repr
value_repr_from_int(size_t nbits, int64_t value)
{
	value_repr result(nbits);
	for (size_t n = 0; n < nbits; ++n) {
		result[n] = '0' + (value & 1);
		value = value >> 1;
	}
	return result;
}

static inline value_repr
value_repr_from_uint(size_t nbits, uint64_t value)
{
	value_repr result(nbits);
	for (size_t n = 0; n < nbits; ++n) {
		result[n] = '0' + (value & 1);
		value = value >> 1;
	}
	return result;
}

uint64_t
value_repr_to_uint(const value_repr & value);

int64_t
value_repr_to_int(const value_repr & value);

}
}

#endif
