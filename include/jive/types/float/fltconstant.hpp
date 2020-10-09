/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_TYPES_FLOAT_FLTCONSTANT_HPP
#define JIVE_TYPES_FLOAT_FLTCONSTANT_HPP

#include <jive/rvsdg/node.hpp>
#include <jive/rvsdg/nullary.hpp>
#include <jive/types/float/flttype.hpp>
#include <jive/types/float/value-representation.hpp>

namespace jive {
namespace flt {

struct type_of_value {
	type operator()(const value_repr & repr) const
	{
		return type();
	}
};

struct format_value {
	std::string operator()(const value_repr & repr) const
	{
		return jive::detail::strfmt(repr);
	}
};

typedef domain_const_op<type, value_repr, format_value, type_of_value> constant_op;

}

// declare explicit instantiation
extern template class domain_const_op<
	type, flt::value_repr, flt::format_value, flt::type_of_value
>;
}

jive::output *
jive_fltconstant(jive::region * region, jive::flt::value_repr value);

static inline jive::output *
jive_fltconstant_float(jive::region * region, float value)
{
	return jive_fltconstant(region, value);
}

#endif
