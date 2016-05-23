/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_VSDG_CONTROL_H
#define JIVE_VSDG_CONTROL_H

#include <jive/util/strfmt.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/gamma.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators/nullary.h>

#include <map>

#include <inttypes.h>

namespace jive {
namespace ctl {

struct type_of_value {
	type operator()(const value_repr & repr) const
	{
		return type(repr.nalternatives());
	}
};

struct format_value {
	std::string operator()(const value_repr & repr) const
	{
		return jive::detail::strfmt(repr.alternative());
	}
};

typedef base::domain_const_op<
	type, value_repr, format_value, type_of_value
> constant_op;

jive::output *
match(
	size_t nbits,
	const std::map<uint64_t, uint64_t> & mapping,
	uint64_t default_alternative,
	size_t nalternatives,
	jive::output * operand);

}

namespace base {
// declare explicit instantiation
extern template class domain_const_op<
	ctl::type, ctl::value_repr, ctl::format_value, ctl::type_of_value
>;
}

}

jive::output *
jive_control_constant(jive_region * region, size_t nalternatives, size_t alternative);

JIVE_EXPORTED_INLINE jive::output *
jive_control_false(jive_region * region)
{
	return jive_control_constant(region, 2, 0);
}

JIVE_EXPORTED_INLINE jive::output *
jive_control_true(jive_region * region)
{
	return jive_control_constant(region, 2, 1);
}

#endif
