/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitand.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

and_op::~and_op() noexcept {}

bool
and_op::operator==(const operation & other) const noexcept
{
	const and_op * o = dynamic_cast<const and_op *>(&other);
	return o && o->type() == type();
}

jive_node *
and_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<and_op>(
		*this,
		region,
		narguments,
		arguments);
}

value_repr
and_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	jive_bitstring_and(&result[0], &arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
and_op::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
and_op::debug_string() const
{
	return "BITAND";
}

std::unique_ptr<jive::operation>
and_op::copy() const
{
	return std::unique_ptr<jive::operation>(new and_op(*this));
}

}
}

const jive_node_class JIVE_BITAND_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITAND",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr,
};

jive::output *
jive_bitand(size_t noperands, jive::output * const * operands)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(operands[0]->type());
	return jive_node_create_normalized(
		operands[0]->node()->graph,
		jive::bits::and_op(type),
		std::vector<jive::output *>(operands, operands + noperands))[0];
}
