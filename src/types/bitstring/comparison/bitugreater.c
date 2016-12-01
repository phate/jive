/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/comparison/bitugreater.h>

#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/control.h>
#include <jive/vsdg/controltype.h>
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
compare_result
ugt_op::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	switch (arg1.ugt(arg2)) {
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

jive::oport *
jive_bitugreater(jive::oport * op1, jive::oport * op2)
{
	jive::region * region = op1->region();
	const jive::bits::type & type = dynamic_cast<const jive::bits::type &>(op1->type());
	return jive_node_create_normalized(region, jive::bits::ugt_op(type), {op1, op2})[0];
}
