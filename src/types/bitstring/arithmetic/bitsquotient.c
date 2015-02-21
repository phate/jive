/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitsquotient.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

sdiv_op::~sdiv_op() noexcept {}

bool
sdiv_op::operator==(const operation & other) const noexcept
{
	const sdiv_op * o = dynamic_cast<const sdiv_op *>(&other);
	return o && o->type() == type();
}
value_repr
sdiv_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	return arg1.sdiv(arg2);
}

jive_binary_operation_flags
sdiv_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sdiv_op::debug_string() const
{
	return "BITSQUOTIENT";
}

std::unique_ptr<jive::operation>
sdiv_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sdiv_op(*this));
}

}
}

jive::output *
jive_bitsquotient(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::sdiv_op(type),
		{op1, op2})[0];
}
