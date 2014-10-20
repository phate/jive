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

ugt_op::~ugt_op() noexcept {}

bool
ugt_op::operator==(const operation & other) const noexcept
{
	const ugt_op * o = dynamic_cast<const ugt_op *>(&other);
	return o && o->type() == type();
}

jive_node *
ugt_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<ugt_op>(
		*this,
		&JIVE_BITUGREATER_NODE,
		region,
		arguments[0],
		arguments[1]);
}

compare_result
ugt_op::reduce_constants(
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
ugt_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
ugt_op::debug_string() const
{
	return "BITUGREATER";
}

std::unique_ptr<jive::operation>
ugt_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ugt_op(*this));
}

}
}

const jive_node_class JIVE_BITUGREATER_NODE = {
	parent : &JIVE_BITCOMPARISON_NODE,
	name : "BITUGREATER",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_bitugreater(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::ugt_op(type),
		{op1, op2})[0];
}
