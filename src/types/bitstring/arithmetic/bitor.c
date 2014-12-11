/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitor.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

or_op::~or_op() noexcept {}

bool
or_op::operator==(const operation & other) const noexcept
{
	const or_op * o = dynamic_cast<const or_op *>(&other);
	return o && o->type() == type();
}

jive_node *
or_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
}

value_repr
or_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	jive_bitstring_or(&result[0], &arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
or_op::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
or_op::debug_string() const
{
	return "BITOR";
}

std::unique_ptr<jive::operation>
or_op::copy() const
{
	return std::unique_ptr<jive::operation>(new or_op(*this));
}

}
}

jive::output *
jive_bitor(size_t noperands, jive::output * const * operands)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(operands[0]->type());
	return jive_node_create_normalized(
		operands[0]->node()->graph,
		jive::bits::or_op(type),
		std::vector<jive::output *>(operands, operands + noperands))[0];
}
