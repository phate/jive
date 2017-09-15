/*
 * Copyright 2010 2011 2012 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/statemux.h>

#include <jive/common.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/simple_node.h>

namespace jive {

/* mux operator */

mux_op::~mux_op() noexcept {}

bool
mux_op::operator==(const operation & other) const noexcept
{
	auto op = dynamic_cast<const mux_op*>(&other);
	return op
	    && op->narguments_ == narguments_
	    && op->nresults_ == nresults_
	    && op->port_ == port_;
}

size_t
mux_op::narguments() const noexcept
{
	return narguments_;
}

const jive::port &
mux_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return port_;
}

size_t
mux_op::nresults() const noexcept
{
	return nresults_;
}

const jive::port &
mux_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return port_;
}

std::string
mux_op::debug_string() const
{
	return "STATEMUX";
}

std::unique_ptr<jive::operation>
mux_op::copy() const
{
	return std::unique_ptr<jive::operation>(new mux_op(*this));
}

/* mux normal form */

mux_normal_form::~mux_normal_form() noexcept
{}

mux_normal_form::mux_normal_form(
	const std::type_info & opclass,
	jive::node_normal_form * parent,
	jive::graph * graph) noexcept
: simple_normal_form(opclass, parent, graph)
{}

bool
mux_normal_form::normalize_node(jive::node * node) const
{
	return simple_normal_form::normalize_node(node);
}

std::vector<jive::output*>
mux_normal_form::normalized_create(
	jive::region * region,
	const jive::simple_op & op,
	const std::vector<jive::output*> & operands) const
{
	return simple_normal_form::normalized_create(region, op, operands);
}

}

namespace {

static jive::node_normal_form *
create_mux_normal_form(
	const std::type_info & opclass,
	jive::node_normal_form * parent,
	jive::graph * graph)
{
	return new jive::mux_normal_form(opclass, parent, graph);
}

static void __attribute__((constructor))
register_node_normal_form(void)
{
	jive::node_normal_form::register_factory(typeid(jive::mux_op), create_mux_normal_form);
}

}
