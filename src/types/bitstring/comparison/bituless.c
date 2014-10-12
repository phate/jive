/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bituless.h>

#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

ult_op::~ult_op() noexcept {}

bool
ult_op::operator==(const operation & other) const noexcept
{
	const ult_op * o = dynamic_cast<const ult_op *>(&other);
	return o && o->type() == type();
}

jive_node *
ult_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<ult_op>(
		*this,
		&JIVE_BITULESS_NODE,
		region,
		arguments[0],
		arguments[1]);
}

compare_result
ult_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char result = jive_bitstring_uless(
		&arg1[0], &arg2[0], nbits);

	switch (result) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
ult_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ult_op::debug_string() const
{
	return "BITULESS";
}

std::unique_ptr<jive::operation>
ult_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ult_op(*this));
}

}
}

const jive_node_class JIVE_BITULESS_NODE = {
	parent : &JIVE_BITCOMPARISON_NODE,
	name : "BITULESS",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bituless(jive::output * operand1, jive::output * operand2)
{
	jive_graph * graph = operand1->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::ult_op>(
			&JIVE_BITULESS_NODE, operand1, operand2);
}
