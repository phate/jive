/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/rvsdg/graph.hpp>
#include <jive/rvsdg/node.hpp>
#include <jive/types/float/flttype.hpp>

#include <string.h>

namespace jive {
namespace flt {

/* float */

type::~type() noexcept {}

std::unique_ptr<jive::type>
type::copy() const
{
	return std::unique_ptr<jive::type>(new type(*this));
}

std::string
type::debug_string() const
{
	return "flt";
}

bool
type::operator==(const jive::type & other) const noexcept
{
	return dynamic_cast<const jive::flt::type*>(&other) != nullptr;
}

}
}
