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
