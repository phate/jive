/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitashr.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
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

jive_node *
ashr_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<ashr_op>(
		*this,
		&JIVE_BITASHR_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
ashr_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	uint64_t shift = jive_bitstring_to_unsigned(
		&arg2[0], arg2.size());
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

}
}

const jive_node_class JIVE_BITASHR_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITASHR",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitashr(jive::output * operand, jive::output * shift)
{
	jive_graph * graph = operand->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::ashr_op>(
			&JIVE_BITASHR_NODE, operand, shift);
}
