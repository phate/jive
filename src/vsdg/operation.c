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

port::port(const jive::base::type & type)
: port(std::move(type.copy()))
{}

port::port(std::unique_ptr<jive::base::type> type)
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

const jive::resource_class *
operation::argument_cls(size_t index) const noexcept
{
	return &jive_root_resource_class;
}

const jive::resource_class *
operation::result_cls(size_t index) const noexcept
{
	return &jive_root_resource_class;
}

}
