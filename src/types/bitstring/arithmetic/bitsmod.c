/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitsmod.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

smod_op::~smod_op() noexcept {}

bool
smod_op::operator==(const operation & other) const noexcept
{
	const smod_op * o = dynamic_cast<const smod_op *>(&other);
	return o && o->type() == type();
}
value_repr
smod_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char quotient[nbits];
	value_repr result(nbits, '0');
	jive_bitstring_division_signed(
		quotient, &result[0],
		&arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
smod_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
smod_op::debug_string() const
{
	return "BITSMOD";
}

std::unique_ptr<jive::operation>
smod_op::copy() const
{
	return std::unique_ptr<jive::operation>(new smod_op(*this));
}

}
}

jive::output *
jive_bitsmod(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::smod_op(type),
		{op1, op2})[0];
}
