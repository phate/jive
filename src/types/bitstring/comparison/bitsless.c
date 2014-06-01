/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitsless.h>

#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bitstring {

sless_operation::~sless_operation() noexcept {}

bool
sless_operation::operator==(const operation & other) const noexcept
{
	const sless_operation * o = dynamic_cast<const sless_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
sless_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<sless_operation>(
		*this,
		&JIVE_BITSLESS_NODE,
		region,
		arguments[0],
		arguments[1]);
}

compare_result
sless_operation::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char result = jive_bitstring_sless(
		&arg1[0], &arg2[0], nbits);

	switch (result) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
sless_operation::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sless_operation::debug_string() const
{
	return "BITSLESS";
}

}
}

const jive_bitcomparison_operation_class JIVE_BITSLESS_NODE_ = {
	base : { /* jive_binary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BITCOMPARISON_NODE,
			name : "BITSLESS",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
			get_label : nullptr,
			match_attrs : nullptr,
			check_operands : nullptr,
			create : nullptr,
		},
		flags : jive_binary_operation_none,
		single_apply_under : NULL,
		multi_apply_under : NULL,
		distributive_over : NULL,
		distributive_under : NULL,
	
		can_reduce_operand_pair : nullptr,
		reduce_operand_pair : nullptr
	},
	type : jive_bitcmp_code_sless,
	compare_constants : NULL
};

jive::output *
jive_bitsless(jive::output * operand1, jive::output * operand2)
{
	jive_graph * graph = operand1->node()->graph;
	return jive::bitstring::detail::binop_normalized_create<
		jive::bitstring::sless_operation>(
			&JIVE_BITSLESS_NODE_.base, operand1, operand2);
}
