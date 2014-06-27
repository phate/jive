/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitdifference.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

sub_op::~sub_op() noexcept {}

bool
sub_op::operator==(const operation & other) const noexcept
{
	const sub_op * o = dynamic_cast<const sub_op *>(&other);
	return o && o->type() == type();
}

jive_node *
sub_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<sub_op>(
		*this,
		&JIVE_BITDIFFERENCE_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
sub_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	jive_bitstring_difference(&result[0], &arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
sub_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sub_op::debug_string() const
{
	return "BITDIFFERENCE";
}

}
}

const jive_node_class JIVE_BITDIFFERENCE_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITDIFFERENCE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitdifference(jive::output * dividend, jive::output * divisor)
{
	jive_graph * graph = dividend->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::sub_op>(
			&JIVE_BITDIFFERENCE_NODE, dividend, divisor);
}
