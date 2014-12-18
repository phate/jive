/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitugreatereq.h>

#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

uge_op::~uge_op() noexcept {}

bool
uge_op::operator==(const operation & other) const noexcept
{
	const uge_op * o = dynamic_cast<const uge_op *>(&other);
	return o && o->type() == type();
}
compare_result
uge_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	char result = jive_bitstring_ugreatereq(
		&arg1[0], &arg2[0], nbits);

	switch (result) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
uge_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
uge_op::debug_string() const
{
	return "BITUGREATEREQ";
}

std::unique_ptr<jive::operation>
uge_op::copy() const
{
	return std::unique_ptr<jive::operation>(new uge_op(*this));
}

}
}

jive::output *
jive_bitugreatereq(jive::output * op1, jive::output * op2)
{
	const jive::bits::type & type =
		dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(
		op1->node()->graph,
		jive::bits::uge_op(type),
		{op1, op2})[0];
}
