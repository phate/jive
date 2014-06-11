/*
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdexcept>

#include <jive/vsdg/operators/base.h>

#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>

namespace jive {

operation::~operation() noexcept {}

bool
operation::operator==(const operation & other) const noexcept
{
	throw std::logic_error("abstract base class");
}

size_t
operation::narguments() const noexcept
{
	throw std::logic_error("abstract base class");
}

const jive::base::type &
operation::argument_type(size_t index) const noexcept
{
	throw std::logic_error("abstract base class");
}

size_t
operation::nresults() const noexcept
{
	throw std::logic_error("abstract base class");
}

const jive::base::type &
operation::result_type(size_t index) const noexcept
{
	throw std::logic_error("abstract base class");
}

jive_node *
operation::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	throw std::logic_error("abstract base class");
}

std::string
operation::debug_string() const
{
	throw std::logic_error("abstract base class");
}

}
