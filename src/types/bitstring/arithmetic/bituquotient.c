/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bituquotient.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

udiv_op::~udiv_op() noexcept {}

bool
udiv_op::operator==(const operation & other) const noexcept
{
	const udiv_op * o = dynamic_cast<const udiv_op *>(&other);
	return o && o->type() == type();
}

jive_node *
udiv_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<udiv_op>(
		*this,
		&JIVE_BITUQUOTIENT_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
udiv_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char remainder[nbits];
	value_repr result(nbits, '0');
	jive_bitstring_division_unsigned(
		&result[0], remainder,
		&arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
udiv_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
udiv_op::debug_string() const
{
	return "BITUQUOTIENT";
}

std::unique_ptr<jive::operation>
udiv_op::copy() const
{
	return std::unique_ptr<jive::operation>(new udiv_op(*this));
}

}
}

const jive_node_class JIVE_BITUQUOTIENT_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITUQUOTIENT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bituquotient(jive::output * dividend, jive::output * divisor)
{
	jive_graph * graph = dividend->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::udiv_op>(
			&JIVE_BITUQUOTIENT_NODE, dividend, divisor);
}
