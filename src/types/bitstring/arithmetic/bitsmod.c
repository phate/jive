/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitsmod.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

smod_op::~smod_op() noexcept {}

bool
smod_op::operator==(const operation & other) const noexcept
{
	const smod_op * o = dynamic_cast<const smod_op *>(&other);
	return o && o->type() == type();
}

jive_node *
smod_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<smod_op>(
		*this,
		&JIVE_BITSMOD_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
smod_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char quotient[nbits];
	value_repr result(nbits, '0');
	jive_bitstring_division_signed(
		quotient, &result[0],
		&arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
smod_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
smod_op::debug_string() const
{
	return "BITSMOD";
}

std::unique_ptr<jive::operation>
smod_op::copy() const
{
	return std::unique_ptr<jive::operation>(new smod_op(*this));
}

}
}

const jive_node_class JIVE_BITSMOD_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITSMOD",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitsmod(jive::output * dividend, jive::output * divisor)
{
	jive_graph * graph = dividend->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::smod_op>(
			&JIVE_BITSMOD_NODE, dividend, divisor);
}
