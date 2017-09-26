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
#include <jive/vsdg/nullary.h>
#include <jive/vsdg/simple_node.h>

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
		if (repr.is_known() && repr.nbits() <= 64)
			return detail::strfmt("BITS", repr.nbits(), "(", repr.to_uint(), ")");

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

static inline jive::output *
create_bitconstant(jive::region * region, const bits::value_repr & vr)
{
	return create_normalized(region, bits::constant_op(vr), {})[0];
}

static inline jive::output *
create_bitconstant(jive::region * region, size_t nbits, int64_t value)
{
	return create_bitconstant(region, {nbits, value});
}

static inline jive::output *
create_bitconstant_undefined(jive::region * region, size_t nbits)
{
	std::string s(nbits, 'X');
	return create_bitconstant(region, bits::value_repr(s.c_str()));
}

static inline jive::output *
create_bitconstant_defined(jive::region * region, size_t nbits)
{
	std::string s(nbits, 'D');
	return create_bitconstant(region, bits::value_repr(s.c_str()));
}

}

#endif
