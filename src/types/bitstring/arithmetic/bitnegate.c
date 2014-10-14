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
namespace bits {

neg_op::~neg_op() noexcept {}

bool
neg_op::operator==(const operation & other) const noexcept
{
	const neg_op * o = dynamic_cast<const neg_op *>(&other);
	return o && o->type() == type();
}

jive_node *
neg_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::unop_create<neg_op>(
		*this,
		&JIVE_BITNEGATE_NODE,
		region,
		arguments[0]);
}

value_repr
neg_op::reduce_constant(
	const value_repr & arg) const
{
	value_repr result(arg);
	jive_bitstring_negate(&result[0], &arg[0], arg.size());
	return result;
}

std::string
neg_op::debug_string() const
{
	return "BITNEGATE";
}

std::unique_ptr<jive::operation>
neg_op::copy() const
{
	return std::unique_ptr<jive::operation>(new neg_op(*this));
}

}
}

const jive_node_class JIVE_BITNEGATE_NODE = {
	parent : &JIVE_BITUNARY_NODE,
	name : "BITNEGATE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitnegate(jive::output * arg)
{
	return jive::bits::detail::unop_normalized_create<
		jive::bits::neg_op>(
			&JIVE_BITNEGATE_NODE, arg);
}
