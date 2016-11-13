/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitslesseq.h>

#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

sle_op::~sle_op() noexcept {}

bool
sle_op::operator==(const operation & other) const noexcept
{
	const sle_op * o = dynamic_cast<const sle_op *>(&other);
	return o && o->type() == type();
}
compare_result
sle_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.sle(arg2)) {
		case '0': return compare_result::static_false;
		case '1': return compare_result::static_true;
		default: return compare_result::undecidable;
	}
}

jive_binary_operation_flags
sle_op::flags() const noexcept
{
	return jive_binary_operation_none;
}

std::string
sle_op::debug_string() const
{
	return "BITSLESSEQ";
}

std::unique_ptr<jive::operation>
sle_op::copy() const
{
	return std::unique_ptr<jive::operation>(new sle_op(*this));
}

}
}

jive::oport *
jive_bitslesseq(jive::oport * op1, jive::oport * op2)
{
	jive::region * region = op1->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::sle_op(type), {op1, op2})[0];
}
