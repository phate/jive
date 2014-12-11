/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitumod.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

umod_op::~umod_op() noexcept {}

bool
umod_op::operator==(const operation & other) const noexcept
{
	const umod_op * o = dynamic_cast<const umod_op *>(&other);
	return o && o->type() == type();
}

jive_node *
umod_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
}

value_repr
umod_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char quotient[nbits];
	value_repr result(nbits, '0');
	jive_bitstring_division_unsigned(
		quotient, &result[0],
		&arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
umod_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
umod_op::debug_string() const
{
	return "BITUMOD";
}

std::unique_ptr<jive::operation>
umod_op::copy() const
{
	return std::unique_ptr<jive::operation>(new umod_op(*this));
}

}
}

jive::output *
jive_bitumod(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::umod_op(type),
		{op1, op2})[0];
}
