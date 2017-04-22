/*
 * Copyright 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * Copyright 2014 Helge Bahmann <hcb@chaoticmind.net>
 * See COPYING for terms of redistribution.
 */

#include <jive/types/float/flttype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>

#include <string.h>

namespace jive {
namespace flt {

/* float */

type::~type() noexcept {}

std::unique_ptr<base::type>
type::copy() const
{
	return std::unique_ptr<base::type>(new type(*this));
}

std::string
type::debug_string() const
{
	return "flt";
}

bool
type::operator==(const jive::base::type & other) const noexcept
{
	return dynamic_cast<const jive::flt::type*>(&other) != nullptr;
}

}
}
