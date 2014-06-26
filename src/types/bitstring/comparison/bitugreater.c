/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitugreater.h>

#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

ugreater_operation::~ugreater_operation() noexcept {}

bool
ugreater_operation::operator==(const operation & other) const noexcept
{
	const ugreater_operation * o = dynamic_cast<const ugreater_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
ugreater_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<ugreater_operation>(
		*this,
		&JIVE_BITUGREATER_NODE,
		region,
		arguments[0],
		arguments[1]);
}

compare_result
ugreater_operation::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char result = jive_bitstring_ugreater(
		&arg1[0], &arg2[0], nbits);

	switch (result) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
ugreater_operation::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ugreater_operation::debug_string() const
{
	return "BITUGREATER";
}

}
}

const jive_node_class JIVE_BITUGREATER_NODE = {
	parent : &JIVE_BITCOMPARISON_NODE,
	name : "BITUGREATER",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitugreater(jive::output * operand1, jive::output * operand2)
{
	jive_graph * graph = operand1->node()->graph;
	return jive::bits::detail::binop_normalized_create<
		jive::bits::ugreater_operation>(
			&JIVE_BITUGREATER_NODE, operand1, operand2);
}
