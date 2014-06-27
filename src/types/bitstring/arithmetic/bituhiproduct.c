/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bituhiproduct.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

umulh_op::~umulh_op() noexcept {}

bool
umulh_op::operator==(const operation & other) const noexcept
{
	const umulh_op * o = dynamic_cast<const umulh_op *>(&other);
	return o && o->type() == type();
}

jive_node *
umulh_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<umulh_op>(
		*this,
		&JIVE_BITUHIPRODUCT_NODE,
		region,
		arguments[0],
		arguments[1]);
}

value_repr
umulh_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char arg1ext[2 * nbits];
	char arg2ext[2 * nbits];
	char resultext[2 * nbits];
	jive_bitstring_extend_unsigned(arg1ext, 2 * nbits, &arg1[0], nbits);
	jive_bitstring_extend_unsigned(arg2ext, 2 * nbits, &arg2[0], nbits);
	jive_bitstring_product(
		resultext, 2 * nbits,
		arg1ext, 2 * nbits,
		arg2ext, 2 * nbits);

	value_repr result(resultext + nbits, resultext + 2 * nbits);
	return result;
}

jive_binary_operation_flags
umulh_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
umulh_op::debug_string() const
{
	return "BITUHIPRODUCT";
}

}
}

const jive_node_class JIVE_BITUHIPRODUCT_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITUHIPRODUCT",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bituhiproduct(jive::output * dividend, jive::output * divisor)
{
	jive_graph * graph = dividend->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::umulh_op>(
			&JIVE_BITUHIPRODUCT_NODE, dividend, divisor);
}
