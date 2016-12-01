/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdgroup.h>

#include <string.h>

#include <jive/util/buffer.h>
#include <jive/vsdg/graph.h>

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
	return declaration()->nelements();
}

const jive::base::type &
group_op::argument_type(size_t index) const noexcept
{
	return declaration()->element(index);
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

jive::oport *
jive_group_create(std::shared_ptr<const jive::rcd::declaration> & decl,
	size_t narguments, jive::oport * const * arguments)
{
	jive::rcd::group_op op(decl);
	jive::region * region = arguments[0]->region();
	return jive_node_create_normalized(
		region, op, std::vector<jive::oport*>(arguments, arguments + narguments))[0];
}

jive::oport *
jive_empty_group_create(jive_graph * graph,
	std::shared_ptr<const jive::rcd::declaration> & decl)
{
	jive::rcd::group_op op(decl);
	return jive_node_create_normalized(graph->root(), op, {})[0];
}
