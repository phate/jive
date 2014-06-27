/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitequal.h>

#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

eq_op::~eq_op() noexcept {}

bool
eq_op::operator==(const operation & other) const noexcept
{
	const eq_op * o = dynamic_cast<const eq_op *>(&other);
	return o && o->type() == type();
}

jive_node *
eq_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<eq_op>(
		*this,
		&JIVE_BITEQUAL_NODE,
		region,
		arguments[0],
		arguments[1]);
}

compare_result
eq_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char result = jive_bitstring_equal(
		&arg1[0], &arg2[0], nbits);

	switch (result) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
eq_op::flags() const noexcept
{
	return jive_binary_operation_commutative;
}

std::string
eq_op::debug_string() const
{
	return "BITEQUAL";
}

}
}

const jive_node_class JIVE_BITEQUAL_NODE = {
	parent : &JIVE_BITCOMPARISON_NODE,
	name : "BITEQUAL",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitequal(jive::output * operand1, jive::output * operand2)
{
	jive_graph * graph = operand1->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::eq_op>(
			&JIVE_BITEQUAL_NODE, operand1, operand2);
}
