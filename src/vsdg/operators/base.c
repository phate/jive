/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdexcept>

#include <jive/vsdg/operators/base.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/resource.h>

namespace jive {

operation::~operation() noexcept {}

const jive_resource_class *
operation::argument_cls(size_t index) const noexcept
{
	return &jive_root_resource_class;
}

const jive_resource_class *
operation::result_cls(size_t index) const noexcept
{
	return &jive_root_resource_class;
}

jive::node *
operation::create_node(jive::region * region, const std::vector<jive::oport*> & operands) const
{
	return jive_opnode_create(*this, region, operands);
}

}
