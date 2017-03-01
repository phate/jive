/*
 * Copyright 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/anchor.h>

#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/graph.h>

namespace jive {

bool
region_anchor_op::operator==(const operation & other) const noexcept
{
	return typeid(*this) == typeid(other);
}

size_t
region_anchor_op::narguments() const noexcept
{
	return 1;
}

const jive::base::type &
region_anchor_op::argument_type(size_t index) const noexcept
{
	static const achr::type type;
	return type;
}

size_t
region_anchor_op::nresults() const noexcept
{
	return 0;
}

const jive::base::type &
region_anchor_op::result_type(size_t index) const noexcept
{
	throw std::logic_error("region_anchor_op has no results");
}

}
