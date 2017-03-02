/*
 * Copyright 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/theta.h>

namespace jive {

theta_op::~theta_op() noexcept
{
}
std::string
theta_op::debug_string() const
{
	return "THETA";
}

std::unique_ptr<jive::operation>
theta_op::copy() const
{
	return std::unique_ptr<jive::operation>(new theta_op(*this));
}

}
