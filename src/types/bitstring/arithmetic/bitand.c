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
namespace bitstring {

and_operation::~and_operation() noexcept {}

bool
and_operation::operator==(const operation & other) const noexcept
{
	const and_operation * o = dynamic_cast<const and_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
and_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<and_operation>(
		*this,
		&JIVE_BITAND_NODE,
		region,
		narguments,
		arguments);
}

value_repr
and_operation::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	jive_bitstring_and(&result[0], &arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
and_operation::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
and_operation::debug_string() const
{
	return "BITAND";
}

}
}

const jive_bitbinary_operation_class JIVE_BITAND_NODE_ = {
	base : { /* jive_bitbinary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BITBINARY_NODE,
			name : "BITAND",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : nullptr,
			match_attrs : nullptr,
			check_operands : nullptr,
			create : nullptr,
		},

		flags : jive_binary_operation_associative | jive_binary_operation_commutative,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,

		can_reduce_operand_pair : nullptr,
		reduce_operand_pair : nullptr,
	},
	type : jive_bitop_code_and
};

jive::output *
jive_bitand(size_t noperands, jive::output * const * operands)
{
	return jive::bitstring::detail::binop_normalized_create<
		jive::bitstring::and_operation>(
			&JIVE_BITAND_NODE_.base, noperands, operands);
}
