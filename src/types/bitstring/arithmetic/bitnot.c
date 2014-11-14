/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitnot.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

not_op::~not_op() noexcept {}

bool
not_op::operator==(const operation & other) const noexcept
{
	const not_op * o = dynamic_cast<const not_op *>(&other);
	return o && o->type() == type();
}
value_repr
not_op::reduce_constant(
	const value_repr & arg) const
{
	value_repr result(arg);
	jive_bitstring_not(&result[0], &arg[0], arg.nbits());
	return result;
}

std::string
not_op::debug_string() const
{
	return "BITNOT";
}

std::unique_ptr<jive::operation>
not_op::copy() const
{
	return std::unique_ptr<jive::operation>(new not_op(*this));
}

}
}

jive::output *
jive_bitnot(jive::output * arg)
{
	const auto & type = dynamic_cast<const jive::bits::type &>(arg->type());
	return jive_node_create_normalized(arg->node()->graph, jive::bits::not_op(type), {arg})[0];
}
