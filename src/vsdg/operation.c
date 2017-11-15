/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/graph.h>
#include <jive/vsdg/operation.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>

namespace jive {

/* port */

port::port(jive::gate * gate)
: gate_(gate)
, rescls_(gate->rescls())
, type_(gate->type().copy())
{}

port::port(const jive::type & type)
: port(std::move(type.copy()))
{}

port::port(std::unique_ptr<jive::type> type)
: gate_(nullptr)
, rescls_(&jive_root_resource_class)
, type_(std::move(type))
{}

port::port(const resource_class * rescls)
: gate_(nullptr)
, rescls_(rescls)
, type_(std::move(rescls->type().copy()))
{}

/* operation */

operation::~operation() noexcept {}

/* simple operation */

simple_op::~simple_op()
{}

/* structural operation */

bool
structural_op::operator==(const operation & other) const noexcept
{
	return typeid(*this) == typeid(other);
}

size_t
structural_op::narguments() const noexcept
{
	return 0;
}

const jive::port &
structural_op::argument(size_t index) const noexcept
{
	JIVE_ASSERT(0 && "Structural operations have no arguments.");
}

size_t
structural_op::nresults() const noexcept
{
	return 0;
}

const jive::port &
structural_op::result(size_t index) const noexcept
{
	JIVE_ASSERT(0 && "Structural operations have no results.");
}

}
