/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitxor.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

xor_op::~xor_op() noexcept {}

bool
xor_op::operator==(const operation & other) const noexcept
{
	const xor_op * o = dynamic_cast<const xor_op *>(&other);
	return o && o->type() == type();
}

jive_node *
xor_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<xor_op>(
		*this,
		region,
		narguments,
		arguments);
}

value_repr
xor_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	jive_bitstring_xor(&result[0], &arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
xor_op::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
xor_op::debug_string() const
{
	return "BITXOR";
}

std::unique_ptr<jive::operation>
xor_op::copy() const
{
	return std::unique_ptr<jive::operation>(new xor_op(*this));
}

}
}

jive::output *
jive_bitxor(size_t noperands, jive::output * const * operands)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(operands[0]->type());
	return jive_node_create_normalized(
		operands[0]->node()->graph,
		jive::bits::xor_op(type),
		std::vector<jive::output *>(operands, operands + noperands))[0];
}
