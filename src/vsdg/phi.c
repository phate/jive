/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/phi.h>

#include <stdio.h>

#include <jive/util/strfmt.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/substitution.h>

namespace jive {

phi_op::~phi_op() noexcept
{
}
std::string
phi_op::debug_string() const
{
	return "PHI";
}

std::unique_ptr<jive::operation>
phi_op::copy() const
{
	return std::unique_ptr<jive::operation>(new phi_op(*this));
}

}
