/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/bitstring/arithmetic/bitor.h>
#include <jive/types/bitstring/bitoperation-classes-private.h>
#include <jive/types/bitstring/constant.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {
namespace bits {

or_operation::~or_operation() noexcept {}

bool
or_operation::operator==(const operation & other) const noexcept
{
	const or_operation * o = dynamic_cast<const or_operation *>(&other);
	return o && o->type() == type();
}

jive_node *
or_operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return detail::binop_create<or_operation>(
		*this,
		&JIVE_BITOR_NODE,
		region,
		narguments,
		arguments);
}

value_repr
or_operation::reduce_constants(
	const value_repr & arg1,
	const value_repr & arg2) const
{
	size_t nbits = std::min(arg1.size(), arg2.size());
	value_repr result(nbits, '0');
	jive_bitstring_or(&result[0], &arg1[0], &arg2[0], nbits);
	return result;
}

jive_binary_operation_flags
or_operation::flags() const noexcept
{
	return jive_binary_operation_associative | jive_binary_operation_commutative;
}

std::string
or_operation::debug_string() const
{
	return "BITOR";
}

}
}

const jive_node_class JIVE_BITOR_NODE = {
	parent : &JIVE_BITBINARY_NODE,
	name : "BITOR",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : jive_binary_operation_get_default_normal_form_, /* inherit */
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr,
};

jive::output *
jive_bitor(size_t noperands, jive::output * const * operands)
{
	return jive::bits::detail::binop_normalized_create<
		jive::bits::or_operation>(
			&JIVE_BITOR_NODE, noperands, operands);
}
