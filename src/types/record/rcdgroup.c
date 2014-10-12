/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdgroup.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>

namespace jive {
namespace rcd {

group_op::~group_op() noexcept
{
}

bool
group_op::operator==(const operation & other) const noexcept
{
	const group_op * op =
		dynamic_cast<const group_op *>(&other);
	return op && op->declaration() == declaration();
}

size_t
group_op::narguments() const noexcept
{
	return declaration()->nelements;
}

const jive::base::type &
group_op::argument_type(size_t index) const noexcept
{
	return *declaration()->elements[index];
}

size_t
group_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
group_op::result_type(size_t index) const noexcept
{
	return result_type_;
}

jive_node *
group_op::create_node(
	jive_region * region,
	size_t narguments_given,
	jive::output * const arguments[]) const
{
	const jive::base::type * argument_types[narguments()];
	for (size_t n = 0; n < narguments(); ++n) {
		argument_types[n] = &argument_type(n);
	}

	const jive::base::type * result_types[1] = {&result_type(0)};

	jive_group_node * node = new jive_group_node(*this);
	node->class_ = &JIVE_GROUP_NODE;

	jive_node_init_(
		node, region,
		narguments(), argument_types, arguments,
		1, result_types);

	return node;
}

std::string
group_op::debug_string() const
{
	return "GROUP";
}

std::unique_ptr<jive::operation>
group_op::copy() const
{
	return std::unique_ptr<jive::operation>(new group_op(*this));
}

}
}

const jive_node_class JIVE_GROUP_NODE = {
	parent : &JIVE_NODE,
	name : "GROUP",
	fini : jive_node_fini_, /* inherit */
	get_default_normal_form : nullptr,
	get_label : nullptr,
	match_attrs : nullptr,
	check_operands : nullptr,
	create : nullptr
};

jive::output *
jive_group_create(const jive::rcd::declaration * decl,
	size_t narguments, jive::output * const * arguments)
{
	jive::rcd::group_op op(decl);
	jive::output * result;
	jive_graph * graph = arguments[0]->node()->region->graph;
	jive_node_create_normalized(&JIVE_GROUP_NODE, graph, &op, narguments, arguments, &result);
	return result;
}

jive::output *
jive_empty_group_create(jive_graph * graph, const jive::rcd::declaration * decl)
{
	jive::rcd::group_op op(decl);
	jive::output * result;
	jive_node_create_normalized(&JIVE_GROUP_NODE, graph, &op, 0, nullptr, &result);
	return result;
}
