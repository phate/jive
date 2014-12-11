/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitnotequal.h>

#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

ne_op::~ne_op() noexcept {}

bool
ne_op::operator==(const operation & other) const noexcept
{
	const ne_op * o = dynamic_cast<const ne_op *>(&other);
	return o && o->type() == type();
}

jive_node *
ne_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
}

compare_result
ne_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char result = jive_bitstring_notequal(
		&arg1[0], &arg2[0], nbits);

	switch (result) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
ne_op::flags() const noexcept
{
	return jive_binary_operation_commutative;
}

std::string
ne_op::debug_string() const
{
	return "BITNOTEQUAL";
}

std::unique_ptr<jive::operation>
ne_op::copy() const
{
	return std::unique_ptr<jive::operation>(new ne_op(*this));
}

}
}

jive::output *
jive_bitnotequal(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::ne_op(type),
		{op1, op2})[0];
}
