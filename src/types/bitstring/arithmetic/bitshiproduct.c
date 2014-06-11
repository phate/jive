/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitshiproduct.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bitstring {

shiproduct_operation::~shiproduct_operation() noexcept {}

bool
shiproduct_operation::operator==(const operation & other) const noexcept
{
	const shiproduct_operation * o = dynamic_cast<const shiproduct_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
shiproduct_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<shiproduct_operation>(
		*this,
		&JIVE_BITSHIPRODUCT_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
shiproduct_operation::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char arg1ext[2 * nbits];
	char arg2ext[2 * nbits];
	char resultext[2 * nbits];
	jive_bitstring_extend_signed(arg1ext, 2 * nbits, &arg1[0], nbits);
	jive_bitstring_extend_signed(arg2ext, 2 * nbits, &arg2[0], nbits);
	jive_bitstring_product(
		resultext, 2 * nbits,
		arg1ext, 2 * nbits,
		arg2ext, 2 * nbits);

	value_repr result(resultext + nbits, resultext + 2 * nbits);
	return result;
}

jive_binary_operation_flags
shiproduct_operation::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
shiproduct_operation::debug_string() const
{
	return "BITSHIPRODUCT";
}

}
}

const jive_bitbinary_operation_class JIVE_BITSHIPRODUCT_NODE_ = {
	base : { /* jive_bitbinary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BITBINARY_NODE,
			name : "BITSHIPRODUCT",
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
	type : jive_bitop_code_shiproduct
};

jive::output *
jive_bitshiproduct(jive::output * dividend, jive::output * divisor)
{
	jive_graph * graph = dividend->node()->graph;
	return jive::bitstring::detail::binop_normalized_create<
		jive::bitstring::shiproduct_operation>(
			&JIVE_BITSHIPRODUCT_NODE_.base, dividend, divisor);
}
