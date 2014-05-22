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
namespace bitstring {

shr_operation::~shr_operation() noexcept {}

bool
shr_operation::operator==(const operation & other) const noexcept
{
	const shr_operation * o = dynamic_cast<const shr_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
shr_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive_output * const arguments[]) const
{
	return detail::binop_create<shr_operation>(
		*this,
		&JIVE_BITSHR_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
shr_operation::reduce_constants(
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
shr_operation::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
shr_operation::debug_string() const
{
	return "BITSHR";
}

}
}

const jive_bitbinary_operation_class JIVE_BITSHR_NODE_ = {
	base : { /* jive_bitbinary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BITBINARY_NODE,
			name : "BITSHR",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : nullptr,
			match_attrs : nullptr,
			check_operands : nullptr,
			create : nullptr
		},

		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : nullptr,
		reduce_operand_pair : nullptr,
	},
	type : jive_bitop_code_shr
};

jive_output *
jive_bitshr(jive_output * operand, jive_output * shift)
{
	jive_graph * graph = operand->node->graph;
	return jive::bitstring::detail::binop_normalized_create<
		jive::bitstring::shr_operation>(
			&JIVE_BITSHR_NODE_.base, operand, shift);
}
