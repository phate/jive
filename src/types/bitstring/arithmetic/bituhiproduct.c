/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bituhiproduct.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

umulh_op::~umulh_op() noexcept {}

bool
umulh_op::operator==(const operation & other) const noexcept
{
	const umulh_op * o = dynamic_cast<const umulh_op *>(&other);
	return o && o->type() == type();
}
value_repr
umulh_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char arg1ext[2 * nbits];
	char arg2ext[2 * nbits];
	char resultext[2 * nbits];
	jive_bitstring_extend_unsigned(arg1ext, 2 * nbits, &arg1[0], nbits);
	jive_bitstring_extend_unsigned(arg2ext, 2 * nbits, &arg2[0], nbits);
	jive_bitstring_product(
		resultext, 2 * nbits,
		arg1ext, 2 * nbits,
		arg2ext, 2 * nbits);

	value_repr result(resultext + nbits, resultext + 2 * nbits);
	return result;
}

jive_binary_operation_flags
umulh_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
umulh_op::debug_string() const
{
	return "BITUHIPRODUCT";
}

std::unique_ptr<jive::operation>
umulh_op::copy() const
{
	return std::unique_ptr<jive::operation>(new umulh_op(*this));
}

}
}

jive::output *
jive_bituhiproduct(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::umulh_op(type),
		{op1, op2})[0];
}
