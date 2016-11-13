/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_BITSTRING_CONSTANT_H
#define JIVE_TYPES_BITSTRING_CONSTANT_H

#include <stdint.h>
#include <vector>

#include <jive/types/bitstring/type.h>
#include <jive/types/bitstring/value-representation.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>

namespace jive {
namespace bits {

struct type_of_value {
	type operator()(const value_repr & repr) const
	{
		return type(repr.nbits());
	}
};

struct format_value {
	std::string operator()(const value_repr & repr) const
	{
		return repr.str();
	}
};

typedef base::domain_const_op<
	type, value_repr, format_value, type_of_value
> constant_op;

inline constant_op
uint_constant_op(size_t nbits, uint64_t value)
{
	return constant_op(value_repr(nbits, value));
}

inline constant_op
int_constant_op(size_t nbits, int64_t value)
{
	return constant_op(value_repr(nbits, value));
}

}

namespace base {
// declare explicit instantiation
extern template class domain_const_op<
	bits::type, bits::value_repr, bits::format_value, bits::type_of_value
>;
}

}

/**
	\brief Create bitconstant
	\param graph Graph to create constant in
	\param nbits Number of bits
	\param bits Values of bits
	\returns Bitstring value representing constant
	
	Convenience function that either creates a new constant or
	returns the output handle of an existing constant.
*/
jive::oport *
jive_bitconstant(jive::region * region, size_t nbits, const char bits[]);

jive::oport *
jive_bitconstant_unsigned(jive::region * region, size_t nbits, uint64_t value);

jive::oport *
jive_bitconstant_signed(jive::region * region, size_t nbits, int64_t value);

JIVE_EXPORTED_INLINE jive::oport *
jive_bitconstant_undefined(jive::region * region, size_t nbits)
{
	size_t i;
	char bits[nbits];
	for (i = 0; i < nbits; i++)
		bits[i] = 'X';

	return jive_bitconstant(region, nbits, bits);
}

JIVE_EXPORTED_INLINE jive::oport *
jive_bitconstant_defined(jive::region * region, size_t nbits)
{
	size_t i;
	char bits[nbits];
	for (i = 0; i < nbits; i++)
		bits[i] = 'D';

	return jive_bitconstant(region, nbits, bits);
}

#endif
