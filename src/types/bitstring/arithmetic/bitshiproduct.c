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
namespace bits {

smulh_op::~smulh_op() noexcept {}

bool
smulh_op::operator==(const operation & other) const noexcept
{
	const smulh_op * o = dynamic_cast<const smulh_op *>(&other);
	return o && o->type() == type();
}

jive_node *
smulh_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<smulh_op>(
		*this,
		&JIVE_BITSHIPRODUCT_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
smulh_op::reduce_constants(
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
smulh_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
smulh_op::debug_string() const
{
	return "BITSHIPRODUCT";
}

}
}

const jive_node_class JIVE_BITSHIPRODUCT_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITSHIPRODUCT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitshiproduct(jive::output * dividend, jive::output * divisor)
{
	jive_graph * graph = dividend->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::smulh_op>(
			&JIVE_BITSHIPRODUCT_NODE, dividend, divisor);
}
