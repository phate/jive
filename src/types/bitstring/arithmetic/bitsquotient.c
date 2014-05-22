/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitsquotient.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bitstring {

squotient_operation::~squotient_operation() noexcept {}

bool
squotient_operation::operator==(const operation & other) const noexcept
{
	const squotient_operation * o = dynamic_cast<const squotient_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
squotient_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive_output * const arguments[]) const
{
	return detail::binop_create<squotient_operation>(
		*this,
		&JIVE_BITSQUOTIENT_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
squotient_operation::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char remainder[nbits];
	value_repr result(nbits, '0');
	jive_bitstring_division_signed(
		&result[0], remainder,
		&arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
squotient_operation::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
squotient_operation::debug_string() const
{
	return "BITSQUOTIENT";
}

}
}

const jive_bitbinary_operation_class JIVE_BITSQUOTIENT_NODE_ = {
	base : { /* jive_bitbinary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BITBINARY_NODE,
			name : "BITSQUOTIENT",
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
	type : jive_bitop_code_squotient
};

jive_output *
jive_bitsquotient(jive_output * dividend, jive_output * divisor)
{
	jive_graph * graph = dividend->node->graph;
	return jive::bitstring::detail::binop_normalized_create<
		jive::bitstring::squotient_operation>(
			&JIVE_BITSQUOTIENT_NODE_.base, dividend, divisor);
}
