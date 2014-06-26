/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitsquotient.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

squotient_operation::~squotient_operation() noexcept {}

bool
squotient_operation::operator==(const operation & other) const noexcept
{
	const squotient_operation * o = dynamic_cast<const squotient_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
squotient_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<squotient_operation>(
		*this,
		&JIVE_BITSQUOTIENT_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
squotient_operation::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char remainder[nbits];
	value_repr result(nbits, '0');
	jive_bitstring_division_signed(
		&result[0], remainder,
		&arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
squotient_operation::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
squotient_operation::debug_string() const
{
	return "BITSQUOTIENT";
}

}
}

const jive_node_class JIVE_BITSQUOTIENT_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITSQUOTIENT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitsquotient(jive::output * dividend, jive::output * divisor)
{
	jive_graph * graph = dividend->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::squotient_operation>(
			&JIVE_BITSQUOTIENT_NODE, dividend, divisor);
}
