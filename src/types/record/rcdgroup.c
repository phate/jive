/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/record/rcdgroup.h>

#include <string.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/simple_node.h>

namespace jive {
namespace rcd {

group_op::~group_op() noexcept
{}

bool
group_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const group_op*>(&other);
	return op && op->result_ == result_ && op->arguments_ == arguments_;
}

size_t
group_op::narguments() const noexcept
{
	return arguments_.size();
}

const jive::port &
group_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return arguments_[index];
}

size_t
group_op::nresults() const noexcept
{
	return 1;
}

const jive::base::type &
group_op::result_type(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_.type();
}

const jive::port &
group_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return result_;
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

jive::output *
jive_group_create(std::shared_ptr<const jive::rcd::declaration> & decl,
	size_t narguments, jive::output * const * arguments)
{
	jive::rcd::group_op op(decl);
	jive::region * region = arguments[0]->region();
	return jive::create_normalized(
		region, op, std::vector<jive::output*>(arguments, arguments + narguments))[0];
}

jive::output *
jive_empty_group_create(jive::graph * graph,
	std::shared_ptr<const jive::rcd::declaration> & decl)
{
	jive::rcd::group_op op(decl);
	return jive::create_normalized(graph->root(), op, {})[0];
}
