/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/regvalue.h>

#include <string.h>

#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/region.h>
#include <jive/rvsdg/simple-node.h>
#include <jive/types/bitstring/type.h>

namespace jive {

regvalue_op::~regvalue_op() noexcept
{}

bool
regvalue_op::operator==(const jive::operation & other) const noexcept
{
	auto op = dynamic_cast<const regvalue_op*>(&other);
	return op
	    && op->regcls() == regcls()
	    && op->operation() == operation();
}

std::string
regvalue_op::debug_string() const
{
	return detail::strfmt(operation().debug_string(), ":", regcls()->name());
}

std::unique_ptr<jive::operation>
regvalue_op::copy() const
{
	return std::unique_ptr<jive::operation>(new regvalue_op(*this));
}

}
