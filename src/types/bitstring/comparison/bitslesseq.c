/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitslesseq.h>

#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

slesseq_operation::~slesseq_operation() noexcept {}

bool
slesseq_operation::operator==(const operation & other) const noexcept
{
	const slesseq_operation * o = dynamic_cast<const slesseq_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
slesseq_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<slesseq_operation>(
		*this,
		&JIVE_BITSLESSEQ_NODE,
		region,
		arguments[0],
		arguments[1]);
}

compare_result
slesseq_operation::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char result = jive_bitstring_slesseq(
		&arg1[0], &arg2[0], nbits);

	switch (result) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
slesseq_operation::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
slesseq_operation::debug_string() const
{
	return "BITSLESSEQ";
}

}
}

const jive_node_class JIVE_BITSLESSEQ_NODE = {
	parent : &JIVE_BITCOMPARISON_NODE,
	name : "BITSLESSEQ",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitslesseq(jive::output * operand1, jive::output * operand2)
{
	jive_graph * graph = operand1->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::slesseq_operation>(
			&JIVE_BITSLESSEQ_NODE, operand1, operand2);
}
