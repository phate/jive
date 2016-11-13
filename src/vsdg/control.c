/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/control.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/types/bitstring/type.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/match.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace base {
// explicit instantiation
template class domain_const_op<
	ctl::type, ctl::value_repr, ctl::format_value, ctl::type_of_value
>;
}

namespace ctl {

jive::oport *
match(
	size_t nbits,
	const std::map<uint64_t, uint64_t> & mapping,
	uint64_t default_alternative,
	size_t nalternatives,
	jive::oport * operand)
{
	match_op op(nbits, mapping, default_alternative, nalternatives);
	return jive_node_create_normalized(operand->region(), op, {operand})[0];
}

}
}

jive::oport *
jive_control_constant(jive::region * region, size_t nalternatives, size_t alternative)
{
	jive::ctl::constant_op op(jive::ctl::value_repr(alternative, nalternatives));
	return jive_node_create_normalized(region, op, {})[0];
}
