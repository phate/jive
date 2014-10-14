/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitnot.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

not_op::~not_op() noexcept {}

bool
not_op::operator==(const operation & other) const noexcept
{
	const not_op * o = dynamic_cast<const not_op *>(&other);
	return o && o->type() == type();
}

jive_node *
not_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::unop_create<not_op>(
		*this,
		&JIVE_BITNOT_NODE,
		region,
		arguments[0]);
}

value_repr
not_op::reduce_constant(
	const value_repr & arg) const
{
	value_repr result(arg);
	jive_bitstring_not(&result[0], &arg[0], arg.size());
	return result;
}

std::string
not_op::debug_string() const
{
	return "BITNOT";
}

std::unique_ptr<jive::operation>
not_op::copy() const
{
	return std::unique_ptr<jive::operation>(new not_op(*this));
}

}
}

const jive_node_class JIVE_BITNOT_NODE = {
	parent : &JIVE_BITUNARY_NODE,
	name : "BITNOT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_unary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr,
};

jive::output *
jive_bitnot(jive::output * arg)
{
	return jive::bits::detail::unop_normalized_create<
		jive::bits::not_op>(
			&JIVE_BITNOT_NODE, arg);
}
