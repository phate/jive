/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitnegate.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bitstring {

negate_operation::~negate_operation() noexcept {}

bool
negate_operation::operator==(const operation & other) const noexcept
{
	const negate_operation * o = dynamic_cast<const negate_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
negate_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive_output * const arguments[]) const
{
	return detail::unop_create<negate_operation>(
		*this,
		&JIVE_BITNEGATE_NODE,
		region,
		arguments[0]);
}

value_repr
negate_operation::reduce_constant(
	const value_repr & arg) const
{
	value_repr result(arg);
	jive_bitstring_negate(&result[0], &arg[0], arg.size());
	return result;
}

std::string
negate_operation::debug_string() const
{
	return "BITNEGATE";
}

}
}

const jive_bitunary_operation_class JIVE_BITNEGATE_NODE_ = {
	base : { /* jive_unary_operation_class */
		base : { /* jive_node_class */
			parent : &JIVE_BITUNARY_NODE,
			name : "BITNEGATE",
			fini : jive_node_fini_, /* inherit */
			get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
			get_label : nullptr,
			match_attrs : nullptr,
			check_operands : nullptr,
			create : nullptr,
		},
		
		single_apply_over : NULL,
		multi_apply_over : NULL,
		
		can_reduce_operand : nullptr,
		reduce_operand : nullptr,
	},
	type : jive_bitop_code_negate
};

jive_output *
jive_bitnegate(jive_output * arg)
{
	return jive::bitstring::detail::unop_normalized_create<
		jive::bitstring::negate_operation>(
			&JIVE_BITNEGATE_NODE_.base, arg);
}
