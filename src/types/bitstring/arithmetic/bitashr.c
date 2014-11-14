/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitashr.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

ashr_op::~ashr_op() noexcept {}

bool
ashr_op::operator==(const operation & other) const noexcept
{
	const ashr_op * o = dynamic_cast<const ashr_op *>(&other);
	return o && o->type() == type();
}
value_repr
ashr_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.nbits(), arg2.nbits());
	value_repr result(nbits, '0');
	uint64_t shift = jive_bitstring_to_unsigned(
		&arg2[0], arg2.nbits());
	jive_bitstring_arithmetic_shiftright(&result[0], &arg1[0], nbits, shift);
	return result;
}

jive_binary_operation_flags
ashr_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ashr_op::debug_string() const
{
	return "BITASHR";
}

std::unique_ptr<jive::operation>
ashr_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ashr_op(*this));
}

}
}

jive::output *
jive_bitashr(jive::output * operand, jive::output * shift)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(operand->type());
	return jive_node_create_normalized(
		operand->node()->graph,
		jive::bits::ashr_op(type),
		{operand, shift})[0];
}
