/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitshr.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

shr_op::~shr_op() noexcept {}

bool
shr_op::operator==(const operation & other) const noexcept
{
	const shr_op * o = dynamic_cast<const shr_op *>(&other);
	return o && o->type() == type();
}
value_repr
shr_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.shr(arg2.to_uint());
}

jive_binary_operation_flags
shr_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
shr_op::debug_string() const
{
	return "BITSHR";
}

std::unique_ptr<jive::operation>
shr_op::copy() const
{
	return std::unique_ptr<jive::operation>(new shr_op(*this));
}

}
}

jive::output *
jive_bitshr(jive::output * operand, jive::output * shift)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(operand->type());
	return jive_node_create_normalized(
		operand->node()->graph,
		jive::bits::shr_op(type),
		{operand, shift})[0];
}
