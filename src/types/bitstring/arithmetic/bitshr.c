/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitshr.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

shr_op::~shr_op() noexcept {}

bool
shr_op::operator==(const operation & other) const noexcept
{
	const shr_op * o = dynamic_cast<const shr_op *>(&other);
	return o && o->type() == type();
}

jive_node *
shr_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<shr_op>(
		*this,
		&JIVE_BITSHR_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
shr_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	uint64_t shift = jive_bitstring_to_unsigned(
		&arg2[0], arg2.size());
	jive_bitstring_shiftright(&result[0], &arg1[0], nbits, shift);
	return result;
}

jive_binary_operation_flags
shr_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
shr_op::debug_string() const
{
	return "BITSHR";
}

std::unique_ptr<jive::operation>
shr_op::copy() const
{
	return std::unique_ptr<jive::operation>(new shr_op(*this));
}

}
}

const jive_node_class JIVE_BITSHR_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITSHR",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitshr(jive::output * operand, jive::output * shift)
{
	jive_graph * graph = operand->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::shr_op>(
			&JIVE_BITSHR_NODE, operand, shift);
}
