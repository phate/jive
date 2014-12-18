/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitproduct.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

mul_op::~mul_op() noexcept {}

bool
mul_op::operator==(const operation & other) const noexcept
{
	const mul_op * o = dynamic_cast<const mul_op *>(&other);
	return o && o->type() == type();
}
value_repr
mul_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	jive_bitstring_product(
		&result[0], nbits,
		&arg1[0], nbits,
		&arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
mul_op::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
mul_op::debug_string() const
{
	return "BITPRODUCT";
}

std::unique_ptr<jive::operation>
mul_op::copy() const
{
	return std::unique_ptr<jive::operation>(new mul_op(*this));
}

}
}

jive::output *
jive_bitmultiply(size_t noperands, jive::output * const * operands)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(operands[0]->type());
	return jive_node_create_normalized(
		operands[0]->node()->graph,
		jive::bits::mul_op(type),
		std::vector<jive::output *>(operands, operands + noperands))[0];
}
