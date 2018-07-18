/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/operation.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/resource.h>
#include <jive/rvsdg/simple-normal-form.h>
#include <jive/rvsdg/structural-normal-form.h>

namespace jive {

/* port */

port::port(jive::gate * gate)
: gate_(gate)
, rescls_(gate->rescls())
, type_(gate->type().copy())
{}

port::port(const jive::type & type)
: port(type.copy())
{}

port::port(std::unique_ptr<jive::type> type)
: gate_(nullptr)
, rescls_(&jive_root_resource_class)
, type_(std::move(type))
{}

port::port(const resource_class * rescls)
: gate_(nullptr)
, rescls_(rescls)
, type_(rescls->type().copy())
{}

/* operation */

operation::~operation() noexcept
{}

jive::node_normal_form *
operation::normal_form(jive::graph * graph) noexcept
{
	return graph->node_normal_form(typeid(operation));
}

/* simple operation */

simple_op::~simple_op()
{}

size_t
simple_op::narguments() const noexcept
{
	return operands_.size();
}

const jive::port &
simple_op::argument(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < narguments());
	return operands_[index];
}

size_t
simple_op::nresults() const noexcept
{
	return results_.size();
}

const jive::port &
simple_op::result(size_t index) const noexcept
{
	JIVE_DEBUG_ASSERT(index < nresults());
	return results_[index];
}

jive::simple_normal_form *
simple_op::normal_form(jive::graph * graph) noexcept
{
	return static_cast<jive::simple_normal_form*>(graph->node_normal_form(typeid(simple_op)));
}

/* structural operation */

bool
structural_op::operator==(const operation & other) const noexcept
{
	return typeid(*this) == typeid(other);
}

jive::structural_normal_form *
structural_op::normal_form(jive::graph * graph) noexcept
{
	return static_cast<structural_normal_form*>(graph->node_normal_form(typeid(structural_op)));
}

}
