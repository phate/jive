/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/immediate-node.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <jive/arch/immediate-type.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/operators/nullary.h>
#include <jive/vsdg/region.h>

/* immediate node */

namespace jive {

immediate_op::~immediate_op() noexcept {}

bool
immediate_op::operator==(const operation & other) const noexcept
{
	const immediate_op * op =
		dynamic_cast<const immediate_op *>(&other);
	return op && jive_immediate_equals(&op->value(), &value());
}

const jive::base::type &
immediate_op::result_type(size_t index) const noexcept
{
	static const jive::imm::type type;
	return type;
}

jive_node *
immediate_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	jive_immediate_node * node = new jive_immediate_node(*this);
	const jive::base::type* restypes[] = {&result_type(0)};
	jive_node_init_(node,
		region,
		0, nullptr, nullptr,
		1, restypes);
	
	return node;
}

std::string
immediate_op::debug_string() const
{
	char tmp[80];
	snprintf(tmp, sizeof(tmp), "%" "lld", value().offset);
	return tmp;
}

std::unique_ptr<jive::operation>
immediate_op::copy() const
{
	return std::unique_ptr<jive::operation>(new immediate_op(*this));
}

}

const jive_node_class JIVE_IMMEDIATE_NODE = {
	parent : &JIVE_NULLARY_OPERATION,
	name : "IMMEDIATE",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_immediate_create(
	jive_graph * graph,
	const jive_immediate * immediate_value)
{
	jive::immediate_op op(*immediate_value);
	return jive_node_create_normalized(graph, op, {})[0];
}
