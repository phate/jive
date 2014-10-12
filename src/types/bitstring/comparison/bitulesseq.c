/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitulesseq.h>

#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

ule_op::~ule_op() noexcept {}

bool
ule_op::operator==(const operation & other) const noexcept
{
	const ule_op * o = dynamic_cast<const ule_op *>(&other);
	return o && o->type() == type();
}

jive_node *
ule_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<ule_op>(
		*this,
		&JIVE_BITULESSEQ_NODE,
		region,
		arguments[0],
		arguments[1]);
}

compare_result
ule_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char result = jive_bitstring_ulesseq(
		&arg1[0], &arg2[0], nbits);

	switch (result) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
ule_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ule_op::debug_string() const
{
	return "BITULESSEQ";
}

std::unique_ptr<jive::operation>
ule_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ule_op(*this));
}

}
}

const jive_node_class JIVE_BITULESSEQ_NODE = {
	parent : &JIVE_BITCOMPARISON_NODE,
	name : "BITULESSEQ",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitulesseq(jive::output * operand1, jive::output * operand2)
{
	jive_graph * graph = operand1->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::ule_op>(
			&JIVE_BITULESSEQ_NODE, operand1, operand2);
}
