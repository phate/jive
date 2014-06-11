/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitsmod.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bitstring {

smod_operation::~smod_operation() noexcept {}

bool
smod_operation::operator==(const operation & other) const noexcept
{
	const smod_operation * o = dynamic_cast<const smod_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
smod_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<smod_operation>(
		*this,
		&JIVE_BITSMOD_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
smod_operation::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char quotient[nbits];
	value_repr result(nbits, '0');
	jive_bitstring_division_signed(
		quotient, &result[0],
		&arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
smod_operation::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
smod_operation::debug_string() const
{
	return "BITSMOD";
}

}
}

const jive_bitbinary_operation_class JIVE_BITSMOD_NODE_ = {
	base : { /* jive_bitbinary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BITBINARY_NODE,
			name : "BITSMOD",
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
	type : jive_bitop_code_smod
};

jive::output *
jive_bitsmod(jive::output * dividend, jive::output * divisor)
{
	jive_graph * graph = dividend->node()->graph;
	return jive::bitstring::detail::binop_normalized_create<
		jive::bitstring::smod_operation>(
			&JIVE_BITSMOD_NODE_.base, dividend, divisor);
}
