/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitshl.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

shl_op::~shl_op() noexcept {}

bool
shl_op::operator==(const operation & other) const noexcept
{
	const shl_op * o = dynamic_cast<const shl_op *>(&other);
	return o && o->type() == type();
}

jive_node *
shl_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
}

value_repr
shl_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	uint64_t shift = jive_bitstring_to_unsigned(
		&arg2[0], arg2.size());
	jive_bitstring_shiftleft(&result[0], &arg1[0], nbits, shift);
	return result;
}

jive_binary_operation_flags
shl_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
shl_op::debug_string() const
{
	return "BITSHL";
}

std::unique_ptr<jive::operation>
shl_op::copy() const
{
	return std::unique_ptr<jive::operation>(new shl_op(*this));
}

}
}

jive::output *
jive_bitshl(jive::output * operand, jive::output * shift)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(operand->type());
	return jive_node_create_normalized(
		operand->node()->graph,
		jive::bits::shl_op(type),
		{operand, shift})[0];
}
